import csv
import json
import random
import string
from collections.abc import Generator
from functools import partial
from pathlib import Path

random.seed(12345)
DATA_DIR = Path("tests/integration/data")
DATASET_SIZE = 10_000


def unique_int_generator(seed: int) -> int:
    return seed


def unique_float_generator(seed: int) -> float:
    """Return a float between seed and seed + 1."""
    return seed + random.random()


def unique_string_generator(length: int, seed: int) -> str:
    """Return a string containing characters 0-9A-Za-z with size no greater
    than the given length.

    For a fixed length, 2 calls to this function with seed_1 < seed_2 will
    result in str_1 being alphabetically lower than str_2.
    Returns seed written in base 62.
    Fails if seed > 62 ** length.
    """

    chars = string.digits + string.ascii_uppercase + string.ascii_lowercase
    base = len(chars)
    if seed > base ** length:
        raise ValueError(f"Cannot generate string for seed {seed}: too large")

    if not seed: return '0'

    s = []
    while seed > 0:
        seed, rem = divmod(seed, base)
        s.append(chars[rem])
    return ''.join(reversed(s))


def row_generator(
    columns: list[dict[str, str]], count: int, ordered: bool = True
) -> Generator[list[int | float | str], None, None]:
    """Yields lists of data of type int, float or string.
    
    Every entry in the list is greater than that in the previous.
    """

    generators = []
    for column in columns:
        type = column["type"]
        if type == "INT": generators.append(unique_int_generator)
        elif type == "REAL": generators.append(unique_float_generator)
        elif len(type) > 6 and type[:4] == "TEXT":
            generators.append(partial(unique_string_generator, int(type[5:-1])))
        else: raise ValueError(f"Unknown type {type}")

    if ordered:
        for n in range(count): yield [gen(n) for gen in generators]
    else:
        indices = list(range(count))
        random.shuffle(indices)
        for n in indices: yield [gen(n) for gen in generators]


def main() -> None:
    """Create 2 .csv datasets for all tables listed in data/schema.json.
    
    Generates 2 sets of DATASET_SIZE rows of data for each dataset, one in
    ascending order (for every column) and the other randomized.
    """

    with open(DATA_DIR / "schema.json") as f:
        tables = json.load(f)["tables"]

    for name, info in tables.items():
        
        # sequential rows
        with open(DATA_DIR / f"{name}_seq.csv", 'w', newline='') as csvfile:
            row_gen = row_generator(info["columns"], DATASET_SIZE)
            writer = csv.writer(
                csvfile, quotechar='"', quoting=csv.QUOTE_STRINGS
            )
            writer.writerows(row_gen)

        # random rows
        with open(DATA_DIR / f"{name}_random.csv", 'w', newline='') as csvfile:
            row_gen = row_generator(info["columns"], DATASET_SIZE, False)
            writer = csv.writer(
                csvfile, quotechar='"', quoting=csv.QUOTE_STRINGS
            )
            writer.writerows(row_gen)


if __name__ == "__main__":
    main()