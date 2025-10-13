# 102_invalid_insert
# Tests invalid INSERT statements

CREATE TABLE t (int INT, real REAL, text TEXT(5));

# missing table
INSERT INTO VALUES (1, 0.1, "one");

# fake table
INSERT INTO fake_t VALUES (1);

# missing column
INSERT INTO t (int, real) VALUES (1, 0.1);

# extra (valid) column
INSERT INTO t (int, real, text, int) VALUES (1, 0.1, "one", 1);

# fake column
INSERT INTO t (int, real, text, fake_col) VALUES (1, 0.1, "one", "extra_val");

# missing value
INSERT INTO t VALUES (1, 0.1);

# extra value
INSERT INTO t VALUES (1, 0.1, "one", "extra_val");

# incorrect value type
INSERT INTO t VALUES ("one", 0.1, "one");
INSERT INTO t VALUES (1, "one", "one");
INSERT INTO t VALUES (1, 0.1, 1);