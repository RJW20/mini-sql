# 002_insert_select
# Tests basic INSERT statements

CREATE TABLE t (int INT, real REAL, text TEXT(5));

# supported types
INSERT INTO t VALUES (1, 0.1, "one"), (2, 0.2, "two");
SELECT * FROM t;

# different insert order
INSERT INTO t (text, real, int) VALUES ("three", 0.3, 3), ("four", 0.4, 4);
SELECT * FROM t;