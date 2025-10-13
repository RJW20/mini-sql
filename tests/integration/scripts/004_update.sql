# 004_update
# Tests basic UPDATE statements

# simple set
CREATE TABLE t (int INT, real REAL, text TEXT(5));
INSERT INTO t VALUES (1, 0.1, "one"), (2, 0.2, "two");
UPDATE t SET int = 3;
SELECT * FROM t;
UPDATE t SET real = 0.3;
SELECT * FROM t;
UPDATE t SET text = "three";
SELECT * FROM t;

# all INT modifications
CREATE TABLE t_int (ones INT, tens INT);
INSERT INTO t_int VALUES (1, 10), (2, 20);
UPDATE t_int SET ones = ones + 1;
SELECT * FROM t_int;
UPDATE t_int SET ones = ones - 1;
SELECT * FROM t_int;
UPDATE t_int SET ones = ones * 2;
SELECT * FROM t_int;
UPDATE t_int SET ones = ones / 2;
SELECT * FROM t_int;
UPDATE t_int SET tens = tens + ones;
SELECT * FROM t_int;
UPDATE t_int SET tens = tens - ones;
SELECT * FROM t_int;
UPDATE t_int SET tens = tens * ones;
SELECT * FROM t_int;
UPDATE t_int SET tens = tens / ones;
SELECT * FROM t_int;

# all REAL modifications
CREATE TABLE t_real (tenths REAL, hundredths REAL);
INSERT INTO t_real VALUES (0.1, 0.01), (0.2, 0.02);
UPDATE t_real SET tenths = tenths + 0.1;
SELECT * FROM t_real;
UPDATE t_real SET tenths = tenths - 0.1;
SELECT * FROM t_real;
UPDATE t_real SET tenths = tenths * 0.2;
SELECT * FROM t_real;
UPDATE t_real SET tenths = tenths / 0.2;
SELECT * FROM t_real;
UPDATE t_real SET hundredths = hundredths + tenths;
SELECT * FROM t_real;
UPDATE t_real SET hundredths = hundredths - tenths;
SELECT * FROM t_real;
UPDATE t_real SET hundredths = hundredths * tenths;
SELECT * FROM t_real;
UPDATE t_real SET hundredths = hundredths / tenths;
SELECT * FROM t_real;

# all TEXT modifications
CREATE TABLE t_text (text1 TEXT(5), text2 TEXT(5));
INSERT INTO t_text VALUES ("a", "b"), ("c", "d");
UPDATE t_text SET text2 = text1;
SELECT * FROM t_text;