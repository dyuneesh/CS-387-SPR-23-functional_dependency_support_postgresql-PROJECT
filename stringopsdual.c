#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "postgresql/src/include/utils/res_tuple_table.h"

//=====================================================================================================
/*
convert a string to lower case
return str needs to be explicitly freed by caller.
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
strips trailing, leading white space...
return str needs to be explicitly freed by caller.
*/
char* strip_space(char* input){
    int i = 0;
    while(input[i] == ' ') i++;


    int j = strlen(input)-1;
    while(input[j] == ' ') j--;

    char* result = (char*)malloc(j-i+2);
    strncpy(result, input+i, j-i+1);

	result[j-i+1] = '\0';

    return result;
}


/**
 * Parses a bracketed list of values and returns an array of strings
 * @param a string of the form       "(5, 6, 'hello', 7.52, 'bye')"
 * @return char** : array of strings of the form [ '5', '6', 'hello', '7.52' , 'bye' ]
 * return str needs to be explicitly freed by caller.
 */
char** parse_values_list(char* values, int* num_items){
    
        if(values == NULL) return NULL;
        char* copy_values = (char*)malloc(strlen(values)+4);
		char* malloced_ptr = copy_values;
        char** result = (char**)malloc(10*sizeof(char*));
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

		
		// freeing malloced memory
		free(malloced_ptr);


        return result;
}


