# MiniSQL

## Create a Connection
```
minisql::Connection connection("blog.db");
```

## Execute SQL
```
connection.exec("CREATE TABLE posts(id INT PRIMARY KEY, title VARCHAR(16), body VARCHAR(128))");
```

## Simple Query
```
for (auto& row : connection.query("SELECT title, body FROM posts WHERE id < 10")) {
    // print row etc
}
```
or if you dislike STL iterators
```
auto cur = connection.query("SELECT * FROM posts");
while (cur.next()) {
    auto& r = cur.row();
    // print row etc
}
```

## Reuse SQL
```
auto stmt = connection.prepare("INSERT INTO posts(id, title, body) VALUES(?1, ?2, ?3)");
stmt.bind(1, 0);
stmt.bind(2, "Hello World");
stmt.bind(3, "This is my first post");
stmt.step();
stmt.reset();
```