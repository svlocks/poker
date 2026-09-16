#!/usr/bin/env python3
"""Render a gate report as Markdown for a job summary or pull request comment."""
import argparse
from pathlib import Path
import sys

import gate

MARKER = "<!-- poker-benchmark-report -->"


def fmt_ratio(value):
    return f"{(value - 1) * 100:+.1f}%"


def render(report, *, title="Benchmark comparison", max_rows=None):
    lines = [MARKER, f"## {title}", ""]
    if report.get("error"):
        lines += [f"**Gate error:** {report['error']}", ""]
        return "\n".join(lines)
    verdict = report.get("verdict", "unknown")
    policy = report.get("policy", {})
    targets = report.get("targets", [])
    pairs = len(report.get("runs", [])) // 2
    headline = {"pass": "Declared targets improved and no profile regressed beyond the guard.",
                "fail": "At least one profile clearly regressed.",
                "inconclusive": "No clear verdict: either no target improved past the threshold, or some intervals were too wide."}
    lines += [f"**Verdict: {verdict}.** {headline.get(verdict, '')}", "",
              f"{pairs} paired runs of {policy.get('records', '?')} records per profile. "
              f"Improvement threshold {policy.get('minimum_improvement', 0) * 100:.0f}%, "
              f"regression guard {policy.get('maximum_regression', 0) * 100:.0f}%. "
              f"Targets: {', '.join(f'`{t}`' for t in targets) if targets else 'none declared (any profile counts)'}.", ""]
    rows = []
    for name, row in report.get("profiles", {}).items():
        interval = row.get("interval")
        rows.append((name, row["median_ratio"], interval))
    guard = 1 + policy.get("maximum_regression", 0)
    gain = 1 - policy.get("minimum_improvement", 0)

    def flag(name, interval):
        if interval is None:
            return "wide"
        lo, hi = interval
        if hi < gain:
            return "faster" + (" (target)" if name in targets else "")
        if lo > guard:
            return "**regressed**"
        if hi > guard:
            return "unclear"
        return ""

    def cell(interval):
        if interval is None:
            return "n/a"
        return f"{fmt_ratio(interval[0])} to {fmt_ratio(interval[1])}"

    interesting = [r for r in rows if r[0] in targets or flag(r[0], r[2])]
    interesting.sort(key=lambda r: r[1])
    lines += ["| Profile | Median change | 95% interval | |", "| --- | ---: | ---: | --- |"]
    for name, median, interval in interesting[:max_rows]:
        lines.append(f"| `{name}` | {fmt_ratio(median)} | {cell(interval)} | {flag(name, interval)} |")
    if not interesting:
        lines.append("| _no profile moved past the thresholds_ | | | |")
    lines += ["", "<details><summary>All profiles</summary>", "",
              "| Profile | Median change | 95% interval |", "| --- | ---: | ---: |"]
    for name, median, interval in sorted(rows):
        lines.append(f"| `{name}` | {fmt_ratio(median)} | {cell(interval)} |")
    lines += ["", "</details>", ""]
    env = report.get("environment", {})
    machine = env.get("machine", "?")
    compiler = (env.get("compiler") or "?").splitlines()[0]
    lines += [f"Negative is faster. Both revisions were built and timed on the same runner ({machine}, {compiler}) "
              "in randomized paired order; noise affects both sides of each pair. Hosted runners vary, so treat "
              "this as evidence to read, not a verdict to trust blindly.", ""]
    return "\n".join(lines)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--report", type=Path, required=True)
    parser.add_argument("--out", type=Path, help="write Markdown here instead of stdout")
    parser.add_argument("--title", default="Benchmark comparison")
    args = parser.parse_args()
    text = render(gate.load_json(args.report), title=args.title)
    if args.out:
        args.out.write_text(text)
    else:
        sys.stdout.write(text)
    return 0


if __name__ == "__main__":
    sys.exit(main())
