# Notes for coding agents

Read [CONTRIBUTING.md](CONTRIBUTING.md) and [HACKING.md](HACKING.md); the
rules there apply to you too. In particular:

- Optimization changes go in `src/` and `include/poker/detail/` only. Do not
  edit tests, the oracle, benchmarks, CMake files, workflows or policy to make
  a candidate pass. If a check fails, the candidate is wrong until shown
  otherwise.
- Ranks are exact. Keep every checked/unchecked, batch and state semantic in
  [docs/api.md](docs/api.md).
- Run `ctest --preset quick` while iterating and `ctest --preset release`
  before opening a PR. Use `tools/gate.py` against a clean `main` checkout for
  performance claims and report the numbers, including inconclusive ones.
- Do not claim a speedup from one run, from a debug build, or from hardware
  you did not measure on.
- Do not push to `main`, `benchmark-results` or `validation/*`, create tags,
  or change repository settings.
