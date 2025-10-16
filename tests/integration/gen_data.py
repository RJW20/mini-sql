import csv
import json
import random
import string
from collections.abc import Generator
from pathlib import Path

random.seed(12345)
DATA_DIR = Path("tests/integration/data")
DATASET_SIZE = 10_000


def unique_int_generator() -> Generator[int, None, None]:
    """Yield integers in ascending order."""
    n = 0
    while (True):
        yield n
        n += 1


def unique_float_generator() -> Generator[float, None, None]:
    """Yield floats in ascending order.
    
    The nth call yields a float between n-1 and n.
    """
    n = 0
    while (True):
        yield n + random.random()
        n += 1


def unique_string_generator(length: int) -> Generator[str, None, None]:
    """Yield strings of given length in alphabetical order.
    
    The nth call returns n-1 written in base 62.
    """

    chars = string.digits + string.ascii_uppercase + string.ascii_lowercase
    base = len(chars)
    max_num = base ** length

    def int_to_base(n: int) -> str:
        s = []
        while n > 0:
            n, r = divmod(n, base)
            s.append(chars[r])
        
        return ''.join(reversed(s))

    yield '0'
    for n in range(1, max_num): yield int_to_base(n)


def row_generator(
    columns: list[dict[str, str]], count: int
) -> Generator[list[int | float | str], None, None]:
    """Yields lists of data of type int, float or string.
    
    Every entry in the list is greater than that in the previous.
    """

    generators = []
    for column in columns:
        type = column["type"]
        if type == "INT": generators.append(unique_int_generator())
        elif type == "REAL": generators.append(unique_float_generator())
        elif len(type) > 6 and type[:4] == "TEXT":
            generators.append(unique_string_generator(int(type[5:-1])))
        else: raise ValueError(f"Unknown type {type}")

    for _ in range(count): yield [next(gen) for gen in generators]


def main() -> None:
    """Create .csv datasets for all tables listed in data/schema.json.
    
    Generates DATASET_SIZE rows of data for each table, in ascending order for
    every column.
    """

    with open(DATA_DIR / "schema.json") as f:
        tables = json.load(f)["tables"]

    for name, info in tables.items():
        row_gen = row_generator(info["columns"], DATASET_SIZE)
        with open(DATA_DIR / f"{name}.csv", 'w', newline='') as csvfile:
            writer = csv.writer(
                csvfile, quotechar='"', quoting=csv.QUOTE_STRINGS
            )
            writer.writerows(row_gen)


if __name__ == "__main__":
    main()