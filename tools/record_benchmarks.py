#!/usr/bin/env python3
"""Record a validated baseline snapshot; this is not a performance merge gate."""
import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import re
import secrets
import statistics
import subprocess

import gate


def summarize(measurements):
    if not measurements:
        raise gate.GateError("no measurements")
    names = set(gate.profile_names())
    if any(set(values) != names for values in measurements):
        raise gate.GateError("incomplete measurement set")
    result = {}
    for name in sorted(names):
        values = [gate.finite_number(sample[name]) for sample in measurements]
        if min(values) <= 0:
            raise gate.GateError("nonpositive measurement")
        result[name] = {"median_seconds_per_item": statistics.median(values),
                        "min_seconds_per_item": min(values), "max_seconds_per_item": max(values)}
    return result


def validate_snapshot(report, expected_commit):
    if not re.fullmatch(r"[0-9a-f]{40}", expected_commit):
        raise gate.GateError("invalid source commit")
    if (report.get("schema") != 1 or report.get("kind") != "baseline_snapshot"
            or report.get("eligible_for_merge") is not False or report.get("source_commit") != expected_commit):
        raise gate.GateError("snapshot provenance mismatch")
    if type(report.get("samples")) is not int or not 1 <= report["samples"] <= 31:
        raise gate.GateError("invalid sample count")
    if len(report.get("runs", [])) != report["samples"]:
        raise gate.GateError("missing recorded runs")
    if set(report.get("profiles", {})) != set(gate.profile_names()):
        raise gate.GateError("incomplete snapshot")
    for row in report["profiles"].values():
        lo, median, hi = (gate.finite_number(row.get(k)) for k in
                          ("min_seconds_per_item", "median_seconds_per_item", "max_seconds_per_item"))
        if not 0 < lo <= median <= hi:
            raise gate.GateError("invalid summary range")


def markdown(report):
    validate_snapshot(report, report["source_commit"])
    def cell(value):
        return str(value).replace("\n", " ").replace("|", "\\|").replace("`", "'")
    lines = ["# Latest main benchmark", "",
             f"Tested source commit: `{report['source_commit']}`", "",
             f"Recorded: {cell(report['created_at'])}. Samples: {report['samples']}; "
             f"target records per profile: {report['records']}.", "",
             f"Machine: {cell(report['environment']['machine'])}. "
             f"Compiler: {cell(report['environment']['compiler'].splitlines()[0])}.", "",
             f"Workflow: {report['workflow_url'] or 'local run'}", "",
             "These are medians and observed ranges from a portable Release build. "
             "Hosted runners can differ between runs. This snapshot does not certify a speedup "
             "or authorize a merge. All timed outputs and required workloads were validated.", "",
             "Times include harness overhead. An item is a hand evaluation, except state "
             "profiles (one state operation) and startup (one initialization). Simulation "
             "profiles include dealing and accumulation; multiply time per item by player "
             "count to obtain time per trial.", "",
             "| Profile | Median ns/item | Observed min–max ns/item | Items/s |",
             "| --- | ---: | ---: | ---: |"]
    for name, row in sorted(report["profiles"].items()):
        median = row["median_seconds_per_item"]
        lines.append(f"| {name} | {median * 1e9:,.1f} | "
                     f"{row['min_seconds_per_item'] * 1e9:,.1f}–{row['max_seconds_per_item'] * 1e9:,.1f} | "
                     f"{1 / median:,.0f} |")
    lines += ["", "See `latest.json` for seeds, hashes, resource observations and environment details; "
              "`history/` preserves previous snapshots. Raw measurements are retained as workflow artifacts.", ""]
    return "\n".join(lines)


