#!/usr/bin/env python3
"""Publish validated data to a separate Git branch without changing the worktree."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile

import gate
from record_benchmarks import markdown, validate_snapshot

RESULTS_REF = "refs/heads/benchmark-results"


def git(root, *args, env=None, input=None):
    return subprocess.check_output(["git", "-C", root, *args], input=input, text=True, env=env).strip()


def remote_ref(root, ref):
    result = git(root, "ls-remote", "--heads", "origin", ref)
    return result.split()[0] if result else None


def publish(root, report, expected_commit):
    validate_snapshot(report, expected_commit)
    payload = json.dumps(report, indent=2, allow_nan=False) + "\n"
    digest = hashlib.sha256(payload.encode()).hexdigest()
    paths = {"latest.json": payload, "latest.md": markdown(report),
             f"history/{expected_commit}/{digest}.json": payload}
    with tempfile.TemporaryDirectory() as directory:
        # A separate index builds the results tree without staging/deleting any
        # files in the source checkout or switching away from its tested commit.
        env = {**os.environ, "GIT_INDEX_FILE": str(Path(directory) / "index"),
               "GIT_AUTHOR_NAME": "github-actions[bot]", "GIT_COMMITTER_NAME": "github-actions[bot]",
               "GIT_AUTHOR_EMAIL": "41898282+github-actions[bot]@users.noreply.github.com",
               "GIT_COMMITTER_EMAIL": "41898282+github-actions[bot]@users.noreply.github.com"}
        for attempt in range(3):
            if remote_ref(root, "refs/heads/main") != expected_commit:
                return "stale: main advanced; snapshot retained as an artifact"
            previous = remote_ref(root, RESULTS_REF)
            if previous:
                git(root, "fetch", "--quiet", "origin", RESULTS_REF)
                previous = git(root, "rev-parse", "FETCH_HEAD")
                git(root, "read-tree", previous, env=env)
            else:
                git(root, "read-tree", "--empty", env=env)
            for path, content in paths.items():
                blob = git(root, "hash-object", "-w", "--stdin", input=content)
                git(root, "update-index", "--add", "--cacheinfo", "100644", blob, path, env=env)
            tree = git(root, "write-tree", env=env)
            if previous and tree == git(root, "rev-parse", f"{previous}^{{tree}}"):
                return "unchanged: snapshot is already published"
            parents = ["-p", previous] if previous else []
            commit = git(root, "commit-tree", tree, *parents, env=env,
                         input=f"Record benchmarks for main {expected_commit}\n")
            if remote_ref(root, "refs/heads/main") != expected_commit:
                return "stale: main advanced; snapshot retained as an artifact"
            result = subprocess.run(["git", "-C", root, "push", "origin", f"{commit}:{RESULTS_REF}"],
                                    capture_output=True, text=True)
            if result.returncode == 0:
                return f"published: {commit}"
            if remote_ref(root, RESULTS_REF) == previous:
                raise gate.GateError("result push rejected; check contents permission and branch rules")
        raise gate.GateError("results branch kept advancing; publication not completed")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, default=Path.cwd())
    parser.add_argument("--report", type=Path, required=True)
    parser.add_argument("--commit", required=True)
    args = parser.parse_args()
    if args.report.stat().st_size > 2 * 1024 * 1024:
        raise gate.GateError("unexpected report size")
    print(publish(args.source.resolve(), gate.load_json(args.report), args.commit))


if __name__ == "__main__":
    main()
