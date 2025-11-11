# Integration Tests for Mini-SQL
This directory contains the integration test suite for the Mini-SQL database engine. Integration tests verify end-to-end correctness by executing SQL scripts against the engine
and comparing their outputs to expected `.out` files.


## Directory Structure
```
tests/integration/
├── data/                       # Generated .csv datasets used in stress-tests
│   └── schema.json             # Defines generation and usage of stress-test datasets
├── output/                     # .out files, 1 per test
├── scripts/                    # .sql files, 1 per test
├── CMakeLists.txt              # Defines test build + generation
├── directory_iterable.hpp      # Yields scripts/.sql files by prefix
├── executor.hpp                # Runs scripts and returns string output
├── gen_data.py                 # Generates data/*.csv
├── gen_tests.py                # Generates 4XX_{test_name}.sql/.out from data/*.csv
└── runner.cpp                  # Runs all scripts and compares expected vs actual outputs
```

## How the tests work
Each integration test consists of a pair of files:
- `<name>.sql` - the SQL script to execute
- `<name>.out` - the expected output of running the script

The test runner:
1. Reads `.sql` files in `scripts/`
2. Executes the statements sequentially
3. Captures output one line at a time (a row, affected counts, errors)
4. Compares with the corresponding line in the `.out` file in `output/`

A test passes if and only if the actual output matches exactly.

### SQL File Format (`.sql`)
Each file contains one or more SQL statements, separated by semicolons. Comments are marked by `#` and run to the end of the current line. For example:
```sql
CREATE TABLE people (id INT, name TEXT(5));           # create a table
INSERT INTO people VALUES (1, "Alice"), (2, "Bob");   # insert into the table
SELECT * FROM people;                                 # select from the table
```

### Output File Format (`.out`)
Mirrors the expected output for the result of each statement:
- "X rows affected" - for CREATE, INSERT, UPDATE, DELETE
- `|` separated values - for SELECT
- "&lt;error category&gt; error: &lt;message&gt;" - for failed statements

The output for the example `.sql` above:
```
0 rows affected
2 rows affected
1 | Alice
2 | Bob
```

## Test Grouping
Integration tests are grouped by category, using a numeric prefix in filenames:
| Group | Category                   | Purpose                       |
|-------|----------------------------|-------------------------------|
| 0XX   | Basic sanity               | Startup, simple DDL/DML       |
| 1XX   | Invalid statements/queries | Error catching/reporting      |
| 2XX   | Compound commands          | WHERE/UPDATE order/condensing |
| 3XX   | Reserved identifiers       | Handling of reserved names    |
| 4XX   | Stress/Volume              | Large data, limits            |

## Stress Tests

### Generation Pipeline
Stress tests (the 4XX group) are automatically generated using two Python scripts.

#### `gen_data.py`
Generates `.csv` datasets in `data/`. For each table listed in `data/schema.json` two datasets of 5 million rows each are produced - one in ascending order (in every field) and the
other in (seeded) random order. Additionally, two flags are added are appended to each row with the first flag being set for the first 1% of rows and the second being set for the
first 0.5%.

#### `gen_tests.py`
Generates various pairs of `.sql` and `.out` files in `scripts/` and `output` from the datasets.

#### Automatic Regeneration (CMake Integration)
CMake automatically handles regeneration of datasets when `data/schema.json` or `gen_data.py` are modified, and regeneration of stress-tests when the datasets are regenerated or
`gen_tests.py` is modified. Also if the integration test runner `runner.cpp` is rebuilt, a full stress-test regeneration is carried out.

#### Manual Regeneration (CMake Targets)
- `make regen-data` - runs `gen_data.py` to regenerate datasets
- `make regen-tests` - runs `gen_tests.py` to regenerate stress-tests from the datasets
- `make regen-all` - runs `gen_data.py` then `gen_tests.py`

### Descriptions
The table below details the size and operations of each stress-test along with the motivation for it:
| Test Name                   | # Tables | Rows per Table | Batch Size | Inserts    | Deletes    | Operations                      | Purpose                                       |
|-----------------------------|----------|----------------|------------|------------|------------|---------------------------------|-----------------------------------------------|
| insert_small_seq            | 4        | 100K           | 1          | Sequential | N/A        | Inserts only                    | Baseline correctness                          |
| insert_small_random         | 4        | 100K           | 1          | Random     | N/A        | Inserts only                    | Check non-contiguous page allocation          |
| insert_large_seq            | 4        | 5M             | 1K         | Sequential | N/A        | Inserts only                    | Stress allocator and file growth              |
| insert_large_random         | 4        | 5M             | 1K         | Random     | N/A        | Inserts only                    | Heavy fragmentation and non-linear allocation |
| insert_large_wide_seq       | 4        | 5M             | 1K         | Sequential | N/A        | Inserts only                    | Stress allocator and file growth              |
| insert_large_wide_random    | 4        | 5M             | 1K         | Random     | N/A        | Inserts only                    | Heavy fragmentation and non-linear allocation |
| insert_update_seq           | 4        | 1M             | 1K         | Sequential | N/A        | Insert → update 10%             | Test update logic, contiguous overwrites      |
| insert_update_random        | 4        | 1M             | 1K         | Random     | N/A        | Insert → update 10%             | Test updates that modify random records       |
| update_random_texts         | 3        | 1M             | 1K         | Random     | N/A        | Insert → update TEXT fields     | Validate variable-length storage integrity    |
| insert_update_delete_seq    | 4        | 1M             | 1K         | Sequential | Sequential | Insert → update 10% → delete 5% | Verify correctness with sequential row churn  |
| insert_update_delete_random | 4        | 1M             | 1K         | Random     | Random     | Insert → update 10% → delete 5% | Vefify correctness for scattered rows         |
| delete_cascade_seq          | 4        | 1M             | 1K         | Sequential | Sequential | Insert → delete 90%             | Check large contiguous deletes                |
| delete_cascade_random       | 4        | 1M             | 1K         | Random     | Random     | Insert → delete 90%             | Check many non-contiguous deletes             |
| drop_random                 | 4        | 1M             | 1K         | Random     | N/A        | Insert → drop → insert          | Force scattered free space and page reuse     |
