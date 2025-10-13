# 104_invalid_update
# Tests invalid UPDATE statements

CREATE TABLE t (
    int INT, int_2 INT, int_3 INT,
    real REAL, real_2 REAL, real_3 REAL,
    text TEXT(5), text_2 TEXT(5)
);

# missing table
UPDATE SET int = 1;

# fake table
UPDATE fake_t SET int = 1;

# missing column
UPDATE t SET = 1;

# fake column
UPDATE t SET fake_col = "extra_val";
UPDATE t SET int = fake_col;

# missing value
UPDATE t SET int;

# incorrect value type
UPDATE t SET int = "one";
UPDATE t SET int = int + "one";
UPDATE t SET int = int - "one";
UPDATE t SET int = int * "one";
UPDATE t SET int = int / "one";
UPDATE t SET real = "one";
UPDATE t SET real = real + "one";
UPDATE t SET real = real - "one";
UPDATE t SET real = real * "one";
UPDATE t SET real = real / "one";
UPDATE t SET text = 1;

# incompatible value type
UPDATE t SET int = real;
UPDATE t SET int = int + real;
UPDATE t SET int = int - real;
UPDATE t SET int = int * real;
UPDATE t SET int = int / real;
UPDATE t SET int = text;
UPDATE t SET int = int + text;
UPDATE t SET int = int - text;
UPDATE t SET int = int * text;
UPDATE t SET int = int / text;
UPDATE t SET real = int;
UPDATE t SET real = real + int;
UPDATE t SET real = real - int;
UPDATE t SET real = real * int;
UPDATE t SET real = real / int;
UPDATE t SET real = text;
UPDATE t SET real = real + text;
UPDATE t SET real = real - text;
UPDATE t SET real = real * text;
UPDATE t SET real = real / text;
UPDATE t SET text = int;
UPDATE t SET text = real;

# invalid operator
UPDATE t SET text = text + "more_text";
UPDATE t SET text = text - "more_text";
UPDATE t SET text = text * "more_text";
UPDATE t SET text = text / "more_text";
UPDATE t SET text = text + text_2;
UPDATE t SET text = text - text_2;
UPDATE t SET text = text * text_2;
UPDATE t SET text = text / text_2;

# unsupported modifications
UPDATE t SET int = int_2 + 1;
UPDATE t SET int = int_2 - 1;
UPDATE t SET int = int_2 * 1;
UPDATE t SET int = int_2 / 1;
UPDATE t SET int = int_2 + int_3;
UPDATE t SET int = int_2 - int_3;
UPDATE t SET int = int_2 * int_3;
UPDATE t SET int = int_2 / int_3;
UPDATE t SET real = real_2 + 0.1;
UPDATE t SET real = real_2 - 0.1;
UPDATE t SET real = real_2 * 0.1;
UPDATE t SET real = real_2 / 0.1;
UPDATE t SET real = real_2 + real_3;
UPDATE t SET real = real_2 - real_3;
UPDATE t SET real = real_2 * real_3;
UPDATE t SET real = real_2 / real_3;