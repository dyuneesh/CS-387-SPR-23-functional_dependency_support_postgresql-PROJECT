#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

//=====================================================================================================

/*
convert a string to lower case
useful for string comparison in case insensitive manner
*/
char* lower_case_str(char* str){

	if(str == NULL) return NULL;\
	if(strlen(str) == 0) return str;

	char* lower_case_str = (char*)malloc(strlen(str)+1);

	strcpy(lower_case_str, str);

	for(int i=0; i<strlen(lower_case_str); i++){
		if(lower_case_str[i] >= 'A' && lower_case_str[i] <= 'Z'){
			lower_case_str[i] = lower_case_str[i] + 32;
		}
	}

	return lower_case_str;
}


/*
it doesn't modify the given input string.
*/
char** parse_list(char* list, int* num_items){

	if(list == NULL) return NULL;
	*num_items = 0;

	char**result = (char**)malloc(100*sizeof(char*));

	char* token;
	char tmp[100];
	int i = 0;
	int j = 0;
	bool list_started = false;


	for(i=0; i<strlen(list); i++){

		if(!list_started){
			while (list[i] == ' ') i++;
			if(list[i] == '\''){
				list_started = true;
				continue;
			}
			else{
				return NULL;
			}
		}
		else{
			if(list[i] != '\''){
				tmp[j++] = list[i];
			}
			else{
				tmp[j] = '\0';
				result[*num_items] = (char*)malloc(strlen(tmp)+1);
				strcpy(result[*num_items], tmp);
				(*num_items)++;
				j = 0;

				i++;
				while (list[i] == ' ' || list[i] == ',') i++;
				if(list[i] != '\''){
					return result;
				}


			}
		}

	}

}


char* create_trigger(char* table, char** lhs, int lhs_num , char** rhs, int rhs_num){

	char* trigger_name = (char*)malloc(strlen(table)+10);
	strcpy(trigger_name, table);
	strcat(trigger_name, "_trigger");

	char* trigger_query = (char*)malloc(1000);
	strcpy(trigger_query, "CREATE TRIGGER ");
	strcat(trigger_query, trigger_name);
	strcat(trigger_query, " BEFORE INSERT ON ");
	strcat(trigger_query, table);
	strcat(trigger_query, "\nreferencing new row as nrow\n");
	strcat(trigger_query, "FOR EACH ROW \n");
	strcat(trigger_query, "WHEN (\n\texists (select * from ");
	strcat(trigger_query, table);
	strcat(trigger_query, " where \n");

	for(int i=0; i<lhs_num; i++){
		strcat(trigger_query, "\t\t nrow.");
		strcat(trigger_query, lhs[i]);
		strcat(trigger_query, " = ");
		strcat(trigger_query, lhs[i]);
		if(i != lhs_num-1){
			strcat(trigger_query, " AND \n");
		}
	}

	strcat(trigger_query, "\n \tAND \n");

	for(int i=0; i<rhs_num; i++){
		strcat(trigger_query, "\t\tnrow.");
		strcat(trigger_query, rhs[i]);
		strcat(trigger_query, " <> ");
		strcat(trigger_query, rhs[i]);
		if(i != rhs_num-1){
			strcat(trigger_query, " OR \n");
		}
	}

	strcat(trigger_query, "\n)\n");
	strcat(trigger_query, "BEGIN\n\tROLLBACK \nEND;");

	return trigger_query;
}

/*
Takes a SINGLE query string as input and returns the modified query string
that creates a new FD table OR inserts into an existing FD table for a particular FD.
*/
char * fd_mod(char* query_string){

	if(strlen(query_string) == 0) return query_string;

	char* copy_query_string = (char*)malloc(strlen(query_string)+1);
	strcpy(copy_query_string, query_string);


	char* token1 = strtok(query_string, " ");  if(token1 == NULL) return copy_query_string;
	char* token2 = strtok(NULL, " ");          if(token2 == NULL) return copy_query_string;
	char* token3 = strtok(NULL, " ");          if(token3 == NULL) return copy_query_string;
	char* token4 = strtok(NULL, ";");          if(token4 == NULL) return copy_query_string;

	token1 = lower_case_str(token1);
	token2 = lower_case_str(token2);
	token3 = lower_case_str(token3);
	char * token4_lower = lower_case_str(token4);

	/*if its not an insert query*/
	if( strcmp(token1, "insert") != 0 || strcmp(token2, "into") != 0){
		return copy_query_string;
	}

    /*if the insert is not using values(.........) OR if insert is not into FD table*/
    char* token4_substr = (char*)malloc(7);
    strncpy(token4_substr, token4_lower, 6);
    if(strcmp(token4_substr, "values") != 0 || strcmp(token3, "fd") != 0){
    	return copy_query_string;
    }

	//NOW INSERT IS INTO FD TABLE.... NEED TO EXTRACT ALL THE ATTRIBUTES OF FD.
	char* token5 = strtok(token4, "'");  if(token5 == NULL) return copy_query_string;
	token4 = token4 + strlen(token5) + 1;
	token5 = strtok(token4, "'");		  if(token5 == NULL) return copy_query_string;
	token4 = token4 + strlen(token5) + 1;

	char* TABLE_NAME = (char*)malloc(strlen(token5)+1);
	strcpy(TABLE_NAME, token5);

	token5 = strtok(token4, "{");		  if(token5 == NULL) return copy_query_string;
	token4 = token4 + strlen(token5) + 1;

	int num_LHS;
	char** FD_LHS = parse_list(token4, &num_LHS);

	token5 = strtok(token4, "{");		  if(token5 == NULL) return copy_query_string;
	token4 = token4 + strlen(token5) + 1;

	int num_RHS;
	char** FD_RHS = parse_list(token4, &num_RHS);

	char* trigger_query = create_trigger(TABLE_NAME, FD_LHS, num_LHS, FD_RHS, num_RHS);
	char* return_query = (char*)malloc(strlen(trigger_query)+strlen(copy_query_string)+3);
	strcpy(return_query, copy_query_string);
	strcat(return_query, ";\n");
	strcat(return_query, trigger_query);

	return return_query;
}


/*
Takes a COMPOSITE query string as input and returns the modified query string
COMPOSITE means consists of multiple queries separated by semicolon
*/
char* fd_mod_multiple(const char * query_string){

	if(query_string == NULL) return NULL;
	if(strlen(query_string) == 0) return query_string;

	
	int required_len = 0;
	int num_queries = 0;
	char* modified_queries[100];

	char* token = strtok(query_string, ";");

	while(token != NULL){
		char* modified_query = fd_mod(token);
		modified_queries[num_queries++] = modified_query;
		required_len += strlen(modified_query);
		token = strtok(NULL, ";");
	}

	char* modified_query_string = (char*)malloc(required_len+1+num_queries);
	modified_query_string[0] = '\0';

	for (int i = 0; i < num_queries; i++)
	{
		strcat(modified_query_string, modified_queries[i]);
		strcat(modified_query_string, ";");
	}
	
	return modified_query_string;

}


//=====================================================================================================


int main(){


	//TEST LATER WITH MOD MULTILE. select * from a; bull shit; bully ; insert ; insert into f select * from s;
	// int num;
	while(1){
		char* query_string = (char*)malloc(1000);
		printf("Enter query string: ");
		scanf("%[^\n]%*c", query_string);
		// char** res = parse_list(query_string,&num);
		// printf("Num tokens: %d\n", num);

		// for(int i = 0; i < num; i++) printf("%s\n",res[i]);
		// printf("---------------------------------\n");
		printf("%s\n", fd_mod(query_string) );
	}


}