CREATE TABLE FD1(
    fd_name varchar(32) NOT NULL,
    table_name varchar(32) NOT NULL,
    lhs varchar(20)[],
    rhs varchar(20)[],
    unique(fd_name, table_name)
);

CREATE TABLE FD2(
    fd_name varchar(32) NOT NULL,
    table_name varchar(32) NOT NULL,
    lhs varchar(20)[],
    rhs varchar(20)[],
    unique(fd_name, table_name)
);

-- CREATE TABLE test(
--     a int,
--     b int,
--     c int
-- );