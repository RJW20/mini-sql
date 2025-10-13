# 001_create
# Tests basic CREATE statements

# supported types
CREATE TABLE t1 (int INT);
CREATE TABLE t2 (real REAL);
CREATE TABLE t3 (text TEXT(16));

# basic combinations
CREATE TABLE t4 (int INT, real REAL, text TEXT(16));
CREATE TABLE t5 (text1 TEXT(16), text2 TEXT(64), text3 TEXT(256));