def record(root, build, out, samples, workflow_url):
    if out.exists():
        raise gate.GateError("output directory must be new")
    if not 1 <= samples <= 31:
        raise gate.GateError("samples must be between 1 and 31")
    if out.is_relative_to(root) and (out == root or out.relative_to(root).parts[0] not in gate.EXCLUDED):
        raise gate.GateError("output must be outside source or in an excluded directory")
    commit = subprocess.check_output(["git", "-C", root, "rev-parse", "HEAD"], text=True).strip()
    if subprocess.check_output(["git", "-C", root, "status", "--porcelain"], text=True).strip():
        raise gate.GateError("record from a clean committed source checkout")
    cache = dict(line.split("=", 1) for line in (build / "CMakeCache.txt").read_text().splitlines()
                 if "=" in line and not line.startswith(("#", "//")))
    required = {"CMAKE_BUILD_TYPE:STRING": "Release", "POKER_NATIVE:BOOL": "OFF",
                "POKER_LTO:BOOL": "OFF", "POKER_SANITIZE:BOOL": "OFF", "POKER_TSAN:BOOL": "OFF",
                "POKER_FUZZ:BOOL": "OFF", "POKER_BUILD_BENCHMARKS:BOOL": "ON"}
    if any(cache.get(k) != v for k, v in required.items()):
        raise gate.GateError("snapshot requires the portable unsanitized Release build profile")
    if Path(cache["CMAKE_HOME_DIRECTORY:INTERNAL"]).resolve() != root:
        raise gate.GateError("build belongs to a different source checkout")
    out.mkdir(parents=True)
    files = gate.inventory(root)
    policy = gate.load_json(root / "benchmarks/policy.json")
    binary = build / "poker_bench"
    binary_hash = hashlib.sha256(binary.read_bytes()).hexdigest()
    gate.audit_symbols(build, out / "symbols.log")
    env = gate.clean_environment()
    # Authentication is only needed by the publisher, never the timed process.
    for name in ("GH_TOKEN", "GITHUB_TOKEN"):
        env.pop(name, None)
    measurements, runs = [], []
    for i in range(samples):
        seed = secrets.randbits(64)
        raw = out / f"raw-{i:02}.json"
        observation = gate.run([binary, f"--seed={seed}", f"--records={policy['records']}",
                                f"--benchmark_out={raw}", "--benchmark_out_format=json", "--benchmark_color=false"],
                               out / f"run-{i:02}.log", env=env, timeout=policy["process_timeout_seconds"])
        data = gate.load_json(raw)
        measurements.append(gate.parse_measurement(data, seed=seed, policy=policy, observation=observation))
        runs.append({"seed": seed, "raw_file": raw.name, "sha256": hashlib.sha256(raw.read_bytes()).hexdigest(),
                     "shared_bytes": int(data["context"]["poker_shared_bytes"]),
                     "state_bytes": int(data["context"]["poker_state_bytes"]), **observation})
    if gate.inventory(root) != files or hashlib.sha256(binary.read_bytes()).hexdigest() != binary_hash:
        raise gate.GateError("source or binary changed during measurement")
    environment = gate.system_metadata()
    compiler = cache.get("CMAKE_CXX_COMPILER:FILEPATH", cache.get("CMAKE_CXX_COMPILER:STRING"))
    if compiler:
        environment["compiler"] = subprocess.check_output([compiler, "--version"], text=True).strip()
    report = {"schema": 1, "kind": "baseline_snapshot", "eligible_for_merge": False,
              "source_commit": commit, "source_digest": gate.digest(files), "binary_sha256": binary_hash,
              "created_at": datetime.now(timezone.utc).isoformat(), "workflow_url": workflow_url,
              "samples": samples, "records": policy["records"], "environment": environment,
              "policy_sha256": hashlib.sha256((root / "benchmarks/policy.json").read_bytes()).hexdigest(),
              "profiles": summarize(measurements), "runs": runs}
    validate_snapshot(report, commit)
    (out / "latest.json").write_text(json.dumps(report, indent=2, allow_nan=False) + "\n")
    (out / "latest.md").write_text(markdown(report))
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--build", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--samples", type=int, default=5)
    parser.add_argument("--workflow-url", default="")
    args = parser.parse_args()
    report = record(args.source.resolve(), args.build.resolve(), args.out.resolve(), args.samples, args.workflow_url)
    print(f"Validated {len(report['profiles'])} profiles across {report['samples']} fresh runs: {args.out}")


if __name__ == "__main__":
    main()
