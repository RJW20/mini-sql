# 007_where
# Tests basic WHERE commands

CREATE TABLE t (int INT, real REAL, text TEXT(5));
INSERT INTO t VALUES (1, 0.1, "a"), (2, 0.2, "aa"), (3, 0.3, "aaa");

# =
SELECT * FROM t WHERE int = 1;
SELECT * FROM t WHERE real = 0.1;
SELECT * FROM t WHERE text = "a";

# !=
SELECT * FROM t WHERE int != 1;
SELECT * FROM t WHERE real != 0.1;
SELECT * FROM t WHERE text != "a";

# >
SELECT * FROM t WHERE int > 1;
SELECT * FROM t WHERE real > 0.1;
SELECT * FROM t WHERE text > "a";

# >=
SELECT * FROM t WHERE int >= 1;
SELECT * FROM t WHERE real >= 0.1;
SELECT * FROM t WHERE text >= "a";

# <
SELECT * FROM t WHERE int < 3;
SELECT * FROM t WHERE real < 0.3;
SELECT * FROM t WHERE text < "aaa";

# <=
SELECT * FROM t WHERE int <= 3;
SELECT * FROM t WHERE real <= 0.3;
SELECT * FROM t WHERE text <= "aaa";

# empty result
SELECT * FROM t WHERE int = 4;
SELECT * FROM t WHERE real > 0.3;
SELECT * FROM t WHERE text < "a";