/*
Parses a Functional dependency LHS/RHS into a list of values.
"{a,b,c}" ==> [ "a" , "b" , "c" ]
return str needs to be explicitly freed by caller.
*/
char** parse_fd(char* fd_string, int* num_attrs){

    *num_attrs = 0;

    if(fd_string == NULL || strlen(fd_string) == 0){
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


/*
creates a query string to validate the functional dependency constraint.
( SEL * FROM TABLE WHERE lhs = <..> && rhs != <..> )
return str needs to be explicitly freed by caller.
*/
char* fd_check_query(char** values, char** attrs, int nattrs, char** lhs, int nlhs, char** rhs, int nrhs, char* table_name){
    
        char* query = (char*)malloc(1000);
    
        strcpy(query, "select * from ");
        strcat(query, table_name);
        strcat(query, "\nwhere\n");

        for(int i = 0; i < nlhs; i++){
            for(int j = 0; j < nattrs; j++){
                if(strcmp(lhs[i], attrs[j]) == 0){
                    strcat(query, attrs[j]);
                    strcat(query, " = '");
                    strcat(query, values[j]);
					strcat(query, "'");
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
                    strcat(query, " <> '");
                    strcat(query, values[j]);
					strcat(query, "'");
                }
            }
            if( i != nrhs-1 ) strcat(query, " or\n");
            else strcat(query, "\n   )\n");
        }
    
        return query;
}



/*
Creates a trigger to check the fd constraiint before inserts.
*/
char* create_trigger(char* table, char** lhs, int lhs_num , char** rhs, int rhs_num){

    // Name the trigger with <tablename>_<trigger>_<lhs>_<rhs>
	char* trigger_name = (char*)malloc(200);

    strcpy(trigger_name, "_lhs");
    for(int i = 0; i < lhs_num ; i++){
        strcat(trigger_name,"_");
        strcat(trigger_name,lhs[i]);
    }

    strcat(trigger_name, "_rhs");
    for(int i = 0; i < rhs_num ; i++){
        strcat(trigger_name,"_");
        strcat(trigger_name,rhs[i]);
    }

	char* trigger_query = (char*)malloc(1000);


	strcpy(trigger_query,"CREATE OR REPLACE FUNCTION check_fd_");
	strcat(trigger_query,table);
	strcat(trigger_query,trigger_name);
	strcat(trigger_query,"()\n");
	strcat(trigger_query,"RETURNS TRIGGER AS $$\n");
	strcat(trigger_query,"BEGIN\n");
	strcat(trigger_query,"\tIF EXISTS ( SELECT 1 FROM ");
	strcat(trigger_query,table);
	strcat(trigger_query," WHERE \n");

	for(int i=0; i<lhs_num; i++){
		strcat(trigger_query,"\t\t");
		strcat(trigger_query,lhs[i]);
		strcat(trigger_query," = NEW.");
		strcat(trigger_query,lhs[i]);
		if(i != lhs_num-1){
			strcat(trigger_query," AND \n");
		}
	}

	strcat(trigger_query,"\n\t\tAND (\n");

	for(int i=0; i<rhs_num; i++){
		strcat(trigger_query,"\t\t\t");
		strcat(trigger_query,rhs[i]);
		strcat(trigger_query," <> NEW.");
		strcat(trigger_query,rhs[i]);
		if(i != rhs_num-1){
			strcat(trigger_query," OR \n");
		}
	}

	strcat(trigger_query,"\n\t\t    )\n\t) THEN\n");

	strcat(trigger_query,"\t\tRAISE EXCEPTION 'FD constraint [");
	for(int i=0; i<lhs_num; i++){
		strcat(trigger_query,lhs[i]);
		if(i != lhs_num-1){
			strcat(trigger_query,",");
		}
	}
	strcat(trigger_query,"] -> [");
	for(int i=0; i<rhs_num; i++){
		strcat(trigger_query,rhs[i]);
		if(i != rhs_num-1){
			strcat(trigger_query,",");
		}
	}

	strcat(trigger_query,"] violated';\n\t\tROLLBACK;\n\tEND IF;\n\tRETURN NEW;\nEND;\n$$ LANGUAGE plpgsql;\n\n");

	



	strcat(trigger_query, "CREATE TRIGGER trigger_");
	strcat(trigger_query, table);
	strcat(trigger_query, trigger_name);
	strcat(trigger_query, "\nBEFORE INSERT ON ");
	strcat(trigger_query, table);
	strcat(trigger_query, "FOR EACH ROW \n");
	strcat(trigger_query, "EXECUTE FUNCTION check_fd_");
	strcat(trigger_query, table);
	strcat(trigger_query, trigger_name);
	strcat(trigger_query, "();\n\n");


	return trigger_query;
}


/*
it returns the insert values list, for insert queries of type ``ins into table values(.....);``
Otherwise returns NULL.
return str needs to be explicitly freed by caller.
*/
char ** insert_parse(char* query_string, int* num_vals, char* table_name, int* mode){

	if(query_string == NULL || strlen(query_string) == 0) return NULL;

	char* copy_query_string = (char*)malloc(strlen(query_string)+4);
	strcpy(copy_query_string, query_string);
	strcat(copy_query_string, "; ");


	char* token1 = strtok(copy_query_string, " ");  if(token1 == NULL) return NULL;
	char* token2 = strtok(NULL, " ");          if(token2 == NULL) return NULL;
	char* token3 = strtok(NULL, " ");          if(token3 == NULL) return NULL;
	char* token4 = strtok(NULL, ";");          if(token4 == NULL) return NULL;

	token1 = lower_case_str(token1);
	token2 = lower_case_str(token2);
	token3 = lower_case_str(token3);
	char * token4_lower = lower_case_str(token4);

	/*if its not an insert query*/
	if( strcmp(token1, "insert") != 0 || strcmp(token2, "into") != 0){
		return NULL;
	}

    /*if the insert is not using values(.........) OR if insert is into FD1 table*/
    char token4_substr[7];
    strncpy(token4_substr, token4_lower, 6);
    if(strcmp(token4_substr, "values") != 0 || strcmp(token3, "fd1") == 0){
    	return NULL;
    }


    char* values_tuple = token4 + 6;
    int num_items = 0;

    char** values = parse_values_list(values_tuple, &num_items);
	*num_vals = num_items;

	strcpy(table_name, token3);

    // if the insert is into second fd2 table.... need to create a trigger. i.e, mode2
    if(strcmp(token3, "fd2") == 0){
        *mode = 2;
    }
    else{
        *mode = 1;
    }


	// freeing malloced memory
	free(token1);
	free(token2);
	free(token3);
	free(token4_lower);
	free(copy_query_string);

    return values;

}





/*
split a Composite query into multiple queries.
return str needs to be explicitly freed by caller.
*/
char** split_query(char* query_string, int* num_queries){

    *num_queries = 0;
	if(query_string == NULL || strlen(query_string) == 0) return NULL;

	char** result = (char**)malloc(10*sizeof(char*));
    char* token;

    token = strtok(query_string, ";");
    while(token != NULL){
        result[*num_queries] = (char*)malloc(strlen(token)+1);
        strcpy(result[*num_queries], token);
        (*num_queries)++;
        token = strtok(NULL, ";");
    }

    return result;
}


// /*
// Takes a COMPOSITE query string as input and executes each query one by one, if FD contraints are satisfied.
// */
// void exec_multiple(const char * query_string){

// 	if(query_string == NULL) return NULL;
// 	if(strlen(query_string) == 0) return query_string;



//     int mode;
// 	char** values;
// 	char* table_name = (char*)malloc(25);
// 	int num_vals;
// 	struct TupleTable* HeaderTable;
// 	struct TupleTable* FDTable;
// 	struct TupleTable* CheckTable;


// 	/**
// 	 * ASSUMPTION : Each query ends with a semicolon, and there are no semicolons within the query itself.
// 	 */
// 	int num_queries = 0;
// 	char** queries = split_query(query_string, &num_queries);


// 	char* token;

// 	for(int i = 0; i < num_queries; i++){
// 		token = queries[i];
// 		bool execute = true;

// 		if(token == NULL || strlen(token) == 0) continue;

// 		num_vals = 0;
// 		values = insert_parse(token,&num_vals, table_name, &mode);

//         if(values == NULL){

//         }
//         else if(mode == 1){
// 			// the query is an insert query. So check the FD constraints.

// 			/*
// 			 *This query is to get the Header info of the Table.
// 			 *Query: select * from <table_name> where 1=1;
// 			 */
// 			char* select_query = (char*)malloc(strlen(table_name) + 30);
// 			strcpy(select_query, "select * from ");
// 			strcat(select_query, table_name);
// 			strcat(select_query, ";");

// 			HeaderTable = exec_simple_query(select_query, 1);
// 			// res_printTupleTable(HeaderTable);
			

// 			/**
// 			 * FD Query: select * from fd where table = <table_name>
// 			 * this query is to get all the fd's registered on this table.
// 			 */
// 			char* fd_query = (char*)malloc(strlen(table_name) + 50);
// 			strcpy(fd_query, "select * from fd where table_name = '");
// 			strcat(fd_query, table_name);
// 			strcat(fd_query, "' ;");

// 			FDTable = exec_simple_query(fd_query, 2);
// 			// res_printTupleTable(FDTable);


// 			struct Row* row;
// 			struct Header* header;
// 			char* fd_name;
// 			char* lhs;
// 			char* rhs;
// 			char** lhs_arr;
// 			char** rhs_arr;
// 			char* check_query;

// 			int nlhs;
// 			int nrhs;
// 			row = FDTable->head;
// 			header = HeaderTable->header;
// 			while(row != NULL){

// 				fd_name = row->values[0];
// 				lhs = row->values[2];
// 				rhs = row->values[3];

				
// 				lhs_arr = parse_fd(lhs, &nlhs);
// 				rhs_arr = parse_fd(rhs, &nrhs);


// 				check_query = fd_check_query(values,header->names, header->nvalid, lhs_arr, nlhs, rhs_arr, nrhs,table_name);

// 				CheckTable = exec_simple_query(check_query, 1);

// 				if(CheckTable->num_rows != 0){
// 					printf("[%d/%d] INVALID INSERT: FD(%s of %s) : %s -> %s Failed\n", i+1 , num_queries ,  fd_name , table_name , lhs, rhs);
// 					execute = false;
// 					break;
// 				}

// 				row = row->next;
// 			}

// 			//free malloced memory + returned malloced memory.
// 			free(select_query);
// 			free(fd_query);
// 			free(HeaderTable);
// 			free(FDTable);

//         }
//         else if(mode == 2){
//             int j = 0;
//             int lhs_num;
//             char* lhs = (char*)malloc( strlen(values[2]) + 1);
//             for(int i = 0; i < strlen(values[2]); i++){
//                 lhs[j++] = values[2][i];
//             }
//             lhs[j] = '\0';
//             char** lhs_arr = parse_fd(lhs, &lhs_num);


//             j = 0;
//             int rhs_num;
//             char* rhs = (char*)malloc( strlen(values[3]) + 1);
//             for(int i = 0; i < strlen(values[3]); i++){
//                 rhs[j++] = values[3][i];
//             }
//             rhs[j] = '\0';
//             char** rhs_arr = parse_fd(rhs, &rhs_num);

//             char* trigger_query = create_trigger(table_name, lhs_arr, lhs_num, rhs_arr, rhs_num );

//             strcat(token,";");
//             strcat(token,trigger_query);


//             free(lhs);
//             free(rhs);
//             free(lhs_arr);
//             free(rhs_arr);
//             free(trigger_query);
//         }

// 		if(!execute) continue;

// 		char* orig_query = (char*)malloc(strlen(token)+2);
// 		strcpy(orig_query, token);
// 		strcat(orig_query, ";");

// 		exec_simple_query(orig_query, 0);

// 		//free malloced memory
// 		free(orig_query);
// 	}

// 	//free malloced memory
// 	free(table_name);

// }





int main(){

    char* lhs[] = {"a","c"};
    char* rhs[] = {"b","d","e"};
    

    char* tq = create_trigger("test",lhs,2,rhs,3);
    printf("%s\n",tq);
}