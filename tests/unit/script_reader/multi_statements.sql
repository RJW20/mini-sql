CREATE TABLE users(id INT, name TEXT);
SELECT * FROM users;
INSERT INTO users VALUES(1, "Alice");
UPDATE users SET name = "Bob" WHERE id = 1;
DELETE FROM users;