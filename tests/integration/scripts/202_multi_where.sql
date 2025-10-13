# 202_multi_where
# Tests compound WHERE commands

CREATE TABLE t (a INT, b INT);
INSERT INTO t VALUES
    (1, 10), (1, 20), (1, 30),
    (2, 10), (2, 20), (2, 30),
    (3, 10), (3, 20), (3, 30);

# conditions on separate columns
SELECT * FROM t WHERE a = 1 AND b != 20;
SELECT * FROM t WHERE b < 30 AND a > 1;

# = & != on the same column
SELECT * FROM t WHERE a = 1 AND a != 1;

# = & not in range < > on the same column
SELECT * FROM t WHERE a = 1 AND a >= 2 AND a < 3;