# 103_invalid_select
# Tests invalid SELECT statements

CREATE TABLE t (int INT, real REAL, text TEXT(5));

# missing table
SELECT * FROM;

# fake table
SELECT * FROM fake_t;

# missing column(s)
SELECT FROM t;

# fake column
SELECT fake_col FROM t;