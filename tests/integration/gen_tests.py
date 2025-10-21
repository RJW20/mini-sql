import json
import random
from collections import namedtuple
from collections.abc import Callable
from enum import Enum
from functools import partial
from pathlib import Path
from typing import TextIO

from gen_data import DATA_DIR, DATASET_SIZE, FLAG_RATES, generator_by_type

random.seed(12345)
SCRIPT_DIR = Path("tests/integration/scripts")
OUTPUT_DIR = Path("tests/integration/output")
BATCH_SMALL = 1
BATCH_LARGE = 1_000
TABLE_SMALL = 10_000
TABLE_MEDIUM = 100_000
TABLE_LARGE = DATASET_SIZE

Column = namedtuple("Column", ["name", "type"])
Columns = list[Column]
Tables = dict[str, dict[str, Columns]]


def build_create_statement(
    name: str, columns: Columns, primary: str | None = None
) -> str:
    """Return a CREATE statement for the table with given name, columns and
    primary key."""

    create = f"CREATE TABLE {name} ({
        ", ".join([f"{column.name} {column.type}" for column in columns])
        }"
    if primary: create += f", PRIMARY KEY({primary})"
    create += ");"

    return create


def build_insert_statement(name: str, values: TextIO, count: int) -> str:
    """Return an INSERT statement for the table with given name which inserts
    the next count values from values."""

    insert = [f"INSERT INTO {name} VALUES"]

    if count == 1:
        insert.append(f"({values.readline().rstrip('\r\n')});")
        return ' '.join(insert)

    for _ in range(count - 1):
        insert.append(f"({values.readline().rstrip('\r\n')}),")
    insert.append(f"({values.readline().rstrip('\r\n')});")
    return '\n\t'.join(insert)


def build_update_statement(name: str, update_column: Column) -> str:
    """Return an UPDATE statement for the table with given name which modifies
    the given column depending on its type:
    * INT - multiplies it by 2
    * REAL - add a uniformly generated float in [0,1)
    * TEXT() - set to a new randomly generated string
    """

    update = f"UPDATE {name} SET {update_column.name} = "

    if update_column.type == "INT":
        update += f"{update_column.name} * 2"
    elif update_column.type == "REAL":
        update += f"{update_column.name} + {random.random()}"
    else:
        update += f"\"{generator_by_type(update_column.type)(
            random.randint(0,TABLE_LARGE)
        )}\""
    
    return update


class Order(Enum):
    SEQUENTIAL = 'seq'
    RANDOM = 'random'


def gen_insert(
    script: TextIO, output: TextIO, tables: Tables, order_type: Order,
    num_rows: int, batch_size: int, include_default_primary: bool = True
) -> None:
    """Write to script and output SQL for creating and inserting into tables
    alongside the expected results.
    
    For each table in tables, there will be multiple SQL tables created and
    inserted into. Each will contain all columns, with a different column set
    as the primary key each time. If include_default_primary is set then an
    addition table with no primary key declared will also be created.
    Depending on order type, the insertion of elements will be in ascending
    order or randomized.
    num_rows is the number of rows to insert into each table.
    batch_size is the amount of rows to insert per INSERT statement."""

    flags = [
        Column(
            f"flag_{int((DATASET_SIZE * rate)/num_rows * 100)}_percent", "INT"
        ) for rate in FLAG_RATES
    ]

    for name, info in tables.items():
        columns = [
            Column(name, type) for name, type in info["columns"].items()
        ]
        for primary in [None] + [column.name for column in columns]:
            if not primary and not include_default_primary: continue
            script.write('\n')
            table_name = name
            if primary: table_name += f"_pk_{primary}" 
            print(
                build_create_statement(table_name, columns + flags, primary),
                file=script
            )
            output.write("0 rows affected\n")
            values_remaining = num_rows
            with open(DATA_DIR / f"{name}_{order_type.value}.csv") as values:
                while values_remaining >= batch_size:
                    print(
                        build_insert_statement(table_name, values, batch_size),
                        file=script
                    )
                    if batch_size == 1: output.write("1 row affected\n")
                    else: output.write(f"{batch_size} rows affected\n")
                    values_remaining -= batch_size


