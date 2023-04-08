# FUNCTIONAL DEPENDENCY IN POSTGRESQL.

## IDEA[1] : Creating a table to store FD's

### TABLE FORMAT

FDTABLE (AB -> DEF )


``table_name``
``lhs`` (concated attributes of lhs of FD : "A|B")
``rhs`` (concated attrs of rhs of FD : "D|E|F" )



When a entry is added to the FD table; Then create a table to store counts whose attributes are

```lhs```
```rhs```
```count```
-A **CHECK CONSTRAINT** WHERE COUNT <= 1 (OR) a uniq constraint , but no need of count here.