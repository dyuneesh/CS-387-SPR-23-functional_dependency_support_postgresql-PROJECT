#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "postgresql/src/include/utils/res_tuple_table.h"

//=====================================================================================================

/*
convert a string to lower case
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

char* strip_space(char* input){
    int i = 0;
    while(input[i] == ' ') i++;


    int j = strlen(input)-1;
    while(input[j] == ' ') j--;

    char* result = (char*)malloc(j-i+2);
    strncpy(result, input+i, j-i+1);

    return result;
}


/**
 * Parses a bracketed list of values and returns an array of strings
 * @param a string of the form       '   (5, 6 , 'hello' , 7.52 , 'bye'  ) '
 * @return char** : array of strings of the form [ 5, 6 , 'hello' , 7.52 , 'bye' ]
 */
char** parse_values_list(char* values, int* num_items){
    
        if(values == NULL) return NULL;
        char* copy_values = (char*)malloc(strlen(values)+1);
        char** result = (char**)malloc(100*sizeof(char*));
        char* token;
        char tmp[100];
        bool attr_started;
        bool string_attr;
    
        strcpy(copy_values, values);

        int i = 0;
        while(copy_values[i] == ' ') i++;
        if(copy_values[i] != '(') return NULL;
        copy_values = copy_values + i+1;

        token = strtok(copy_values, ")");
        if(token == NULL) return NULL;
        token[strlen(token)] = ' ';

        attr_started = false;
        string_attr = false;

        *num_items = 0;
        int j = 0;
        for(int i = 0; i < strlen(token); i++){
            if(!attr_started){
                while(token[i] == ' ' || token[i] == ',') i++;
                attr_started = true;

                if(token[i] == '\''){
                    string_attr = true;
                }
                else{
                    string_attr = false;
                    tmp[j++] = token[i];
                }
            }
            else{
                    if( (string_attr && token[i] == '\'') || !string_attr && (token[i] == ' ' || token[i] == ',') ){
                        tmp[j] = '\0';

                        result[*num_items] = (char*)malloc(strlen(tmp)+1);
                        strcpy(result[(*num_items)++], tmp);
                        
                        attr_started = false;
                        j = 0;

                    }
                    else{
                        tmp[j++] = token[i];
                    }
                }
            }

        return result;
}


/*
Parses a Functional dependency LHS/RHS into a list of values.
"{a,b,c}" ==> [ "a" , "b" , "c" ]
*/
char** parse_fd(char* fd_string, int* num_attrs){

    if(fd_string == NULL || strlen(fd_string) == 0){
        *num_attrs = 0;
        return NULL;
    }

    for(int i = 0; i < strlen(fd_string); i++){
        if(fd_string[i] == ',') (*num_attrs)++;
    }
    (*num_attrs)++;


    char* copy_fd_string = (char*)malloc(strlen(fd_string)+1);
    strcpy(copy_fd_string, fd_string); 

    char** result = (char**)malloc((*num_attrs)*sizeof(char*));
    char* token;

    token = strtok(copy_fd_string, "{,}");
    if(token == NULL) return NULL;

    for(int i = 0; i < *num_attrs; i++){
        result[i] = strip_space(token);
        token = strtok(NULL, "{,}");
    }


    //freeing malloced memory.
    free(copy_fd_string);

    return result;

}



char* fd_check_query(char** values, char** attrs, int nattrs, char** lhs, int nlhs, char** rhs, int nrhs, char* table_name){
    
        char* query = (char*)malloc(1000);
    
        strcpy(query, "select * from ");
        strcat(query, table_name);
        strcat(query, "\nwhere\n");

        for(int i = 0; i < nlhs; i++){
            for(int j = 0; j < nattrs; j++){
                if(strcmp(lhs[i], attrs[j]) == 0){
                    strcat(query, attrs[j]);
                    strcat(query, " = ");
                    strcat(query, values[j]);
                }
            }
            if( i != nlhs-1 ) strcat(query, " and\n");
            else strcat(query, "\nAND(\n");
        }

        for(int i = 0; i < nrhs; i++){
            for(int j = 0; j < nattrs; j++){
                if(strcmp(rhs[i], attrs[j]) == 0){
                    strcat(query, "\t");
                    strcat(query, attrs[j]);
                    strcat(query, " <> ");
                    strcat(query, values[j]);
                }
            }
            if( i != nrhs-1 ) strcat(query, " or\n");
            else strcat(query, "\n   )\n");
        }
    
        return query;
}


/*
Takes a SINGLE query string as input and returns the modified query string
That Checks the Functional Dependency constraints.
*/
char * fd_mod(char* query_string, struct TupleTable* tuple_table){

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

    /*if the insert is not using values(.........) OR if insert is into FD table*/
    char* token4_substr = (char*)malloc(7);
    strncpy(token4_substr, token4_lower, 6);
    if(strcmp(token4_substr, "values") != 0 || strcmp(token3, "fd") == 0){
    	return copy_query_string;
    }

    char* values_tuple = token4 + 6;
    int num_items = 0;

    char** values = parse_values_list(values_tuple, &num_items);

    // for(int i = 0; i < num_items; i++){
    // 	printf("\"%s\"\n", values[i]);
    // }


    /**
     *This query is to get the Header info of the Table.
     *Query: select * from <table_name> where 1=1;
    */
    char* table_name = token3;

    char* select_query = (char*)malloc(strlen(table_name) + 30);
    strcpy(select_query, "select * from ");
    strcat(select_query, table_name);
    strcat(select_query, " where 1=1 ;");
    

    /**
     * FD Query: select * from fd where table = <table_name>
     * this query is to get all the fd's registered on this table.
     */
    char* fd_query = (char*)malloc(strlen(table_name) + 30);
    strcpy(fd_query, "select * from fd where table = ");
    strcat(fd_query, table_name);
    strcat(fd_query, " ;");





    return copy_query_string;

}





int main(){

    // // test fd check query.

    // char* values[] = {"'a'", "'b'", "'c'", "'d'"};
    // char* attrs[] = {"a", "b", "c", "d"};
    // int nattrs = 4;

    // char* lhs[] = {"d", "a"};
    // int nlhs = 2;

    // char* rhs[] = {"b", "c"};
    // int nrhs = 2;

    // char* table_name = "test";

    // char* query = fd_check_query(values, attrs, nattrs, lhs, nlhs, rhs, nrhs, table_name);

    // printf("%s\n", query);


    //test fd parse/..
    char* fd_string = "{apple, ball, cat,dumo}";
    int num_attrs = 0;
    char** attrs = parse_fd(fd_string, &num_attrs);

    for(int i = 0; i < num_attrs; i++){
        printf("%s\n", attrs[i]);
    }

}
