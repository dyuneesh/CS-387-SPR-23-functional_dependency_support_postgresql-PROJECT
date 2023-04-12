# IDEA1

Idea is to modify  / overload the PortalRun() or exec_simple_query() or etc etc functions so that they "RETURN" 

either tuples (select * ) 
OR
number of tuples for a select query with a condition. Then in an insert, if we find that the count is > 0 ..... then we reject that query...


Call Trace for a simple select query:

`PostgresMain()`[has a loop reading queries, calls `exec_simple_query()`]
|--`exec_simple_query(&query_string)` (void)
|--calls parse_analyse_rewrite.....query
    |--Then sets Destination formats, creates Portal with some options etc etc
    |--`PortalRun()` (bool)
        |--nprocessed = `PortalRunSelect(portal, true, count, dest)`; (ret int)
                |--`ExecutorRun()` (void)
                    |--`StandardExecutorRun()` (void)
                        |--`ExecutePlan()` (void)
                            |--slot = `ExecProcNode(planstate);`  (slot has the tuple)



First I changed the return value of `exec_simple_query` to return number of tuples processed if a select query. OR 0.
Also added extra argument to `PortalRun()` to store the number of tuples processed.

But we need to stop the output to terminal when we do these kind of select queries where we only need a count.

The header is printed inside `StandardExecutorRun()` at 
```

	/*
	 * startup tuple receiver, if we will be emitting tuples
	 */
	estate->es_processed = 0;

	sendTuples = (operation == CMD_SELECT ||
				  queryDesc->plannedstmt->hasReturning);

	if (sendTuples)
		dest->rStartup(dest, operation, queryDesc->tupDesc);
```
Then it sends the `sendTuples` parameter to `ExecutorPlan()` 
and if `sendTuples` is `True` , it sends tuples (data tuples) to reciever.

```
		if (sendTuples)
		{
			/*
			 * If we are not able to send the tuple, we assume the destination
			 * has closed and no more tuples can be sent. If that's the case,
			 * end the loop.
			 */
			if (!dest->receiveSlot(slot, dest))
				break;
		}
```

-->So I think if we turn `sendTuples` to False; then it wont print to terminal.
-->So need to change `ExecuteRunStandard()` code to set `sendTuples = False` which infact needs some signal by `ExecuteRun()` to set that flag
and it needs flag from `PortalRunSelect()` <---`PortalRun()` <----- `exec_simple_query()`



# THE MODIFIED CALL SEQUENCE IS.

|--INT `exec_simple_query(&query_string,fd_query?)`
    |--bool `PortalRun(........., ** int* count **)`--->fetches result into count.
        |--nprocessed = `int PortalRunSelect(portal, true, count, dest, **bool block_output**)`;
                |--void `ExecutorRun(......, ** bool block_output **)`  passes the flag to child.
                    |--void `StandardExecutorRun(...., ** bool block_output **)` ``if(flag) sendTuples = false`` --> pass to child.
                        |--void `ExecutePlan(......, sendTuples, ....)`
                            |--slot = `ExecProcNode(planstate);`  (slot has the tuple)



so if we set fd_query = False, then exec_simple_query returns 0
else PortalRun() sets the *tuple_count = nprocessed, hence exec_simple_query() returns tuple_count_select.




## About the structure of slot

### QUERY DESCRIPTOR.

in `standardExecutorRun()` there is this `queryDesc`  which has all info about the query..... it also has the column names of the table.

Lokking at the definitions of the `queryDesc` struct, we can see that we can acess the attributes(column names) by using

```
queryDesc->TupDesc->attrs[n] /* and n can be found using */
queryDesc->TupDesc->nattrs

/*
each attribute has a @attname field
and @atttypid field. @attlen etc etc ....
Basically all pg_attribute fields.

I guess the typeids are standard for string int etc.. so maybe we dont need table acesses again to fetch these.
we can hard code then for now.
*/

```

### ACTUAL DATA TUPLES.

TupleTableSlot* slot;

```
// tts_ops: containes some fucntions.
// tts_tupleDescriptor: ITS JUST THE TUPLE DESCRIPTOR WE'VE SEEN PREVIOSULY.
// tts_values: current per attribute values..... its a Datum* (Datum = a 4 byte pointer kindof)

/*
so **tts_values gives attrs? right
*/

// tts_tid
// tts_tableOid: table oid of the tuple


/*
Now the slot actually doesnt contain the values....
*tts_values is zero!!

They actually get filled when we do this dest->receiverSlot()....

debugging into it, we find that it goes to printtup.c /backend/acess/common/ then it has some printing functions..
but it calls slot_getattr() in tuptable.h... then if not valid i.e, not filled yet

it calls slot_getsomeattrs()  ...... 
-->slot_getsomeattrsint()-->  .....backend/executor/execTuple.c
    here it uses the tts_ops functions..

-->slot->tts_ops->getsomeattrs(slot,nattrs)  FILLS UP THE FIRST n attrs.
*/


SO WE NEED TO CALL THAT FUNCTION IF WE TRY TO FETCH TUPLES.

```






### HANDLING DATATYPES OF VALUES..... 

```

backend> insert into d values ( -29 , 'police' , '{ 22 , 23 , 24 }' , '{"kim","jong"}' , 100.25);
Number of Tuples(a): 6
backend> select * from d;
         1: a   (typeid = 23, len = 4, typmod = -1, byval = t)
         2: b   (typeid = 1043, len = -1, typmod = 14, byval = f)
         3: c   (typeid = 1007, len = -1, typmod = -1, byval = f)
         4: d   (typeid = 1015, len = -1, typmod = 14, byval = f)
         5: e   (typeid = 1700, len = -1, typmod = 327686, byval = f)
        ----
         1: a = "-29"   (typeid = 23, len = 4, typmod = -1, byval = t)
         2: b = "police"        (typeid = 1043, len = -1, typmod = 14, byval = f)
         3: c = "{22,23,24}"    (typeid = 1007, len = -1, typmod = -1, byval = f)
         4: d = "{kim,jong}"    (typeid = 1015, len = -1, typmod = 14, byval = f)
         5: e = "100.25"        (typeid = 1700, len = -1, typmod = 327686, byval = f)
        ----
Number of Tuples(a): 6
backend> select c[1] from d;
         1: c   (typeid = 23, len = 4, typmod = -1, byval = t)
        ----
         1: c = "22"    (typeid = 23, len = 4, typmod = -1, byval = t)
        ----
Number of Tuples(a): 6
backend> select d[1] from d;
         1: d   (typeid = 1043, len = -1, typmod = 14, byval = f)
        ----
         1: d = "kim"   (typeid = 1043, len = -1, typmod = 14, byval = f)
        ----
Number of Tuples(a): 6
backend> 



We can hardcode the datatypes of string , int , array , numeric etc i think.
We need array beacuse we store fd lhs rhs in an array.




And in the printtup.c->debugtup() function there is a way to convert the attribute into a string value.
ex all the types get converted into string.... we can use that and convert it back to our required type.
"-29"
"police"
"{22,23,24}"
"{kim,jong}"
"100.25"

```




## LATER ..... NEED TO HANDLE STRING...COMMAS INSIDE STRINGS ETC ETC...
backend> insert into d values ( 1 , 'fool' , '{56,42}' , '{"ab,d","vk"}' , 5.65 );
Number of rows: 2
Number of attributes: 5
a       b       c       d       e
--------------------------------------------------------------------
1       fool    {56,42} {"ab,d",vk}     5.65
-29     police  {22,23,24}      {kim,jong}      100.25
--------------------------------------------------------