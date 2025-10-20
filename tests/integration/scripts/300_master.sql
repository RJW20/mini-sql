# 300_master
# Tests statements involving the reserved master table

# CREATE
CREATE TABLE master (name TEXT(16), sql TEXT(256), root INT, next_rowid INT);

# SELECT
CREATE TABLE t1 (int INT);
CREATE TABLE t2 (real REAL);
CREATE TABLE t3 (text TEXT(16));
SELECT * FROM master;

# INSERT
INSERT INTO master VALUES (1);

# UPDATE
UPDATE master SET name = 1;

# DELETE
DELETE FROM master;