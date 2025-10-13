# 201_multi_update
# Tests compound UPDATE statements

CREATE TABLE t (a INT, b INT, c INT);
INSERT INTO t VALUES (1, 10, 100), (2, 20, 200), (3, 30, 300);

# update order (happens sequentially not simultaneously)
UPDATE t SET a = a + 1, b = b + a, c = c + b;
SELECT * FROM t;
UPDATE t SET c = c - b, b = b - a, a = a - 1;
SELECT * FROM t;