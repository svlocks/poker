# Changelog

All notable changes to this project are documented here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and versions follow
[Semantic Versioning](https://semver.org/).

## [Unreleased]

## [0.1.0] - 2026-09-16

First public release.

### Added

- Exact evaluation of 5 to 52 card hands with a C API and header-visible C++
  templates; ranks follow Cactus Kev numbering (1 to 7462).
- Omaha evaluation for any hole and board sizes, packed batches, shared-board
  batches, and immutable prepared states with clone, extend and completion.
- A portable CPU backend using rank-mask classification and a perfect hash for
  five, six and seven cards, with table-driven Omaha grouping.
- Test suite with an independent oracle, every 5/6/7-card hand checked,
  randomized differential tests, sanitizer and fuzz targets, and an
  installed-package consumer test.
- A paired A/B benchmark harness (`tools/gate.py`) with output verification and
  order-statistic confidence intervals, run automatically on pull requests.
- Published baseline benchmarks on the `benchmark-results` branch after every
  merge to `main`.

[Unreleased]: https://github.com/svlocks/poker/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/svlocks/poker/releases/tag/v0.1.0
