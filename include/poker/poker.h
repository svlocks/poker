#ifndef POKER_POKER_H
#define POKER_POKER_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

/* Card = 4 * (rank - 2) + suit. Ranks 2..14; suits clubs, diamonds, hearts, spades.
 * Ranks: 1 strongest (royal flush), 7462 weakest. Zero is never a valid rank.
 * All unchecked calls require valid, distinct cards and sufficient output space.
 * Buffers are borrowed. Batch outputs must not overlap inputs. */
typedef uint8_t poker_card;
typedef uint16_t poker_rank;
typedef enum poker_status {
  POKER_OK = 0, POKER_INVALID_ARGUMENT, POKER_INVALID_CARD,
  POKER_DUPLICATE_CARD, POKER_INVALID_COUNT, POKER_INSUFFICIENT_MEMORY,
  POKER_UNSUPPORTED_BACKEND, POKER_INVALID_STATE
} poker_status;

typedef struct poker_backend_info {
  uint32_t abi_version;
  const char* name;
  size_t shared_bytes;
  size_t state_bytes;
  size_t state_alignment;
} poker_backend_info;

const poker_backend_info* poker_backend(void);
/* NULL or "portable". Other names fail; no silent fallback when forced. */
poker_status poker_initialize(const char* backend, size_t memory_budget);

poker_rank poker_eval5(const poker_card cards[5]);
poker_rank poker_eval6(const poker_card cards[6]);
poker_rank poker_eval7(const poker_card cards[7]);
poker_rank poker_eval_high_unchecked(const poker_card* cards, size_t count);
poker_rank poker_eval_omaha_unchecked(const poker_card* holes, size_t hole_count,
                                    const poker_card* board, size_t board_count);
poker_status poker_eval_high(const poker_card* cards, size_t count, poker_rank* out);
poker_status poker_eval_omaha(const poker_card* holes, size_t hole_count,
                            const poker_card* board, size_t board_count, poker_rank* out);

/* Packed rows: count * cards_per_hand cards; count ranks. Zero rows do no work. */
void poker_high_batch(const poker_card* cards, size_t cards_per_hand, size_t count, poker_rank* out);
void poker_omaha_batch(const poker_card* holes, size_t hole_count, const poker_card* boards,
                      size_t board_count, size_t count, poker_rank* out);
void poker_omaha_board_batch(const poker_card* holes, size_t hole_count, const poker_card* board,
                            size_t board_count, size_t count, poker_rank* out);
void poker_holdem_board_batch(const poker_card* holes, const poker_card* board,
                             size_t board_count, size_t count, poker_rank* out);

/* Caller-owned opaque states. Allocate using poker_backend()->state_bytes/alignment.
 * Prepared states are immutable after creation, shareable between threads, and
 * process-local. Budget includes shared tables plus one state. */
poker_status poker_prepare_high(void* state, size_t capacity, const poker_card* known,
                                size_t known_count, size_t final_count, size_t memory_budget);
poker_status poker_prepare_omaha(void* state, size_t capacity, const poker_card* holes,
  size_t hole_count, const poker_card* board, size_t board_count,
  size_t final_hole_count, size_t final_board_count, size_t memory_budget);
/* For unrestricted states, additional cards occupy the holes argument; board_count=0.
 * out may equal state. On error out is unchanged. */
poker_status poker_extend(const void* state, const poker_card* holes, size_t hole_count,
  const poker_card* board, size_t board_count, void* out, size_t capacity);
poker_status poker_clone(const void* state, void* out, size_t capacity);
/* Completion counts are fixed by the prepared state. For high, use holes and NULL board. */
poker_rank poker_complete_unchecked(const void* state, const poker_card* holes, const poker_card* board);
poker_status poker_complete(const void* state, const poker_card* holes, const poker_card* board, poker_rank* out);
void poker_complete_batch(const void* state, const poker_card* holes, const poker_card* boards,
                          size_t count, poker_rank* out);

#ifdef __cplusplus
}
#endif
#endif
