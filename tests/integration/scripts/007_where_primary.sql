# 007_where_primary
# Tests basic WHERE commands on PRIMARY KEY

CREATE TABLE t (int INT, PRIMARY KEY(int));
INSERT INTO t VALUES (1), (2), (3);

# condition on primary
SELECT * FROM t WHERE int = 1;
SELECT * FROM t WHERE int != 1;
SELECT * FROM t WHERE int > 1;
SELECT * FROM t WHERE int >= 1;
SELECT * FROM t WHERE int < 3;
SELECT * FROM t WHERE int <= 3;

# empty result
SELECT * FROM t WHERE int = 4;