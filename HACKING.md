# Hacking on poker

Development notes: layout, building, testing, benchmarking and how CI is put
together. For the contribution rules see [CONTRIBUTING.md](CONTRIBUTING.md).

## Layout

```
include/poker/poker.h          C API (public, stable)
include/poker/poker.hpp        C++ templates (public, stable)
include/poker/detail/          evaluator internals (optimization PRs edit here)
src/                           evaluator implementation (and here)
tests/                         GoogleTest suite, independent oracle, exhaustive checker, fuzzer
benchmarks/                    Google Benchmark driver and policy.json
tools/                         Python: gate.py (paired A/B), record/publish baselines, CI helpers
docs/                          API reference and benchmarking methodology
.github/workflows/             ci.yml, benchmark-comment.yml, release.yml
```

`tests/oracle.cpp` is a deliberately slow, separately written evaluator. It
never calls into the library, and the library never sees it. Every test and
benchmark compares against it.

## Requirements

A C++20 compiler (GCC 12+, Clang 15+, MSVC 2022), CMake 3.25+, Ninja, Python
3.10+ and Git. GoogleTest and Google Benchmark are fetched at pinned commits on
the first configure; later builds reuse the checkout under `build/*/_deps`.

## Building and testing

```sh
cmake --preset release
cmake --build --preset release -j
ctest --preset quick          # unit, C consumer, install consumer, tool tests, benchmark smoke
ctest --preset release        # quick + every five-card hand (8 shards)
```

Other configure presets: `debug`, `native` (`-march=native` + LTO), `clang`,
`asan`, `tsan`, `fuzz` (Clang libFuzzer) and `exhaustive` (also registers the
6 and 7-card domains, which take a while). Each has a matching build preset,
and `asan`, `tsan` and `exhaustive` have test presets.

To check a whole domain in one process, including the five-card category and
rank-multiplicity census:

```sh
build/release/poker_exhaustive 5 0 1        # CARDS SHARD SHARDS
build/release/poker_exhaustive 7 3 8        # shard 3 of 8 for seven cards
```

The fuzzer:

```sh
cmake --preset fuzz && cmake --build --preset fuzz
build/fuzz/poker_fuzz -runs=100000 -max_len=12
```

Tests take a seed from `POKER_TEST_SEED` so a randomized failure can be
replayed.

## Benchmarks

`poker_bench` runs all 149 profiles once each with a fresh corpus and verifies
every timed output against the oracle:

```sh
build/release/poker_bench --seed=12345 --records=512 \
  --benchmark_out=build/release/run.json --benchmark_out_format=json
```

Use a new seed per run. `--smoke` runs a tiny corpus and is what `ctest` uses.

### Comparing two trees

`tools/gate.py` is the paired A/B harness CI uses. It builds a base and a
candidate tree with identical flags, runs the quick tests on both, optionally
runs the exhaustive census, and then times them in randomized pairs with the
same seed per pair. Run the copy of the script that belongs to the base tree:

```sh
git worktree add ../poker-main main
python3 ../poker-main/tools/gate.py --base ../poker-main --candidate . \
  --out .artifacts/compare-1 --target batch/high/7/1024 --exhaustive-max-cards 0
```

Exit code 0 means the targets improved and nothing regressed beyond the guard,
1 means a clear regression or an error, 3 means inconclusive. Add
`--diagnostic` for a three-pair smoke comparison (never "passes"), and render
`report.json` as Markdown with `tools/gate_summary.py`. Only `src/` and
`include/poker/detail/` may differ between the two trees; the gate refuses
anything else so the harness itself is never part of the comparison.

`tools/mutation_check.py` builds several deliberately wrong evaluators and
confirms the tests reject each one. Run it after changing tests or the oracle.

### Recording a baseline

```sh
python3 tools/record_benchmarks.py --build build/release --out .artifacts/baseline --samples 5
```

CI does this on every push to `main` and publishes the result to the
`benchmark-results` branch. See [docs/benchmarking.md](docs/benchmarking.md).

## CI

Everything runs from `.github/workflows/ci.yml` on pull requests and pushes to
`main`:

| Job | What |
| --- | --- |
| `scope` | Classifies a PR as implementation-only or maintenance; reads `Benchmark targets:` |
| `test (linux-gcc, linux-clang, linux-arm, macos, windows)` | Release build, full `ctest` including every five-card hand |
| `sanitizer (asan, tsan)` | Quick suite under ASan+UBSan and TSan |
| `exhaustive (6|7, 0..3)` | Every six- and seven-card hand, four shards each |
| `lint` | `git clang-format` on changed lines |
| `benchmark` | Paired comparison against the PR base; implementation-only PRs |
| `record`, `publish` | Push to `main` only: measure and publish the baseline |

`benchmark-comment.yml` runs after CI in the trusted context and posts the
benchmark summary on the PR. It executes no code from the PR.

Workflows on a fork's first PR wait for a maintainer to approve them. That is
GitHub's default and the only manual step before merge.

Lint workflow files locally with [actionlint](https://github.com/rhysd/actionlint).

## Releasing

1. Update the version in `CMakeLists.txt` (`project(... VERSION x.y.z)`).
2. Move the `Unreleased` notes in `CHANGELOG.md` under a `## [x.y.z] - date` heading.
3. Merge, then tag: `git tag -a vx.y.z -m "vx.y.z" && git push origin vx.y.z`.

`release.yml` checks that the tag, the CMake version and the changelog agree,
runs the quick suite, and creates the GitHub release with the changelog section
as notes.
