# Mini-SQL

Mini-SQL is a small, embeddable SQL database engine written in modern C++. It
is loosely inspired by SQLite and was built to explore how real database
engines work internally, while remaining clean, readable, and practical.

The engine supports a useful subset of SQL, persistent on-disk storage, and
indexed access via B+ trees, packaged as a C++ library with no external
dependencies.

## Features

- SQL parsing and execution
- Persistent on-disk storage (`.db` file per database)
- Multiple tables per database
- B+ tree–backed storage engine
- Primary key indexing
- Simple query planner with primary-key optimisations
- Page-based storage engine
- Cross-platform (Linux, macOS, Windows)
- No external dependencies
- Written in C++17

## Using Mini-SQL

### Requirements
- CMake $\ge$ 3.15
- C++17-compatible compiler

### Installation
You can either download and extract a release archive from the GitHub releases
page or clone the repository `git clone https://github.com/RJW20/mini-sql.git`.
Then build and install the library:
```
cmake -S . -B build
cmake --build build
cmake --install build
```
By default, this builds a static library and installs to your system’s standard
CMake prefix (e.g. /usr/local on Linux/macOS). You can build a shared library by enabling `BUILD_SHARED_LIBS=ON` or specify a custom install location using
`CMAKE_INSTALL_PREFIX`:
```
cmake -S . -B build \
  -DBUILD_SHARED_LIBS=ON \
  -DCMAKE_INSTALL_PREFIX=/path/to/install
cmake --build build
cmake --install build
```

### Linking
Once installed, `mini-sql` can be found using `find_package`. In your project's
CMakeLists.txt:
```
cmake_minimum_required(VERSION 3.15)
project(myproject LANGUAGES CXX)

find_package(mini-sql CONFIG REQUIRED)

add_executable(myproject main.cpp)
target_link_libraries(myproject PRIVATE minisql::minisql)
```
If you installed `mini-sql` to a non-standard location, tell CMake where to
find it:
```
cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/install
```
If you created a shared build for `mini-sql` and are on Windows ensure to copy
`minisql.dll` from the install to your projects `build` folder.

Both static and shared builds and linking are supported and tested
automatically via GitHub actions on every push.

## API

### Create a Connection
To create a new or connect to an already existing database:
```
#include <minisql/connection.hpp>

minisql::Connection connection{"/* database name */.db"};
```

### Execute SQL
To execute SQL (`CREATE`, `INSERT`, `UPDATE`, `DELETE`, `DROP`) on a database
with a preestablished connection:
```
connection.exec(/* CREATE statement etc */);
```

### Query
To obtain rows from a `TABLE` inside a database:
```
for (auto& row : connection.query(/* SELECT statement */)) {
    // print row etc
}
```
or if you dislike STL iterators:
```
auto rs = connection.query(/* SELECT statement */);
while (rs.next()) {
    auto& r = rs.current();
    // print row etc
}
```

### Scripts
A `ScriptReader` class is available for ease of executing multiple SQL
statements. A script may contain one or more SQL statements, separated by
semicolons. Comments may be marked by `#` and run to the end of the current
line. To execute such a script:
```
#include <minisql/script_reader.hpp>

minisql::ScriptReader script{"script.sql"};
while (auto statement = script.next()) {
    connnection.exec(*stmt);
}
```

## Supported SQL
For the following examples placeholder names (denoted with <>) should be
wholly replaced (no <>) with your own identifiers.