def gen_insert_update(
    script: TextIO, output: TextIO, tables: Tables, order_type: Order,
    num_rows: int, batch_size: int
) -> None:
    """Write to script and output SQL for creating, inserting into and updating
    rows within tables alongside the expected results.
    
    Calls gen_insert first.
    Updates rows via build_update_statement for those that have the first flag
    set.
    """

    gen_insert(script, output, tables, order_type, num_rows, batch_size)

    update_count = int(DATASET_SIZE * FLAG_RATES[0])
    script.write('\n')
    for name, info in tables.items():
        columns = [
            Column(name, type) for name, type in info["columns"].items()
        ]
        for primary in [None] + [name for name, type in columns]:
            table_name = name
            if primary: table_name += f"_pk_{primary}"
            update = build_update_statement(
                table_name,
                [column for column in columns if column.name != primary][0]
            )
            update += \
                f" WHERE flag_{int(update_count/num_rows * 100)}_percent = 1;"
            print(update, file=script)
            output.write(f"{update_count} rows affected\n")


def gen_update_texts(
    script: TextIO, output: TextIO, tables: Tables, order_type: Order,
    num_rows: int, batch_size: int
) -> None:
    """Write to script and output SQL for creating, inserting into and updating
    rows within tables alongside the expected results.
    
    Calls gen_insert first (with include_default_primary False).
    Updates rows via build_update_statement for those that have the first flag
    set.
    Ensures a different column is updated per table.
    """

    gen_insert(
        script, output, tables, order_type, num_rows, batch_size, False
    )

    update_count = int(DATASET_SIZE * FLAG_RATES[0])
    script.write('\n')
    for name, info in tables.items():
        columns = [
            Column(name, type) for name, type in info["columns"].items()
        ]
        for i, primary in enumerate([name for name, type in columns]):
            table_name = name
            if primary: table_name += f"_pk_{primary}"
            update = build_update_statement(
                table_name,
                columns[(i + 1) % len(columns)]
            )
            update += \
                f" WHERE flag_{int(update_count/num_rows * 100)}_percent = 1;"
            print(update, file=script)
            output.write(f"{update_count} rows affected\n")


def gen_insert_update_delete(
    script: TextIO, output: TextIO, tables: Tables, order_type: Order,
    num_rows: int, batch_size: int
) -> None:
    """Write to script and output SQL for creating, inserting into, updating
    rows within and deleting rows from tables alongside the expected results.
    
    Calls gen_insert_update first.
    Deletes rows that have the second flag set.
    """

    gen_insert_update(script, output, tables, order_type, num_rows, batch_size)

    delete_count = int(DATASET_SIZE * FLAG_RATES[1])
    script.write('\n')
    for name, info in tables.items():
        columns = [
            Column(name, type) for name, type in info["columns"].items()
        ]
        for primary in [None] + [name for name, type in columns]:
            table_name = name
            if primary: table_name += f"_pk_{primary}"
            delete = f"DELETE FROM {table_name} WHERE " \
                f"flag_{int(delete_count/num_rows * 100)}_percent = 1;"
            print(delete, file=script)
            output.write(f"{delete_count} rows affected\n")


def gen_delete_cascade(
    script: TextIO, output: TextIO, tables: Tables, order_type: Order,
    num_rows: int, batch_size: int
) -> None:
    """Write to script and output SQL for creating, inserting into and deleting
    rows from tables alongside the expected results.
    
    Calls gen_insert first.
    Deletes rows that have the first flag not set.
    """
    
    gen_insert(script, output, tables, order_type, num_rows, batch_size)

    unchanged_count = int(DATASET_SIZE * FLAG_RATES[0])
    delete_count = num_rows - unchanged_count
    script.write('\n')
    for name, info in tables.items():
        columns = [
            Column(name, type) for name, type in info["columns"].items()
        ]
        for primary in [None] + [name for name, type in columns]:
            table_name = name
            if primary: table_name += f"_pk_{primary}"
            delete = f"DELETE FROM {table_name} WHERE " \
                f"flag_{int(unchanged_count/num_rows * 100)}_percent = 0;"
            print(delete, file=script)
            output.write(f"{delete_count} rows affected\n")


