# 106_invalid_primary
# Tests invalid use of PRIMARY KEY declarations

# missing column
CREATE TABLE t (int INT, real REAL, text TEXT(5), PRIMARY KEY());

# fake column
CREATE TABLE t (int INT, real REAL, text TEXT(5), PRIMARY KEY(fake_col));

CREATE TABLE t_int (int INT, PRIMARY KEY(int));
CREATE TABLE t_real (real REAL, PRIMARY KEY(real));
CREATE TABLE t_text (text TEXT(5), PRIMARY KEY(text));

# duplicate primary keys
INSERT INTO t_int VALUES (1), (1);
INSERT INTO t_real VALUES (0.1), (0.1);
INSERT INTO t_text VALUES ("one"), ("one");

# primary key update
UPDATE t_int SET int = 2;
UPDATE t_real SET real = 0.1;
UPDATE t_text SET text = "two";