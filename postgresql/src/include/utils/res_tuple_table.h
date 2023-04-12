/*
THIS CONTAINS THE STRUCT TO STORE TUPLES RETURNED BY A SELECT QUERY.

the rows of the result are chained like a linked list.
whereas, each row is like an arrays of pointers to the values.
pointers point to character strings....., and we can retrieve the underlying datatype by appropriate string to XXX functions.

OR we can directly convert the target itself to a string and , compare it with the tuple attributes.

Usages: mostly in the call sequence leading to ExecPlan()

src/include/utils/res_tuple_table.h

*/

#include <stdbool.h>


#ifndef RES_TUPLE_TABLE_H
#define RES_TUPLE_TABLE_H


struct Header
{
    int nvalid;         // number of valid attributes i.e, number of attributes filled yet
    char** names;       // array of char*'s denoting the names of the attributes
    int* type_ids;      // integer array of typeids of each column.
};

struct Row
{
    int nvalid;
    char** values;
    struct Row *next;      // pointer to the next row
};


struct TupleTable
{
    int num_rows;
    int nattrs;
    int mode;       /* mode in which the query was executed. see exec_simple_query() for what each mode means. */
    int nprocessed;

    struct Header *header;
    struct Row *head;
};

/*
-------------------------------------------------------------------------------
                        ROW OPERATIONS                                        |
-------------------------------------------------------------------------------
*/


// struct Row* res_makeRow(int n){
//     struct Row* r = (struct Row*)malloc(sizeof(struct Row));
//     r->nvalid = 0;
//     r->values = (char**)malloc(n * sizeof(char*));
//     r->next = NULL;
//     return r;
// }

// struct Header* res_makeHeader(int n){
//     struct Header* h = (struct Header*)malloc(sizeof(struct Header));
//     h->nvalid = 0;
//     h->names = (char**)malloc(n * sizeof(char*));
//     h->type_ids = (int*)malloc(n * sizeof(int));
//     return h;
// }


// /*
// We assume that space has already been allocated in the values array, when initialising the TupleTable.
// Just fill the values in the array.
// */
// void res_addValueToRow(struct Row* r, char* value){
//     r->values[r->nvalid++] = value;
// }

// void res_addNameToHeader(struct Header* h, char* name, int type_id){
//     h->names   [h->nvalid]   = name;
//     h->type_ids[h->nvalid++] = type_id;
// }


// void res_printRow(struct Row* r){
//     int i;
//     for(i = 0; i < r->nvalid; i++){
//         printf("%s\t", r->values[i]);
//     }
//     printf("\n");
// }

// void res_printHeader(struct Header* h){
//     int i;
//     for(i = 0; i < h->nvalid; i++){
//         printf("%s\t", h->names[i]);
//     }
//     printf("\n");
// }


// /*
// -------------------------------------------------------------------------------
//                         TUPLE TABLE OPERATIONS                                 |
// -------------------------------------------------------------------------------
// */

// struct TupleTable* res_makeTupleTable(int mode){
//     struct TupleTable* t = (struct TupleTable*)malloc(sizeof(struct TupleTable));
//     t->num_rows = 0;
//     t->nattrs = 0;
//     t->mode = mode;
//     t->nprocessed = 0;
//     t->header = NULL;
//     t->head = NULL;
//     return t;
// }

// void res_addHeaderTupleTable(struct TupleTable* t, struct Header* h){
//     t->header = h;
//     t->nattrs = h->nvalid;
// }

// bool res_isTupleTableEmpty(struct TupleTable *t)
// {
//     if(t->num_rows == 0)
//         return true;
//     else
//         return false;
// }


// /*
// Inserts the tuple at the start rather than at the end.
// For performance reasons.
// Because later, we anyway scan the whole table sequentially.
// */
// void res_TupleTableInsertRow(struct TupleTable*t, struct Row* r)
// {
//     if(res_isTupleTableEmpty(t))
//     {
//         t->head = r;
//     }
//     else
//     {
//         r->next = t->head;
//         t->head = r; 
//     }
//     t->num_rows++;
// }

// void res_TupleTableInsertValues(struct TupleTable*t, char** values)
// {
//     struct Row *r = (struct Row *)malloc(sizeof(struct Row));
//     r->values = values;
//     r->next = NULL;

//     if(res_isTupleTableEmpty(t))
//     {
//         t->head = r;
//     }
//     else
//     {
//         r->next = t->head;
//         t->head = r; 
//     }
//     t->num_rows++;
// }



// void res_printTupleTable(struct TupleTable *t)
// {

//     //---- HEADER -----
//     printf("Number of rows: %d\n", t->num_rows);
//     printf("Number of attributes: %d\n", t->nattrs);
//     printf("Attribute names: ");

//     res_printHeader(t->header);

//     printf("--------------------------------------------------------------------\n");

//     struct Row *r = t->head;
//     while(r != NULL)
//     {
//         res_printRow(r);
//         r = r->next;
//     }

//     printf("--------------------------------------------------------------------\n");
// }




extern struct Row* res_makeRow(int n);

extern struct Header* res_makeHeader(int n);


/*
We assume that space has already been allocated in the values array, when initialising the TupleTable.
Just fill the values in the array.
*/
extern void res_addValueToRow(struct Row* r, char* value);

extern void res_addNameToHeader(struct Header* h, char* name, int type_id);

extern void res_printRow(struct Row* r);

extern void res_printHeader(struct Header* h);

/*
-------------------------------------------------------------------------------
                        TUPLE TABLE OPERATIONS                                 |
-------------------------------------------------------------------------------
*/

extern struct TupleTable* res_makeTupleTable(int mode);

extern void res_addHeaderTupleTable(struct TupleTable* t, struct Header* h);

extern bool res_isTupleTableEmpty(struct TupleTable *t);



/*
Inserts the tuple at the start rather than at the end.
For performance reasons.
Because later, we anyway scan the whole table sequentially.
*/
extern void res_TupleTableInsertRow(struct TupleTable*t, struct Row* r);


extern void res_TupleTableInsertValues(struct TupleTable*t, char** values);


extern void res_printTupleTable(struct TupleTable *t);




#endif