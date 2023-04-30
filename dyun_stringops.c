#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

//=====================================================================================================

/*
it doesn't modify the given input string.
*/
char** parse_list(char* list, int* num_items, int* size_items){
	if(list == NULL) return NULL;
	*num_items = 0;
	*size_items = 0;
	char**result = (char**)malloc(100*sizeof(char*));
	char* token;
	char tmp[100];
	int i = 0;
	int j = 0;


	while(list[i] == ' ') i++;
	while(i<strlen(list)) {
		while(list[i] != ' ' && list[i] != ',' && list[i] != '\0') {
			tmp[j++] = list[i++];
		}
		tmp[j] = '\0';
		result[*num_items] = (char*)malloc(strlen(tmp)+1);
		strcpy(result[*num_items], tmp);
		(*num_items)++;
		(*size_items) += j;
		j = 0;

		while(list[i] == ' ') i++;
		if(list[i] == '\0') {
			return result;
		}
		else { //comma
			i++;
			while(list[i] == ' ') i++;
		}
	}
	//for safety
	return result;
}

char* lower_case_str(char* str){
	if(str == NULL) return NULL;
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

bool check_creat_fd(char* qs) {
	//create fd <fd_name> on <tab_name> a,b,c : d,e,f
	char* copy_qs = (char*)malloc(strlen(qs)+1);
	strcpy(copy_qs, qs);

	char* tok_creat = strtok(copy_qs, " ");		if(tok_creat == NULL) {free(copy_qs); return false;}
	char* tok_fd = strtok(NULL, " ");			if(tok_fd == NULL) {free(copy_qs); return false;}

	tok_creat = lower_case_str(tok_creat);
	tok_fd = lower_case_str(tok_fd);

	if( strcmp(tok_creat, "create") != 0 || strcmp(tok_fd, "fd")) {
		free(tok_creat); free(tok_fd); free(copy_qs);
		return false;
	}
	
	free(tok_creat); free(tok_fd); free(copy_qs);
	return true;
}

bool check_del_fd(char* qs) {
	//delete fd <fd_name> on <tab_name>
	char* copy_qs = (char*)malloc(strlen(qs)+1);
	strcpy(copy_qs, qs);

	char* tok_del = strtok(copy_qs, " ");		if(tok_del == NULL) {free(copy_qs); return false;}
	char* tok_fd = strtok(NULL, " ");			if(tok_fd == NULL) {free(copy_qs); return false;}

	tok_del = lower_case_str(tok_del);
	tok_fd = lower_case_str(tok_fd);

	if( strcmp(tok_del, "delete") != 0 || strcmp(tok_fd, "fd")) {
		free(tok_del); free(tok_fd); free(copy_qs);
		return false;
	}
	
	free(tok_del); free(tok_fd); free(copy_qs);
	return true;
}

char* my_fd_mod(char* query_string, char* table_name) {
	if(strlen(query_string) == 0) return query_string;

	char* copy_qs = (char*)malloc(strlen(query_string)+1);
	strcpy(copy_qs, query_string);

	char *FD_TABLE_NAME = lower_case_str(FD_TABLE_NAME);


	//create fd <fd_name> on <tab_name> a,b,c : d,e,f
	if(check_creat_fd(copy_qs)) {
		char* tok_creat = strtok(copy_qs, " "); 
		char* tok_fd = strtok(NULL, " ");
		char* tok_fdname = strtok(NULL, " ");   if(tok_fdname == NULL) {return copy_qs;}
		char* tok_on = strtok(NULL, " "); 		if(tok_on == NULL) {return copy_qs;}
		char* tok_tabname = strtok(NULL, " ");	if(tok_tabname == NULL) {return copy_qs;}
		char* tok_lhs = strtok(NULL, ":");		if(tok_lhs == NULL) {return copy_qs;}
		char* tok_rhs = strtok(NULL, ";");		if(tok_rhs == NULL) {return copy_qs;}

		tok_on = lower_case_str(tok_on);		if( strcmp(tok_on, "on") != 0 ) {
			free(tok_on);
			return copy_qs;
		}
		tok_fdname = lower_case_str(tok_fdname);
		tok_tabname = lower_case_str(tok_tabname);
		tok_lhs = lower_case_str(tok_lhs);
		tok_rhs = lower_case_str(tok_rhs);
		
		int num_lhs, size_lhs;
		char** FD_LHS = parse_list(tok_lhs,&num_lhs,&size_lhs);
		int num_rhs, size_rhs;
		char** FD_RHS = parse_list(tok_rhs,&num_rhs,&size_rhs);

		char* my_query = (char*)malloc(
			50+strlen(FD_TABLE_NAME)+strlen(tok_fdname)+strlen(tok_tabname)+(size_lhs+size_rhs)+3*(num_lhs+num_rhs)
		);
		strcpy(my_query, "insert into "); strcat(my_query, FD_TABLE_NAME);	strcat(my_query, " values('");

		strcat(my_query, tok_fdname); strcat(my_query, "','");	strcat(my_query, tok_tabname);
		strcat(my_query, "','{");
		for(int i=0; i<num_lhs; i++) {
			if(i != 0) strcat(my_query, ",");
			strcat(my_query, "\""); strcat(my_query, FD_LHS[i]); strcat(my_query, "\"");
		}
		strcat(my_query, "}','{");
		for(int i=0; i<num_rhs; i++) {
			if(i != 0) strcat(my_query, ",");
			strcat(my_query, "\"");	strcat(my_query, FD_RHS[i]); strcat(my_query, "\"");
		}
		strcat(my_query, "}');");
		free(copy_qs); 
		free(tok_fdname); free(tok_on); free(tok_tabname); free(tok_lhs); free(tok_rhs);		
		return my_query;
	}
	else if(check_del_fd(copy_qs)) { // delete_fd
		//delete fd <fd_name> from <tab_name> ;
		char* tok_del = strtok(copy_qs, " "); 
		char* tok_fd = strtok(NULL, " ");
		char* tok_fdname = strtok(NULL, " ");   if(tok_fdname == NULL) {return copy_qs;}
		char* tok_from = strtok(NULL, " "); 		if(tok_from == NULL) {return copy_qs;}
		char* tok_tabname = strtok(NULL, " ;");	if(tok_tabname == NULL) {return copy_qs;}

		tok_from = lower_case_str(tok_from);		if( strcmp(tok_from, "from") != 0 ) {
			free(tok_from);
			return copy_qs;
		}
		tok_fdname = lower_case_str(tok_fdname);
		tok_tabname = lower_case_str(tok_tabname);
		//need to free tok_ fdname,tabname,from,
		char* my_query = (char*)malloc(
			60+strlen(FD_TABLE_NAME)+strlen(tok_fdname)+strlen(tok_tabname)
		);
		strcpy(my_query, "delete from "); strcat(my_query, FD_TABLE_NAME);	
		strcat(my_query, " where table_name = '"); strcat(my_query, tok_tabname);
		strcat(my_query, "' and fd_name = '"); strcat(my_query, tok_fdname); strcat(my_query, "';");

		free(copy_qs);
		free(tok_fdname); free(tok_from); free(tok_tabname); 
		return my_query;

	}
	else { // insert into : code

	}
	return copy_qs;
}

//=====================================================================================================

int main()
{

	// TEST LATER WITH MOD MULTILE. select * from a; bull shit; bully ; insert ; insert into f select * from s;
	//  int num;
	while (1)
	{
		char *query_string = (char *)malloc(1000);
		printf("Enter query string: ");
		scanf("%[^\n]%*c", query_string);
		// char** res = parse_list(query_string,&num);
		// printf("Num tokens: %d\n", num);

		// for(int i = 0; i < num; i++) printf("%s\n",res[i]);
		// printf("---------------------------------\n");
		printf("%s\n", my_fd_mod(query_string));
	}
}