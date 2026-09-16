#!/usr/bin/env python3
"""Classify a pull request for CI: implementation-only or maintenance, plus declared targets.

Writes `implementation_only`, `targets` and `changed` to $GITHUB_OUTPUT when set,
and prints the same values. Never fails on a maintenance PR; the classification
only decides whether the benchmark comparison job runs.
"""
import argparse
import os
from pathlib import Path
import re
import subprocess
import sys

import gate


def changed_files(root, base, head):
    output = subprocess.check_output(["git", "-C", str(root), "diff", "--no-renames", "--name-only", "-z", base, head])
    return [path for path in output.decode().split("\0") if path]


def implementation_only(paths):
    if not paths:
        return False
    try:
        gate.check_changes({}, dict.fromkeys(paths, "changed"))
    except gate.GateError:
        return False
    return True


def declared_targets(body):
    """Return the profile IDs on a `Benchmark targets:` line, or [] when absent or malformed."""
    matches = re.findall(r"^\s*(?:Benchmark|Poker) targets:[ \t]*(.*)$", body or "", re.MULTILINE | re.IGNORECASE)
    if len(matches) != 1:
        return []
    values = [value.strip().strip("`") for value in matches[0].split(",") if value.strip()]
    known = set(gate.profile_names())
    if any(value not in known for value in values) or len(values) != len(set(values)):
        return []
    return values


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path.cwd())
    parser.add_argument("--base", required=True, help="base commit")
    parser.add_argument("--head", default="HEAD")
    args = parser.parse_args()
    paths = changed_files(args.root, args.base, args.head)
    scope = implementation_only(paths)
    targets = declared_targets(os.environ.get("PR_BODY", ""))
    lines = [f"implementation_only={'true' if scope else 'false'}", f"targets={','.join(targets)}",
             f"changed={len(paths)}"]
    output = os.environ.get("GITHUB_OUTPUT")
    if output:
        with open(output, "a") as handle:
            handle.write("\n".join(lines) + "\n")
    print("\n".join(lines))
    if not scope and paths:
        print("Maintenance change: touches files outside src/ and include/poker/detail/. "
              "Benchmark comparison is skipped; code-owner review applies.", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