def run_gen_test(
    base_test_name: str, order_type: Order,
    test_generator: Callable[[TextIO, TextIO, Tables, Order], None],
    tables: Tables
) -> None:
    """Run the given test generator.
    
    Creates script and output files to pass to the generator, and filters
    tables to only include those flagged for the given test name.
    """

    test_name = \
        f"{400 + run_gen_test.test_no}_{base_test_name}_{order_type.value}"
    run_gen_test.test_no += 1

    with open(SCRIPT_DIR / f"{test_name}.sql", 'w') as script, \
        open(OUTPUT_DIR / f"{test_name}.out", 'w') as output:
        script.write(f"# {test_name}\n")
        script.write(
            "# This test was auto-generated - do not modify directly\n"
        )
        filtered_tables = {
            name: info for name, info in tables.items()
            if base_test_name in info["tests"]
        }
        test_generator(script, output, filtered_tables, order_type)

run_gen_test.test_no = 0


def main() -> None:
    """Create .sql and .out pairs in scripts/ and output/ respectively for the
    follwing stress-tests:
    * insert_small_seq
    * insert_small_random
    * insert_large_seq
    * insert_large_random
    * insert_large_wide_seq
    * insert_large_wide_random
    * insert_update_seq
    * insert_update_random
    * update_texts_random
    * insert_update_delete_seq
    * insert_update_delete_random
    * delete_cascade_seq
    * delete_cascade_random
    """

    with open(DATA_DIR / "schema.json") as f:
        tables = json.load(f)["tables"]
    
    # insert
    run_gen_test(
        "insert_small", Order.SEQUENTIAL,
        partial(gen_insert, num_rows=TABLE_SMALL, batch_size=BATCH_SMALL),
        tables
    )
    run_gen_test(
        "insert_small", Order.RANDOM,
        partial(gen_insert, num_rows=TABLE_SMALL, batch_size=BATCH_SMALL),
        tables
    )
    run_gen_test(
        "insert_large", Order.SEQUENTIAL,
        partial(gen_insert, num_rows=TABLE_LARGE, batch_size=BATCH_LARGE),
        tables
    )
    run_gen_test(
        "insert_large", Order.RANDOM,
        partial(gen_insert, num_rows=TABLE_LARGE, batch_size=BATCH_LARGE),
        tables
    )
    run_gen_test(
        "insert_large_wide", Order.SEQUENTIAL,
        partial(gen_insert, num_rows=TABLE_LARGE, batch_size=BATCH_LARGE),
        tables
    )
    run_gen_test(
        "insert_large_wide", Order.RANDOM,
        partial(gen_insert, num_rows=TABLE_LARGE, batch_size=BATCH_LARGE),
        tables
    )

    # update
    run_gen_test(
        "insert_update", Order.SEQUENTIAL,
        partial(
            gen_insert_update, num_rows=TABLE_MEDIUM, batch_size=BATCH_LARGE
        ),
        tables
    )
    run_gen_test(
        "insert_update", Order.RANDOM,
        partial(
            gen_insert_update, num_rows=TABLE_MEDIUM, batch_size=BATCH_LARGE
        ),
        tables
    )
    run_gen_test(
        "update_texts", Order.RANDOM,
        partial(
            gen_update_texts, num_rows=TABLE_MEDIUM, batch_size=BATCH_LARGE
        ),
        tables
    )

    # delete
    run_gen_test(
        "insert_update_delete", Order.SEQUENTIAL,
        partial(
            gen_insert_update_delete, num_rows=TABLE_MEDIUM,
            batch_size=BATCH_LARGE
        ),
        tables
    )
    run_gen_test(
        "insert_update_delete", Order.RANDOM,
        partial(
            gen_insert_update_delete, num_rows=TABLE_MEDIUM,
            batch_size=BATCH_LARGE
        ),
        tables
    )
    run_gen_test(
        "delete_cascade", Order.SEQUENTIAL,
        partial(
            gen_delete_cascade, num_rows=TABLE_MEDIUM, batch_size=BATCH_LARGE
        ),
        tables
    )
    run_gen_test(
        "delete_cascade", Order.RANDOM,
        partial(
            gen_delete_cascade, num_rows=TABLE_MEDIUM, batch_size=BATCH_LARGE
        ),
        tables
    )


if __name__ == "__main__":
    main()