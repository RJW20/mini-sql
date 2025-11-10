# 107_invalid_where
# Tests invalid WHERE commands

CREATE TABLE t (int INT, real REAL, text TEXT(5));

# missing column
SELECT * FROM t WHERE = 1;

# fake column
SELECT * FROM t WHERE fake_col = 1;

# missing value
SELECT * FROM t WHERE int;

# incorrect value type
SELECT * FROM t WHERE int = "a";
SELECT * FROM t WHERE int != "a";
SELECT * FROM t WHERE int > "a";
SELECT * FROM t WHERE int >= "a";
SELECT * FROM t WHERE int < "a";
SELECT * FROM t WHERE int <= "a";
SELECT * FROM t WHERE real = "a";
SELECT * FROM t WHERE real != "a";
SELECT * FROM t WHERE real > "a";
SELECT * FROM t WHERE real >= "a";
SELECT * FROM t WHERE real < "a";
SELECT * FROM t WHERE real <= "a";
SELECT * FROM t WHERE text = 1;
SELECT * FROM t WHERE text != 1;
SELECT * FROM t WHERE text > 1;
SELECT * FROM t WHERE text >= 1;
SELECT * FROM t WHERE text < 1;
SELECT * FROM t WHERE text <= 1;