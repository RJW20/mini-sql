# 002_select
# Tests basic SELECT statements

CREATE TABLE t (int INT, real REAL, text TEXT(5));

# empty table
SELECT * FROM t;

INSERT INTO t VALUES (1, 0.1, "one"), (2, 0.2, "two");

# all combinations
SELECT * FROM t;
SELECT int FROM t;
SELECT real FROM t;
SELECT text FROM t;
SELECT int, real FROM t;
SELECT real, int FROM t;
SELECT int, text FROM t;
SELECT text, int FROM t;
SELECT real, text FROM t;
SELECT text, real FROM t;
SELECT int, real, text FROM t;
SELECT int, text, real FROM t;
SELECT real, text, int FROM t;
SELECT real, int, text FROM t;
SELECT text, int, real FROM t;
SELECT text, real, int FROM t;

# repeated selection in same query
SELECT int, real, text, real, int FROM t;