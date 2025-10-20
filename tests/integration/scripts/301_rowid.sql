# 301_rowid
# Tests statements involving the reserved rowid column

# defining a rowid column
CREATE TABLE t (rowid INT);
CREATE TABLE t (rowid INT, PRIMARY KEY(rowid));

CREATE TABLE t (int INT);

# inserting with rowid
INSERT INTO t (rowid, int) VALUES (1, 1);

INSERT INTO t VALUES (1), (2);

# selecting rowid
SELECT rowid FROM t;

# conditioning on rowid
SELECT int FROM t WHERE rowid = 1;

# modifying rowid
UPDATE t SET rowid = 2;