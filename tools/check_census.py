#!/usr/bin/env python3
"""Run one exhaustive shard and verify it covered exactly its assigned interval."""
import argparse
from pathlib import Path
import subprocess
import sys

import gate


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", type=Path, required=True, help="poker_exhaustive executable")
    parser.add_argument("--cards", type=int, choices=(5, 6, 7), required=True)
    parser.add_argument("--shard", type=int, required=True)
    parser.add_argument("--shards", type=int, required=True)
    parser.add_argument("--log", type=Path, required=True)
    args = parser.parse_args()
    if not 0 <= args.shard < args.shards:
        parser.error("shard must be in [0, shards)")
    args.log.parent.mkdir(parents=True, exist_ok=True)
    with args.log.open("w") as log:
        result = subprocess.run([str(args.binary), str(args.cards), str(args.shard), str(args.shards)],
                                stdout=log, stderr=subprocess.STDOUT)
    if result.returncode:
        print(args.log.read_text(), file=sys.stderr)
        raise gate.GateError(f"exhaustive check failed with exit code {result.returncode}")
    census = gate.verify_census(args.log, args.cards, args.shard, args.shards)
    print(f"{census['checked']:,} {args.cards}-card sets verified (shard {args.shard + 1}/{args.shards})")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except gate.GateError as error:
        print(str(error), file=sys.stderr)
        sys.exit(1)