### `CREATE`
The `CREATE` statement is used to create a new `TABLE` in a database:
```
CREATE TABLE <table_name> (<column_name> <data_type>, ...);
```
Supported data types can be found [here](#data-types).

A `TABLE` may not:
- share its name with any others in the same database
- have duplicate column names
- exceed **508 bytes** in width (or **512** if specifying a `PRIMARY KEY`, see below)

### `PRIMARY KEY`
The `PRIMARY KEY` constraint may be specified for any column in a `TABLE`:
```
CREATE TABLE <table_name> (<column_name> <data_type>, ..., PRIMARY KEY(<column_name>));
```
A `TABLE` may only have one `PRIMARY KEY`. Values in the specified column must
be unique and cannot be updated.

### `SELECT`
The `SELECT` statement is used to select specified columns from a `TABLE`:
```
SELECT <column_name>, ... FROM <table_name>;
```
or to `SELECT` all columns in the order they were defined:
```
SELECT * FROM <table_name>;
```
Rows may be filtered by appending a `WHERE` clause (see below).

### `WHERE`
A `WHERE` clause is used to filter rows in `SELECT`, `UPDATE` and `DELETE`
statements:
```
SELECT * FROM <table_name> WHERE <condition>;
```
A condition has the form:
```
<column_name> <operator> <value>
```
Supported operators are =, !=, >, >=, <, <= (comparison for
`TEXT` is in lexicographical order).

Multiple conditions may be chained together with `AND` operators:
```
SELECT * FROM <table_name> WHERE <condition> AND ...;
```

### `INSERT`
The `INSERT` statement is used to insert new rows into a `TABLE`:
```
INSERT INTO <table_name> VALUES (<value>, ...), ...;
```
The order of the values making up rows must match the order the columns were
defined in, and every column in a row must have a value. To `INSERT` rows with
values in a different order you must first specify the order of the columns:
```
INSERT INTO <table_name> (<column_name>, ...) VALUES (<value>, ...), ...;
```

### `UPDATE`
The `UPDATE` statement is used to modify existing rows in a `TABLE`:
```
UPDATE <table_name> SET <modification> WHERE <condition>;
```
A modification can have multiple forms depending on a column's data type:
- `INT`, `REAL`, `TEXT`:
  ```
  <column_name> = <value>
  <column_name_1> = <column_name_2>
  ```
- `INT`, `REAL`
  ```
  <column_name> = <column_name> <operator> <value>
  <column_name_1> = <column_name_1> <operator> <column_name_2>
  ```
`<column_name_1>` and `<column_name_2>` must have the same data type. Supported
operators are $+$, $-$, /, *.

Multiple modifications may be chained together, and will be executed in the
order they are given:
```
UPDATE <table_name> SET <modification>, ... WHERE <condition>;
```
If the `WHERE` clause is omitted, all rows will be updated.

### `DELETE`
The `DELETE` statement is used to delete existing rows from a `TABLE`:
```
DELETE FROM <table_name> WHERE <condition>;
```
If the `WHERE` clause is omitted, all rows will be deleted.

### `DROP`
The `DROP` statement is used to `DROP` an existing `TABLE` from a database:
```
DROP TABLE <table_name>;
```

### Data Types
Mini-SQL supports 3 data types:

| Data Type | Stores                          | Accepts (auto-modifications)                         |
|-----------|---------------------------------|------------------------------------------------------|
| `INT`     | integer (32 bit)                | integers, doubles (truncated towards 0)              |
| `REAL`    | double precision float (64 bit) | integers, doubles                                    |
| `TEXT(n)` | text of size $n$                | double-quotes strings (padded/truncated to size $n$) |

## Architecture Overview
Mini-SQL follows a traditional relational database architecture, with a clear
separation between query execution, storage and resource management.

### Execution Model
SQL statements are executed using a lazy, iterator-based execution pipeline:
```
SQL Text
  -> Parser
  -> Validator
  -> Planner
  -> Iterator Tree
  -> Cursor
  -> Page-backed B+ Tree
  -> Row Stream
```
- The planner transforms validated SQL into an immutable tree of iterators.
- Each iterator exposes a `next()` interface and pulls rows from its child on
  demand (volcano-style execution).
- Rows are passed through the pipeline as lightweight row-views, allowing
  filtering and projection to avoid unnecessary materialisation.
- Full materialisation occurs only at the API boundary when results are
  returned to the user.

`SELECT`, `UPDATE` and `DELETE` statements share the same underlying execution
mechanisms, differing only in how produced rows are consumer.

The planner applies simple rule-based optimisations:
- Queries with `WHERE` predicates on the `PRIMARY KEY` are executed using an
  index scan instead of a full table scan.
- Equality predicates on the `PRIMARY KEY` are converted into bound index scans.
- Redundant or contradictory `PRIMARY KEY` constraints are simplified or
  eliminated during planning.

### Storage Engine
Mini-SQL uses a page-based storage engine with a fixed page size of
**4096 bytes**.
- Tables are stored as persistent, on-disk B+ trees, keyed by the `PRIMARY KEY`
  or an automatically assigned `row_id`.
- All B+ tree nodes reside directly on pages; node metadata and contents are
  read from and written to pages as needed.
- Leaf nodes store the underlying row data in a fixed-width layout, enabling
  efficient traversal and predictable storage behavior.
- B+ trees are fully persistent and are not rebuilt on startup.

### Buffer and Resource Management
Mini-SQL uses an explicit buffer pool to manage page access and lifetime.
- Pages are loaded from disk into in-memory frames on demand.
- The frame manager hands out lightweight FrameView handles, which provide
  scoped access to a frame.
- Each frame maintains a pin count indicating how many active users are
  currently referencing it.
- Pin counts are managed using RAII: when a FrameView (or a higher-level object
  using it, such as a B+ tree node) is destroyed, the pin count is
  automatically decremented.

Only frames with a pin count of zero are eligible for eviction:
- When a frame becomes unpinned, it is added to an LRU list for reuse.
- When a frame is selected for reuse, any dirty page it contains is flushed to
  disk before being overwritten.

In addition to the buffer pool, Mini-SQL maintains a persistent free-page
list:
- When pages are no longer needed (e.g. when a table is dropped and its B+ tree
  is disassembled), their page numbers are added to the free list.
- The free list is stored on disk as page-backed blocks, each containing a
  small header followed by a list of free page numbers.
- When allocating a new page, the engine first attempts to reuse a page from
  the free list; if the free list is empty, the database file is extended by
  one page.

This design ensures safe page reuse, predictable write-back behavior, and
efficient space management, while keeping resource lifetimes explicit and
deterministic.

### Database Access
Database connections reference shared database instances internally; multiple
connections to the same database file are supported, though the engine is
currently single-threaded and does not implement locking.

## Testing
To build tests, set the `BUILD_TESTS` flag during the configuration:
```
cmake -S . -B build -DBUILD_TESTS=ON
cmake --build build
```

### Integration Tests
There is extensive [integration testing](tests/integration/README.md) covering
SQL execution and storage behaviour. To run the integration tests:
```
./build/tests/integration/runner
```
By default, this runs all integration tests. Providing runtime arguments allows
selection of specific prefixes of scripts to run, with execution order
preserved. For example:
- `001` runs just script `001_<name>.sql`
- `001 002` runs scripts `001_<name>.sql` and `002_<name>.sql`
- `0` runs all scripts `0XX_<name>.sql`
- `0 1` runs all scripts `0XX_<name>.sql` and `1XX_<name>.sql`

All integration tests are run via GitHub actions on every push.

### Unit Tests
There are also [unit tests](tests/unit/) for a limited number of components.
Each unit test is built into its own executable and can be run using:
```
./build/unit/<test_name>
```
where `<test_name>` corresponds to the unit test source file.

## Limitations
- Single-threaded
- No joins
- No secondary indexes
- Fixed page sizes (**4096 bytes**)
- Maximum table width (**512 bytes**)

## Future Work
Possible future improvements include:
- Secondary index support
- Configurable page sizes
- Basic performance benchmarks