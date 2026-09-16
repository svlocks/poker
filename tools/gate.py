#!/usr/bin/env python3
"""Trusted local PR evaluator. Only implementation files are taken from a candidate.

The trusted base and this process must be controlled by the reviewer/CI operator.
This is a validation harness, not a sandbox for actively malicious native code.
"""
from __future__ import annotations

import argparse
from concurrent.futures import ThreadPoolExecutor
import hashlib
import json
import math
import os
from pathlib import Path
import platform
import re
import secrets
import signal
import shutil
import statistics
import subprocess
import sys
import time

EXCLUDED = {".git", "build", ".artifacts", ".deps", ".venv", ".local", "__pycache__", "CMakeUserPresets.json", ".envrc", ".direnv"}
EDITABLE = ("src/", "include/poker/detail/")


class GateError(Exception):
    pass


def profile_names():
    names = []
    for n in (5, 6, 7):
        names += [f"scalar/high/{n}", f"native/high/{n}"]
    names += [f"batch/high/{n}/{b}" for n in (5, 6, 7, 9) for b in (1, 8, 31, 128, 1024)]
    for h in (4, 5, 6):
        for b in (3, 4, 5):
            names += [f"scalar/omaha/{h}/{b}"]
            names += [f"batch/omaha/{h}/{b}/{n}" for n in (1, 8, 64, 1024)]
            names += [f"shared/omaha/{h}/{b}/{n}" for n in (2, 6)]
    names += [f"shared/holdem/{b}/{n}" for b in (3, 4, 5) for n in (2, 6, 9)]
    for known in (2, 5, 6):
        for reuse in (1, 8, 64):
            names += [f"prepared/high/{known}/{reuse}", f"total/high/{known}/{reuse}"]
    for h in (4, 5, 6):
        for reuse in (1, 16, 128):
            names += [f"prepared/omaha/{h}/{reuse}", f"total/omaha/{h}/{reuse}"]
    for players in (2, 6):
        names += [f"simulation/holdem/{players}", f"simulation/omaha/{players}"]
    for threads in (1, 2, 4):
        names += [f"parallel/high/7/{threads}", f"parallel/omaha/4/5/{threads}"]
    names += [f"state/{operation}/{game}" for operation in ("clone", "extend") for game in ("high", "omaha")]
    names += ["startup/initialize"]
    return names


def expected_batches(name, records):
    if name.startswith("parallel/"):
        return max(int(name.split("/")[-1]), math.ceil(records / 64))
    if name.startswith(("scalar/", "native/", "state/")):
        return records
    return math.ceil(records / int(name.split("/")[-1]))


def expected_items(name, records):
    if name.startswith("simulation/"):
        return records * int(name.split("/")[-1])
    if name == "startup/initialize":
        return 1
    if name.startswith(("scalar/", "native/", "state/")):
        return records
    batch = 64 if name.startswith("parallel/") else int(name.split("/")[-1])
    return expected_batches(name, records) * batch


def inventory(root: Path):
    files = {}
    for parent, dirs, names in os.walk(root, followlinks=False):
        dirs[:] = sorted(d for d in dirs if d not in EXCLUDED)
        for name in dirs + names:
            path = Path(parent) / name
            if name in EXCLUDED:
                continue
            if path.is_symlink():
                raise GateError(f"symlink not permitted in source snapshot: {path}")
            if path.is_file():
                rel = path.relative_to(root).as_posix()
                files[rel] = hashlib.sha256(path.read_bytes()).hexdigest()
    if "CMakeLists.txt" not in files:
        raise GateError(f"not a project snapshot: {root}")
    return files


def digest(files):
    return hashlib.sha256(json.dumps(files, sort_keys=True).encode()).hexdigest()


def check_changes(base, candidate):
    changed = sorted(p for p in base.keys() | candidate.keys() if base.get(p) != candidate.get(p))
    forbidden = [p for p in changed if not p.startswith(EDITABLE) or not p.endswith((".cpp", ".hpp", ".h", ".S"))]
    if forbidden:
        raise GateError("optimization changes protected files; use separate contract/maintenance review: " + ", ".join(forbidden))
    return changed


def snapshot(root, files, dest):
    dest.mkdir(parents=True)
    for rel in files:
        target = dest / rel
        target.parent.mkdir(parents=True, exist_ok=True)
        content = (root / rel).read_bytes()
        if hashlib.sha256(content).hexdigest() != files[rel]:
            raise GateError(f"source changed while snapshotting: {rel}")
        target.write_bytes(content)


