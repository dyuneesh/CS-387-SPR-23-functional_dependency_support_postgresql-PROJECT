create table FD(
    id serial primary key,
    name varchar(50) not null,
    lhs varchar(50)[] not null,
    rhs varchar(50)[] not null,
);

create table A(
    name varchar(50) primary key,
    age int,
    height int,
    weight int,
    city varchar(50),
    state varchar(50),
    country varchar(50),
);