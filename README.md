# poker

[![CI](https://github.com/svlocks/poker/actions/workflows/ci.yml/badge.svg)](https://github.com/svlocks/poker/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

A fast, exact poker hand evaluator in C++20 with a plain C interface.

Give it five to fifty-two cards and it returns the rank of the best five-card
hand as a single integer: **1 is a royal flush, 7462 is 7-5-4-3-2 offsuit**.
Equal ranks tie. Omaha (exactly two hole cards plus three board cards) is
supported for any hole and board sizes, along with batch evaluation, shared
boards, and reusable prepared states for Monte Carlo loops.

Every hand in the 5, 6 and 7-card domains is checked against an independent
reference on every change. See [how it is tested](#testing).

## Quick start

```cmake
include(FetchContent)
FetchContent_Declare(poker
  GIT_REPOSITORY https://github.com/svlocks/poker.git
  GIT_TAG        main)   # or a release tag such as v0.1.0
FetchContent_MakeAvailable(poker)
target_link_libraries(your_target PRIVATE poker::evaluator)
```

Or build and install it, then `find_package(poker CONFIG REQUIRED)`:

```sh
cmake -S . -B build -DPOKER_BUILD_TESTS=OFF -DPOKER_BUILD_BENCHMARKS=OFF
cmake --build build
cmake --install build --prefix /your/prefix
```

The installed library has no dependencies beyond the C++ runtime.

## Usage

Cards are bytes: `4 * (rank - 2) + suit`, with ranks 2 to 14 (ace) and suits
clubs = 0, diamonds = 1, hearts = 2, spades = 3. The ace of spades is 51.

### C++

```cpp
#include <poker/poker.hpp>

// Ah Kh Qh Jh Th 2c 3d
std::array<poker::card, 7> hand{50, 46, 42, 38, 34, 0, 5};
poker::rank r = poker::evaluate(hand);          // 1, a royal flush

std::array<poker::card, 4> holes{51, 47, 0, 1};  // As Ks 2c 2d
std::array<poker::card, 5> board{50, 46, 42, 4, 8};
poker::rank o = poker::evaluate_omaha(holes, board);
```

The templates are fixed-size and header-visible, so the compiler can inline
the evaluator into your loop.

### C

```c
#include <poker/poker.h>

poker_card hand[7] = {50, 46, 42, 38, 34, 0, 5};
poker_rank r = poker_eval7(hand);                       /* no validation */

poker_rank checked;
if (poker_eval_high(hand, 7, &checked) == POKER_OK) { /* validated input */ }

/* 1000 seven-card hands packed back to back */
poker_rank out[1000];
poker_high_batch(packed_cards, 7, 1000, out);
```

Functions with `_unchecked` in the name, the fixed-size `poker_eval5/6/7`
calls, and batch functions trust their input. The checked variants return a
`poker_status` and reject bad counts, invalid cards and duplicates.

### Prepared states

When many hands share known cards, prepare once and complete many times:

```c
const poker_backend_info* info = poker_backend();
void* state = aligned_alloc(info->state_alignment, info->state_bytes);
poker_card known[2] = {51, 47};                          /* As Ks */
poker_prepare_high(state, info->state_bytes, known, 2, 7, 1 << 20);

poker_card runout[5] = {/* five board cards */};
poker_rank r = poker_complete_unchecked(state, runout, NULL);
```

States are immutable once prepared, so threads can share them freely.
`poker_extend` derives a new state with more known cards, and
`poker_complete_batch` finishes many runouts at once.

The full API is documented in [docs/api.md](docs/api.md).

## Performance

Every merge to `main` re-measures all 149 benchmark workloads on a
GitHub-hosted Linux runner and publishes the report, with the tested commit,
CPU and compiler, to the `benchmark-results` branch:

**[Latest benchmark report](https://github.com/svlocks/poker/blob/benchmark-results/latest.md)**

Shared CI machines are noisy, so treat those figures as ballpark numbers for a
portable build rather than a claim of record speed. Pull requests are compared
against `main` in paired runs on one machine; the methodology is in
[docs/benchmarking.md](docs/benchmarking.md).

## Testing

The test suite compares the evaluator against an independently written
reference implementation:

- every one of the 2,598,960 five-card hands, plus the category and
  per-rank census that pins down the 7462 distinct values;
- every six-card (20,358,520) and seven-card (133,784,560) hand, sharded
  across CI jobs;
- randomized differential and metamorphic tests for larger hands, Omaha,
  batches and prepared states;
- address, undefined-behavior and thread sanitizers, a libFuzzer target, and
  an installed-package consumer test;
- Linux (GCC, Clang, x86-64 and arm64), macOS and Windows (MSVC).

Build and run it yourself:

```sh
cmake --preset release
cmake --build --preset release
ctest --preset quick       # about a minute
ctest --preset release     # adds every five-card hand
```

## Contributing

Pull requests are welcome, especially ones that make it faster. Optimization
PRs get an automatic paired benchmark against `main` on the same runner, and a
summary is posted on the PR. Read [CONTRIBUTING.md](CONTRIBUTING.md) first; it
is short. [HACKING.md](HACKING.md) covers the layout, presets and tooling.

## License

[MIT](LICENSE).
