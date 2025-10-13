# 101_invalid_create
# Tests invalid CREATE statements

# duplicate table
CREATE TABLE t1 (int INT);
CREATE TABLE t1 (real REAL);

# missing table name
CREATE TABLE (int INT, real REAL, text TEXT(16));

# missing table columns
CREATE TABLE t2;

# missing column type argument
CREATE TABLE t2 (text TEXT);

# invalid column types
CREATE TABLE t2 (text STRING);