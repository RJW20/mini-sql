# 005_drop
# Tests DROP statements

# empty table
CREATE TABLE t (int INT, real REAL, text TEXT(5));
DROP TABLE t;

# populated table
CREATE TABLE t (int INT, real REAL, text TEXT(5));
INSERT INTO t VALUES
    (1, 0.1, "one"), (2, 0.2, "two"), (3, 0.3, "three"), (4, 0.4, "four");
DROP TABLE t;