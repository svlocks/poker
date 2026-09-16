# API reference

The public surface is `include/poker/poker.h` (C) and `include/poker/poker.hpp`
(C++ templates). Everything under `include/poker/detail/` is internal and
changes without notice.

## Cards and ranks

- One standard 52-card deck, no jokers, standard high-hand rankings.
- A card is `4 * (rank - 2) + suit`: ranks 2 to 14 (ace is 14), suits clubs 0,
  diamonds 1, hearts 2, spades 3. So `2c` is 0 and `As` is 51.
- A rank is a 16-bit integer from 1 (royal flush) to 7462 (7-5-4-3-2 offsuit),
  following Cactus Kev's numbering. Lower is stronger, equal values tie, suits
  never break ties, and 0 is never a valid rank.
- A-2-3-4-5 is the lowest straight. Q-K-A-2-3 is not a straight.

Unrestricted ("high") evaluation takes 5 to 52 distinct cards and finds the
best five. Omaha takes at least two hole cards and at least three board cards
(at most 52 in total) and always uses exactly two hole cards and three board
cards. Larger hole or board counts are mathematically defined extensions, not
a claim about real deal sizes. Hi/Lo and short decks are out of scope.

## Checked and unchecked calls

```c
poker_status poker_eval_high(const poker_card*, size_t count, poker_rank* out);
poker_status poker_eval_omaha(const poker_card* holes, size_t, const poker_card* board, size_t, poker_rank* out);
```

The checked functions validate counts first, then card values and duplicates
across both groups, and write `*out` only on `POKER_OK`. They cannot make an
invalid pointer safe; the caller supplies readable storage of the stated size.

```c
poker_rank poker_eval5(const poker_card[5]);
poker_rank poker_eval6(const poker_card[6]);
poker_rank poker_eval7(const poker_card[7]);
poker_rank poker_eval_high_unchecked(const poker_card*, size_t count);
poker_rank poker_eval_omaha_unchecked(const poker_card* holes, size_t, const poker_card* board, size_t);
```

Unchecked calls, the fixed-size calls, the C++ templates and all batch
functions require valid, distinct cards and correct counts. Passing bad input
is undefined behavior, not an error you can detect afterwards.

```cpp
template <std::size_t N> poker::rank poker::evaluate(const std::array<poker::card, N>&);
template <std::size_t H, std::size_t B> poker::rank poker::evaluate_omaha(const std::array<card, H>&, const std::array<card, B>&);
```

The templates expose the implementation to the compiler rather than wrapping a
call, so they inline into the caller.

## Batches

```c
void poker_high_batch(const poker_card* cards, size_t cards_per_hand, size_t count, poker_rank* out);
void poker_omaha_batch(const poker_card* holes, size_t hole_count, const poker_card* boards, size_t board_count, size_t count, poker_rank* out);
void poker_omaha_board_batch(const poker_card* holes, size_t hole_count, const poker_card* board, size_t board_count, size_t count, poker_rank* out);
void poker_holdem_board_batch(const poker_card* holes, const poker_card* board, size_t board_count, size_t count, poker_rank* out);
```

Inputs are packed rows with no padding, borrowed for the call and never
modified. `out` must have room for `count` ranks and must not overlap the
inputs. A zero-length batch does nothing, even with null pointers. The Hold'em
shared-board batch takes two hole cards per row and 3 to 5 board cards; the
others follow the scalar rules for counts.

## Prepared states

```c
const poker_backend_info* poker_backend(void);   /* state_bytes, state_alignment, shared_bytes */
poker_status poker_prepare_high(void* state, size_t capacity, const poker_card* known, size_t known_count, size_t final_count, size_t memory_budget);
poker_status poker_prepare_omaha(void* state, size_t capacity, const poker_card* holes, size_t hole_count, const poker_card* board, size_t board_count, size_t final_hole_count, size_t final_board_count, size_t memory_budget);
poker_status poker_clone(const void* state, void* out, size_t capacity);
poker_status poker_extend(const void* state, const poker_card* holes, size_t hole_count, const poker_card* board, size_t board_count, void* out, size_t capacity);
poker_rank   poker_complete_unchecked(const void* state, const poker_card* holes, const poker_card* board);
poker_status poker_complete(const void* state, const poker_card* holes, const poker_card* board, poker_rank* out);
void         poker_complete_batch(const void* state, const poker_card* holes, const poker_card* boards, size_t count, poker_rank* out);
```

A state is caller-owned opaque storage. Ask `poker_backend()` for its size and
alignment and hand over that much correctly aligned memory. Preparing fixes
the final hand shape, records the known cards (zero is fine), and needs no
destructor. Completing supplies exactly the missing cards; for unrestricted
states they go in `holes` and `board` is null. Completion batches pack the
missing cards into rows.

`poker_clone` copies a state. `poker_extend` makes a new state with more known
cards and may write back to the same buffer. Neither `complete` variant ever
mutates the state, so independent states, and shared states that nobody is
extending, are safe to use from many threads at once. Do not extend a state
while another thread is completing from it.

States are process-local. Don't serialize them or pass them between library
versions. Checked state calls detect malformed metadata but cannot validate an
arbitrary pointer.

## Initialization and memory

```c
poker_status poker_initialize(const char* backend, size_t memory_budget);
```

Pass `NULL` or `"portable"`. Unknown names fail; a forced backend never
silently falls back. Rank tables are also built on first use if you skip
explicit initialization, and initialization is thread-safe. Direct evaluation
needs no state object.

`memory_budget` covers the shared tables plus one state. It excludes your own
buffers, stack, the C++ runtime, code pages and transient memory used while
building tables. `poker_backend()->shared_bytes` reports the table size; the
benchmark policy separately caps whole-process RSS. Steady-state evaluation,
completion, clone and extend never allocate.

Input size affects runtime a lot. Correctness holds for every legal size; speed
at unusual sizes is not a promise.

## Compatibility

Public headers, memory semantics, the reference tests and the benchmark policy
are stable. Optimization changes stay inside `src/` and
`include/poker/detail/`. Adding a backend, changing the ABI or adding a batch
layout is an API change: open an issue first, and expect it to come with its
own tests and benchmark profiles.

The C interface is currently meant for static linking. A versioned shared
library ABI is future work.
