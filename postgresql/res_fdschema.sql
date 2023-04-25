CREATE TABLE FD(
    name varchar(32) NOT NULL,
    table varchar(32) NOT NULL,
    lhs varchar(20)[],
    rhs varchar(20)[],
    unique(name, table)
)