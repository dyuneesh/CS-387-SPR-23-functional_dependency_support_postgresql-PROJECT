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


About the structure of slot.



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