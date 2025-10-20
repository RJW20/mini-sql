# 004_delete
# Tests basic DELETE statements

CREATE TABLE t (int INT, real REAL, text TEXT(5));

# empty table
DELETE FROM t;

INSERT INTO t VALUES (1, 0.1, "one"), (2, 0.2, "two");

# delete all
DELETE FROM t;
SELECT * FROM t;