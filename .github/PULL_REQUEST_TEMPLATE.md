<!-- Thanks for the PR. See CONTRIBUTING.md for what reviewers look for. -->

## What and why

<!-- What does this change, and why should it be faster / more correct / better? -->

## Type of change

- [ ] Optimization (only `src/` and `include/poker/detail/` changed)
- [ ] Bug fix
- [ ] API, build, tests or tooling
- [ ] Documentation

## Benchmarks (optimizations only)

<!-- Keep this exact line, edit the list. Profile IDs: docs/benchmarking.md. -->
Benchmark targets: batch/high/7/1024

<!-- Your own numbers: hardware, compiler, what you ran, before/after. -->

## Checklist

- [ ] `ctest --preset release` passes locally
- [ ] Ranks are unchanged (no test, oracle or policy edits)
- [ ] Changed lines are `git clang-format` clean
