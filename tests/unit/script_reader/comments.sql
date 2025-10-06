# leading comment
CREATE TABLE t(a INT); # trailing comment
INSERT INTO t VALUES(1);   # another trailing comment
# full line comment
SELECT * FROM t;#no space after comment marker
INSERT INTO t VALUES(2);#comment with ; in it ;;;;
SELECT * FROM t;#comment without newline at EOF