def clean_environment():
    env = dict(os.environ)
    for key in list(env):
        if key.startswith(("BENCHMARK_", "CMAKE_", "DYLD_", "LD_")) or key in {"CFLAGS", "CXXFLAGS", "CPPFLAGS", "LDFLAGS", "CPATH", "LIBRARY_PATH", "GTEST_FILTER", "GTEST_TOTAL_SHARDS", "GTEST_SHARD_INDEX", "GTEST_REPEAT"}:
            del env[key]
    return env


def run(args, log, *, env=None, timeout=300):
    """Parent observes wall time and (on Unix) the specific child's peak RSS."""
    start = time.monotonic()
    with open(log, "wb") as output:
        child = subprocess.Popen([str(x) for x in args], stdout=output, stderr=subprocess.STDOUT,
                                 env=env, start_new_session=os.name == "posix")
        usage = None
        if hasattr(os, "wait4"):
            while True:
                pid, status, usage = os.wait4(child.pid, os.WNOHANG)
                if pid:
                    child.returncode = os.waitstatus_to_exitcode(status)
                    break
                if time.monotonic() - start > timeout:
                    os.killpg(child.pid, signal.SIGKILL)
                    child.wait()
                    raise GateError(f"timeout: {args[0]} (see {log})")
                time.sleep(0.025)
        else:
            try:
                child.wait(timeout=timeout)
            except subprocess.TimeoutExpired as exc:
                child.kill(); child.wait()
                raise GateError(f"timeout (see {log})") from exc
    if child.returncode:
        raise GateError(f"command failed ({child.returncode}): {args[0]} (see {log})")
    rss = None if usage is None else usage.ru_maxrss * (1 if sys.platform == "darwin" else 1024)
    return {"wall_seconds": time.monotonic() - start, "peak_rss_bytes": rss}


def system_metadata():
    result = {"system": platform.platform(), "machine": platform.machine(), "processor": platform.processor()}
    for name, command in (("compiler", ["c++", "--version"]), ("cmake", ["cmake", "--version"]), ("ninja", ["ninja", "--version"]), ("cpu", ["lscpu", "--json"])):
        if shutil.which(command[0]):
            proc = subprocess.run(command, capture_output=True, text=True, check=True)
            result[name] = proc.stdout.strip()
    result["build_profile"] = {"CMAKE_BUILD_TYPE": "Release", "POKER_NATIVE": False, "POKER_LTO": False, "POKER_SANITIZE": False}
    return result


def reject_constant(value):
    raise GateError(f"non-finite JSON constant: {value}")


