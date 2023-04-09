# FUNCTIONAL DEPENDENCY IN POSTGRESQL.

## IDEA[1] : Creating a table to store FD's

### TABLE FORMAT

FDTABLE (AB -> DEF )


``table_name``
``lhs`` (concated attributes of lhs of FD : "A|B")
``rhs`` (concated attrs of rhs of FD : "D|E|F" )

:STORAGE of lhs / rhs has 2 possibilities 
    -as a concated string.
    -as a ARRAY[.]

Ex: 
    INSERT INTO FD VALUES ('department',{'A','B'},{'D','E','F'});

need to see which enables the queries to be  smaller. or more generalisable / robust.


When a entry is added to the FD table; Then create a ``**Trigger**`` which checks the concerning table before any INSERT / UPDATE.
-->Also when u delete a entry from FD table..... Then try to delete the Trigger as a later work.

TRiggers : https://www.postgresql.org/docs/current/sql-createtrigger.html
