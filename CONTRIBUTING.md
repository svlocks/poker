# Contributing

Thanks for your interest. This project is small and the rules are few. The
short version: keep the ranks exact, don't touch the test harness in the same
PR as an optimization, and bring numbers when you claim speed.

## What we're looking for

- **Faster evaluation.** New algorithms, better tables, SIMD, anything that
  moves the benchmarks without changing results.
- **Bug reports with a hand.** If a rank is wrong, tell us the cards. The
  reference implementation makes these quick to confirm.
- **Portability fixes** for compilers or platforms in the CI matrix.
- **Documentation fixes.**

Things we generally won't take:

- Refactors or reformatting without a functional or measurable reason.
- API changes without an issue first. The API is meant to stay stable.
- Performance PRs that only help a synthetic case at the cost of others.
- Changes that relax tests, oracle checks or benchmark policy to make a
  candidate pass.

## Before you start

For anything beyond a small fix, open an issue describing what you want to
change and why. For optimizations, that saves you from duplicating work
someone else is already doing.

Build instructions, presets, and the local tooling are in
[HACKING.md](HACKING.md).

## Optimization pull requests

The evaluator implementation lives in two places:

- `src/`
- `include/poker/detail/`

An optimization PR changes **only** those files. Everything else (public
headers, tests, the reference oracle, benchmarks, CMake files, workflows,
policy) is owned by the maintainers and needs a separate PR. This is what lets
CI benchmark your change against `main` automatically: the same tests and
harness run on both sides.

When you open the PR:

1. Add one line to the description naming the workloads you expect to improve,
   for example `Benchmark targets: batch/high/7/1024, scalar/high/7`. Profile
   IDs are listed in [docs/benchmarking.md](docs/benchmarking.md). Without this
   line, any workload counts.
2. Include your own measurements: what you ran, on what hardware and compiler,
   and what you saw before and after. Inconclusive or negative results are
   worth reporting too.
3. Explain the idea. A paragraph on why it should be faster is more useful to
   a reviewer than the diff.

CI then runs the full correctness suite on every platform and a paired
benchmark on one Linux runner: `main` and your branch are built with identical
flags and timed in randomized pairs, 31 times each. A summary is posted on the
PR. The check turns red on a clear regression in any of the 149 workloads (more
than 3 percent) or a broken build. "Faster" and "inconclusive" are both green;
the reviewer reads the numbers. Hosted runners are shared machines, so a result
that surprises you is worth re-running locally before you argue with it.

Merging is manual. The bar is: all correctness checks green, a measured
improvement on the declared targets, and no regressions we're not willing to
accept.

## Everything else

Changes outside the implementation directories go through ordinary review by a
code owner. CI still runs, but the benchmark comparison is skipped because the
two sides would no longer share a harness.

## Style

- C++20, no compiler extensions. Warnings are errors in spirit: keep the build
  clean under `-Wall -Wextra -Wpedantic -Wconversion -Wshadow` and `/W4`.
- Run `git clang-format` on your commits before pushing. CI checks only the
  lines you changed, so don't reformat surrounding code.
- Keep the library free of I/O, allocation in steady state, and global mutable
  state other than the lazily built tables.
- One logical change per PR. Small commits with clear messages are appreciated
  but we squash on merge, so don't worry about history inside the PR.

## AI-assisted contributions

Using an AI tool to write or review code is fine. You are still the author: you
need to understand the change, have run it, and be able to discuss it. Please
don't submit generated PRs you haven't tested, and don't paste tool output as a
PR description. If a change was largely produced by a tool, say so.

## Reporting security issues

See [SECURITY.md](SECURITY.md).

## License

By contributing you agree that your contributions are licensed under the
project's [MIT license](LICENSE).