def unique_object(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            raise GateError(f"duplicate JSON key: {key}")
        result[key] = value
    return result


def load_json(path):
    try:
        return json.loads(Path(path).read_text(), parse_constant=reject_constant, object_pairs_hook=unique_object)
    except (ValueError, OSError) as exc:
        raise GateError(f"invalid JSON: {path}") from exc


def finite_number(value):
    if isinstance(value, bool) or not isinstance(value, (int, float)) or not math.isfinite(value):
        raise GateError("missing, non-numeric, or non-finite measurement")
    return value


def parse_measurement(report, *, seed, policy, observation):
    if not isinstance(report, dict) or not isinstance(report.get("context"), dict) or not isinstance(report.get("benchmarks"), list):
        raise GateError("invalid report structure")
    context = report["context"]
    required = {"poker_protocol": "1", "poker_seed": str(seed), "poker_records": str(policy["records"]), "poker_smoke": "false", "poker_backend": "portable"}
    if any(context.get(k) != v for k, v in required.items()):
        raise GateError("benchmark provenance, backend, or corpus mismatch")
    for key, cap in (("poker_shared_bytes", policy["maximum_shared_bytes"]), ("poker_state_bytes", policy["maximum_state_bytes"]), ("poker_init_ns", policy["maximum_initialization_ns"])):
        try:
            value = int(context[key])
        except (KeyError, ValueError, TypeError) as exc:
            raise GateError(f"invalid context field {key}") from exc
        if value < (0 if key in ("poker_shared_bytes", "poker_init_ns") else 1) or value > cap:
            raise GateError(f"resource limit violated: {key}")
    rss = observation.get("peak_rss_bytes")
    if rss is None or finite_number(rss) > policy["maximum_process_rss_mib"] * 1024 * 1024:
        raise GateError("process RSS unavailable or over budget")
    # Compile-time tables can make initialization faster than the clock can
    # resolve; a zero reading is recorded as one nanosecond rather than rejected.
    rows = {"startup/initialize": max(int(context["poker_init_ns"]), 1) * 1e-9}
    if rows["startup/initialize"] > observation["wall_seconds"] * 1.05:
        raise GateError("impossible initialization duration")
    units = {"ns": 1e-9, "us": 1e-6, "ms": 1e-3, "s": 1.0}
    for row in report["benchmarks"]:
        if not isinstance(row, dict) or not isinstance(row.get("name"), str):
            raise GateError("invalid workload structure")
        name = row.get("name", "").split("/iterations:")[0]
        if name not in profile_names() or name in rows:
            raise GateError("unexpected or duplicate workload: " + name)
        if row.get("error_occurred") or row.get("run_type") != "iteration" or row.get("iterations") != 1:
            raise GateError("skipped, failed, repeated, or aggregate-only workload: " + name)
        count = expected_items(name, policy["records"])
        if finite_number(row.get("verified_items")) != count:
            raise GateError("missing or incorrect verified work count: " + name)
        if name.startswith("simulation/") and finite_number(row.get("verified_trials")) != policy["records"]:
            raise GateError("simulation skipped trials")
        if not name.startswith("simulation/"):
            if finite_number(row.get("verified_batches")) != expected_batches(name, policy["records"]):
                raise GateError("batch coverage mismatch")
        if row.get("time_unit") not in units:
            raise GateError("unknown timing unit")
        seconds = finite_number(row.get("real_time")) * units[row["time_unit"]]
        if seconds <= 0 or seconds > observation["wall_seconds"] * 1.05:
            raise GateError("impossible duration")
        rate = finite_number(row.get("items_per_second"))
        if not math.isclose(rate, count / seconds, rel_tol=1e-5):
            raise GateError("throughput and elapsed time disagree")
        rows[name] = seconds / count
    if set(rows) != set(profile_names()):
        raise GateError("missing workloads: " + ", ".join(sorted(set(profile_names()) - set(rows))))
    return rows


def median_interval(values, alpha):
    """Distribution-free order-statistic interval for the population median.

    Assumes independent paired trials. No interval is claimed when too few
    repetitions can support the requested simultaneous confidence level.
    """
    ordered = sorted(values)
    n = len(ordered)
    k = 0
    tail = 0
    for candidate in range(1, n // 2 + 1):
        tail += math.comb(n, candidate - 1)
        if 2 * tail / (2 ** n) <= alpha:
            k = candidate
    if not k:
        return None
    return ordered[k - 1], ordered[n - k]


def decide(pairs, targets, policy, diagnostic=False):
    """Compare paired measurements. With no declared targets, any profile may
    supply the improvement; declared targets restrict where it must appear."""
    names = set(profile_names())
    if not set(targets) <= names:
        raise GateError("unknown target profile")
    watched = set(targets) or names
    if not pairs:
        raise GateError("no paired measurements")
    for base, candidate in pairs:
        if set(base) != names or set(candidate) != names:
            raise GateError("incomplete paired measurements")
        for row in (base, candidate):
            if any(finite_number(v) <= 0 for v in row.values()):
                raise GateError("nonpositive measurement")
    alpha = policy["family_error_probability"] / len(names)  # simultaneous bounds, including target selection
    result = {}
    any_gain = False
    all_guarded = True
    clear_regression = False
    for name in sorted(names):
        ratios = [c[name] / b[name] for b, c in pairs]
        interval = median_interval(ratios, alpha)
        row = {"median_ratio": statistics.median(ratios), "interval": interval}
        if interval is None:
            all_guarded = False
        else:
            lo, hi = interval
            all_guarded &= hi <= 1 + policy["maximum_regression"]
            clear_regression |= lo > 1 + policy["maximum_regression"]
            any_gain |= name in watched and hi < 1 - policy["minimum_improvement"]
        result[name] = row
    verdict = "inconclusive"
    if clear_regression:
        verdict = "fail"
    elif all_guarded and any_gain and len(pairs) >= policy["pairs"] and not diagnostic:
        verdict = "pass"
    return {"verdict": verdict, "eligible_for_merge": verdict == "pass", "profiles": result,
            "method": "paired median ratio; exact binomial order-statistic intervals; Bonferroni family adjustment"}


def audit_symbols(build, log):
    archive = build / "libpoker_evaluator.a"
    if not shutil.which("nm") or not archive.is_file():
        raise GateError("symbol audit unavailable")
    proc = subprocess.run(["nm", "--defined-only", str(archive)], capture_output=True, text=True, check=True)
    Path(log).write_text(proc.stdout)
    forbidden = {"main", "clock_gettime", "gettimeofday", "getrusage", "times", "time", "fopen", "fwrite", "write", "exit", "_exit", "dlsym"}
    for line in proc.stdout.splitlines():
        symbol = line.split()[-1] if line.split() else ""
        if symbol in forbidden or re.match(r"_ZNK?(6oracle|9benchmark)", symbol):
            raise GateError("candidate interposes on trusted measurement or reference code: " + symbol)


def binary_hashes(build):
    hashes = {}
    for name in ("poker_bench", "poker_tests", "poker_exhaustive", "poker_c_consumer",
                 "libpoker_evaluator.a", "libpoker_oracle.a"):
        path = build / name
        if path.is_symlink() or not path.is_file():
            raise GateError("validated binaries must remain regular files")
        hashes[name] = hashlib.sha256(path.read_bytes()).hexdigest()
    return hashes


def verify_census(log, cards, shard, shards):
    total = math.comb(52, cards)
    begin, end = total * shard // shards, total * (shard + 1) // shards
    expected = {"cards": cards, "shard": shard, "shards": shards, "begin": begin,
                "end": end, "checked": end - begin, "total": total}
    result = load_json(log)
    if result != expected or any(type(value) is not int for value in result.values()):
        raise GateError("exhaustive validation did not cover the complete assigned domain")
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--base", type=Path, required=True, help="reference source directory; also supplies the tools and build files")
    parser.add_argument("--candidate", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True, help="new report directory")
    parser.add_argument("--target", action="append", default=[],
                        help="profile that must improve; repeatable. Without targets any profile may supply the gain")
    parser.add_argument("--diagnostic", action="store_true", help="short local experiment; can never grant merge approval")
    parser.add_argument("--exhaustive-max-cards", type=int, choices=(0, 5, 6, 7),
                        help="largest exhaustive domain to check (default: policy); 0 skips the census entirely")
    parser.add_argument("--cpus", help="comma-separated Linux logical CPUs for this job, including parallel profiles")
    parser.add_argument("--jobs", type=int, default=2)
    args = parser.parse_args()
    if not 1 <= args.jobs <= 64:
        parser.error("jobs must be between 1 and 64")
    args.base, args.candidate, args.out = args.base.resolve(), args.candidate.resolve(), args.out.resolve()
    for root in (args.base, args.candidate):
        if args.out.is_relative_to(root) and (args.out == root or args.out.relative_to(root).parts[0] not in EXCLUDED):
            parser.error("output must be outside source trees or inside an excluded directory such as .artifacts")
    if args.out.exists():
        parser.error("output directory must be new; old runs cannot be reused")
    args.out.mkdir(parents=True)
    report = {"schema": 1, "verdict": "fail", "eligible_for_merge": False}
    try:
        base_files, candidate_files = inventory(args.base), inventory(args.candidate)
        changed = check_changes(base_files, candidate_files)
        # Critical: invoke THIS script from the trusted base, not the candidate.
        if Path(__file__).resolve() != (args.base / "tools/gate.py").resolve():
            raise GateError("invoke the gate.py belonging to --base")
        policy = load_json(args.base / "benchmarks/policy.json")
        if not set(args.target) <= set(profile_names()):
            raise GateError("unknown target")
        if args.cpus is not None:
            if not hasattr(os, "sched_setaffinity"):
                raise GateError("CPU affinity unsupported")
            os.sched_setaffinity(0, {int(cpu) for cpu in args.cpus.split(",")})
        exhaustive_max = policy["exhaustive_max_cards"] if args.exhaustive_max_cards is None else args.exhaustive_max_cards
        if args.diagnostic:
            exhaustive_max = min(exhaustive_max, 5)
        report.update(base_digest=digest(base_files), candidate_digest=digest(candidate_files), changed=changed,
                      targets=args.target, policy=policy, policy_digest=hashlib.sha256((args.base / "benchmarks/policy.json").read_bytes()).hexdigest(),
                      platform=platform.platform(), machine=platform.machine(), processor=platform.processor(),
                      affinity=sorted(os.sched_getaffinity(0)) if hasattr(os, "sched_getaffinity") else None,
                      diagnostic=args.diagnostic, exhaustive_max_cards=exhaustive_max, runs=[],
                      environment=system_metadata(), binary_hashes={}, correctness={})
        env = clean_environment()
        def execute(command, log, *, source, build, writing=False, output_dir=None, env=env, timeout=300):
            del source, build, writing, output_dir  # kept for call-site symmetry
            return run(command, log, env=env, timeout=timeout)
        builds = {}
        for name, root, files in (("base", args.base, base_files), ("candidate", args.candidate, candidate_files)):
            src = args.out / name / "source"
            snapshot(root, files, src)
            build = args.out / name / "build"
            builds[name] = build
            execute(["cmake", "-S", src, "-B", build, "-G", "Ninja", "-DCMAKE_BUILD_TYPE=Release"],
                    args.out / f"{name}-configure.log", source=src, build=build, writing=True, timeout=600)
            execute(["cmake", "--build", build, "-j", args.jobs], args.out / f"{name}-build.log",
                    source=src, build=build, writing=True, timeout=600)
            audit_symbols(build, args.out / f"{name}-symbols.log")
            report["binary_hashes"][name] = binary_hashes(build)
            fresh = secrets.randbits(64)
            execute(["ctest", "--test-dir", build, "--output-on-failure", "--no-tests=error", "-L", "quick"],
                    args.out / f"{name}-tests.log", source=src, build=build, writing=True,
                    env={**env,"POKER_TEST_SEED":str(fresh)}, timeout=300)
        # Reject broken candidates before spending time on either full census.
        for name, build in builds.items():
            src = args.out / name / "source"
            report["correctness"][name] = []
            for cards in range(5, exhaustive_max + 1):
                if binary_hashes(build) != report["binary_hashes"][name]:
                    raise GateError("validated executable changed during correctness checks")
                print(f"{name}: validating every {cards}-card set", flush=True)
                # Keep five-card census/multiplicity assertions in its single
                # process. Larger domains use disjoint combinatorial intervals.
                shards = 1 if cards == 5 else args.jobs
                def census(shard):
                    log = args.out / f"{name}-exhaustive-{cards}-{shard}.log"
                    execute([build / "poker_exhaustive", str(cards), str(shard), str(shards)],
                            log, source=src, build=build, timeout=14400)
                    return verify_census(log, cards, shard, shards)
                with ThreadPoolExecutor(max_workers=shards) as pool:
                    report["correctness"][name].extend(pool.map(census, range(shards)))
        rounds = 3 if args.diagnostic else policy["pairs"]
        pairs = []
        for round_number in range(rounds):
            seed = secrets.randbits(64)
            order = ["base", "candidate"]
            if secrets.randbits(1):
                order.reverse()
            pair = {}
            for name in order:
                if binary_hashes(builds[name]) != report["binary_hashes"][name]:
                    raise GateError("benchmark binary changed after validation")
                output = args.out / f"{round_number:03}-{name}.json"
                io = args.out / "io" / f"{round_number:03}-{name}"
                io.mkdir(parents=True)
                child_output = io / "measurement.json"
                observation = execute([builds[name] / "poker_bench", f"--seed={seed}", f"--records={policy['records']}",
                    f"--benchmark_out={child_output}", "--benchmark_out_format=json", "--benchmark_color=false"],
                    args.out / f"{round_number:03}-{name}.log", source=args.out / name / "source", build=builds[name],
                    output_dir=io, timeout=policy["process_timeout_seconds"])
                if child_output.is_symlink() or not child_output.is_file():
                    raise GateError("measurement must be a regular file")
                child_output.rename(output)
                shutil.rmtree(io)
                pair[name] = parse_measurement(load_json(output), seed=seed, policy=policy, observation=observation)
                report["runs"].append({"pair": round_number, "variant": name, "seed": seed, "raw_file": output.name,
                    "sha256": hashlib.sha256(output.read_bytes()).hexdigest(), **observation})
            pairs.append((pair["base"],pair["candidate"]))
            print(f"paired experiment {round_number+1}/{rounds} complete",flush=True)
        report.update(decide(pairs,args.target,policy,args.diagnostic))
        # Recheck immutable build inputs after executing candidate code.
        for name, expected in (("base",base_files),("candidate",candidate_files)):
            if inventory(args.out / name / "source") != expected:
                raise GateError("source snapshot was modified during evaluation")
            if binary_hashes(builds[name]) != report["binary_hashes"][name]:
                raise GateError("benchmark binary changed during evaluation")
    except (GateError, OSError, subprocess.SubprocessError, ValueError, RuntimeError) as exc:
        report.update(verdict="fail",eligible_for_merge=False,error=str(exc))
    (args.out / "report.json").write_text(json.dumps(report,indent=2,allow_nan=False)+"\n")
    print(json.dumps({k:report[k] for k in ("verdict","eligible_for_merge")})+f" Report: {args.out / 'report.json'}")
    if "error" in report:
        print(report["error"],file=sys.stderr)
    return {"pass":0,"fail":1,"inconclusive":3}[report["verdict"]]


if __name__ == "__main__":
    sys.exit(main())
