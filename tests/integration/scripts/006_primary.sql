# 006_primary
# Tests basic use of PRIMARY KEY declarations

# supported types
CREATE TABLE t1 (int INT, PRIMARY KEY(int));
CREATE TABLE t2 (real REAL, PRIMARY KEY(real));
CREATE TABLE t3 (text TEXT(16), PRIMARY KEY(text));

# variable position in declared order
CREATE TABLE t4 (int INT, real REAL, text TEXT(5), PRIMARY KEY(int));
CREATE TABLE t5 (int INT, real REAL, text TEXT(5), PRIMARY KEY(real));
CREATE TABLE t6 (int INT, real REAL, text TEXT(5), PRIMARY KEY(text));