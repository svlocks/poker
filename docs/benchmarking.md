# Benchmarking

How performance is measured, how pull requests are compared with `main`, and
where the numbers end up.

## What is measured

`poker_bench` defines 149 profiles. Each fixes an input shape and an amount of
work, so a candidate cannot improve a score by changing the batch size or
skipping preparation. Groups:

| Prefix | Meaning |
| --- | --- |
| `scalar/high/N` | N-card hands through the C API, one at a time, each input address depending on the previous result |
| `native/high/N` | The same through the header-visible C++ template |
| `batch/high/N/B` | Packed batches of B hands (N = 5, 6, 7, 9; B = 1, 8, 31, 128, 1024) |
| `scalar/omaha/H/B`, `batch/omaha/H/B/N` | Omaha with H hole and B board cards |
| `shared/holdem/B/N`, `shared/omaha/H/B/N` | N players against one shared board |
| `prepared/…`, `total/…` | Completing a prepared state; `total` also includes preparing it |
| `state/clone/…`, `state/extend/…` | One state operation |
| `simulation/holdem/P`, `simulation/omaha/P` | A complete P-player Monte Carlo trial: deal, evaluate everyone, accumulate |
| `parallel/…/T` | T worker threads on prestarted workers |
| `startup/initialize` | Table initialization in a fresh process |

The `scalar` figures include a harness-imposed dependency chain and are a
latency-style measurement. Batch and simulation figures are throughput.
`tools/gate.py` has the full list (`profile_names()`).

## Fresh inputs, verified outputs

Every benchmark process gets a new 64-bit seed chosen *after* the binaries are
built. A specified SplitMix64 generator deals the corpus so runs are
reproducible across standard libraries, and each profile mixes in its name so
profiles do not share data. Google Benchmark runs exactly one timed traversal
per profile per process: no warm-up over the same hands, no repeats to pick
from.

After timing, every output is checked against the oracle. Missing or wrong
ranks, mutated inputs, wrong batch or trial counts, skipped rows, aggregate-only
rows, duplicate JSON keys, non-finite values and rate/time fields that disagree
all fail the run. Simulations keep every individual rank and compare them
exactly.

The parent process also records wall time and peak RSS for each child and
enforces the caps in `benchmarks/policy.json`: 256 MiB process RSS, 1 MiB of
shared tables, 4 KiB per state, and one second to initialize. A symbol audit
of the built library rejects definitions of `clock_gettime`, `main`, `exit`
and similar, so a candidate cannot interpose on the timer or the reference.

## Comparing a PR against main

For an implementation-only PR, CI checks out the PR base and the PR merge
commit side by side and runs the base tree's `tools/gate.py`:

1. Both trees are hashed. Only `src/` and `include/poker/detail/` may differ.
2. Both are built with the same portable Release flags and pass the quick
   tests under a fresh seed. The six- and seven-card censuses run in separate
   CI jobs, so the gate skips them here.
3. 31 paired experiments run. Each pair uses one new seed for both sides and a
   coin flip decides which side runs first. Binary hashes are re-checked before
   every run.
4. For every profile, the 31 candidate/base time ratios are summarized by an
   exact binomial order-statistic confidence interval for the median, with a
   Bonferroni adjustment across all 149 profiles (family error 5 percent).

The verdict:

- **pass**: every profile's upper bound is at most 1.03, and at least one
  declared target's upper bound is below 0.99. Without a declared target, any
  profile may supply the improvement.
- **fail**: some profile's lower bound is above 1.03.
- **inconclusive**: anything else, including intervals too wide to call.

The CI job fails only on **fail** or a gate error. The summary, with intervals
for every profile, goes to the job summary and a PR comment. The 3 percent
regression guard and 1 percent improvement threshold are in
`benchmarks/policy.json`; they are judgment calls for a shared runner, not
statements about what that runner can resolve.

Two things this does not do: it does not certify anything about hardware other
than the runner it ran on, and it does not make a wrong evaluator look right.
Correctness comes from the test jobs, not the benchmark.

## Published baselines

After each push to `main`, CI builds that commit, runs `poker_bench` five times
with fresh seeds, validates every run as above, and publishes:

- `latest.md` and `latest.json` on the `benchmark-results` branch, and
- `history/<commit>/<report-hash>.json` for every snapshot.

Raw benchmark output is kept as a workflow artifact for 30 days. The publisher
checks that `main` still points at the tested commit before pushing and uses a
separate index, so it never touches the source tree or `main` itself. A
repository rule (`.github/rulesets/benchmark-results.json`) blocks force pushes
to and deletion of the results branch, so history there is append-only.

These reports show medians and ranges from one hosted runner. Comparing two of
them from different days tells you less than the paired comparison above,
because the machine may have changed.

## Running it yourself

See [HACKING.md](../HACKING.md#benchmarks). Local hardware you control, with a
fixed CPU set (`--cpus`) and nothing else running, gives much tighter intervals
than CI.
