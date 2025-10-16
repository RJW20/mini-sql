# 101_invalid_create
# Tests invalid CREATE statements

# missing table name
CREATE TABLE (int INT, real REAL, text TEXT(16));

# duplicate table
CREATE TABLE t1 (int INT);
CREATE TABLE t1 (real REAL);

# too long table name
CREATE TABLE t12345678912345678912345678912345 (int INT);

# missing table columns
CREATE TABLE t2;

# duplicate columns
CREATE TABLE t2 (int INT, int INT);

# missing column type argument
CREATE TABLE t2 (text TEXT);

# invalid column types
CREATE TABLE t2 (text STRING);

# column types create too wide of a table
CREATE TABLE t2 (text TEXT(509));