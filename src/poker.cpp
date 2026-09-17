#include <poker/poker.h>
#include <poker/detail/portable.hpp>
#include <array>
#include <cstddef>
#include <cstring>

namespace {
constexpr std::uint32_t magic = 0x504b5231;
struct State {
  std::uint32_t tag = magic;
  std::uint8_t omaha = 0;
  std::uint8_t nh = 0, nb = 0, final_h = 0, final_b = 0;
  // Holes occupy [0, final_h); the board occupies [final_h, final_h + final_b).
  // Both groups are fixed at prepare time, so a single array holds every known
  // card without holes and board regions that are never both full.
  std::array<poker_card, 52> cards{};
};
// The state is opaque to callers: only the header and the cards that are
// actually present are ever read back. Moving those bytes alone keeps clone,
// extend, prepare and completion independent of the unused tail, whose size is
// caller-owned scratch space.
struct Header {
  std::uint32_t tag;
  std::uint8_t omaha, nh, nb, final_h, final_b;
};
constexpr std::size_t header_bytes = offsetof(State, cards);
constexpr std::size_t cards_offset = offsetof(State, cards);
static_assert(sizeof(Header) >= header_bytes);
constexpr poker_backend_info info{1, "portable", poker::detail::shared_bytes, sizeof(State), alignof(State)};

bool storage_ok(const void* p, std::size_t capacity) {
  return p && capacity >= sizeof(State) && reinterpret_cast<std::uintptr_t>(p) % alignof(State) == 0;
}
bool read_header(const void* ptr, Header& head) {
  if (!storage_ok(ptr, sizeof(State))) return false;
  std::memcpy(&head, ptr, header_bytes);
  return head.tag == magic && head.omaha <= 1 && head.nh <= head.final_h && head.nb <= head.final_b &&
    head.final_h + head.final_b <= 52 &&
    (head.omaha ? (head.final_h >= 2 && head.final_b >= 3) : (head.final_h >= 5 && head.final_b == 0));
}
void write_header(void* ptr, const Header& head) { std::memcpy(ptr, &head, header_bytes); }
const poker_card* holes_of(const void* ptr) {
  return reinterpret_cast<const poker_card*>(static_cast<const unsigned char*>(ptr) + cards_offset);
}
// The board is stored directly after the holes, whose count is fixed by the
// state header, so only the header is needed to locate it.
const poker_card* board_of(const void* ptr, std::uint8_t final_h) {
  return holes_of(ptr) + final_h;
}
poker_card* holes_of(void* ptr) {
  return reinterpret_cast<poker_card*>(static_cast<unsigned char*>(ptr) + cards_offset);
}
poker_card* board_of(void* ptr, std::uint8_t final_h) {
  return holes_of(ptr) + final_h;
}
poker_status validate(const poker_card* cards, std::size_t n, std::uint64_t& used) {
  if (n && !cards) return POKER_INVALID_ARGUMENT;
  for (std::size_t i = 0; i < n; ++i) {
    if (cards[i] >= 52) return POKER_INVALID_CARD;
    const auto bit = std::uint64_t{1} << cards[i];
    if (used & bit) return POKER_DUPLICATE_CARD;
    used |= bit;
  }
  return POKER_OK;
}
poker_status validate_two(const poker_card* h, std::size_t nh, const poker_card* b, std::size_t nb) {
  std::uint64_t used = 0;
  const auto status = validate(h, nh, used);
  return status == POKER_OK ? validate(b, nb, used) : status;
}
// Ranks the hand described by a state plus the cards that complete it. The
// completed hand is assembled in the frame; the state itself is not copied.
poker_rank completed(const void* state, const poker_card* h, const poker_card* b) {
  Header head{};
  std::memcpy(&head, state, header_bytes);
  const auto missing_h = static_cast<std::size_t>(head.final_h - head.nh);
  const auto missing_b = static_cast<std::size_t>(head.final_b - head.nb);
  if (head.omaha) {
    poker_card holes[52];
    poker_card board[52];
    if (head.nh) std::memcpy(holes, holes_of(state), head.nh);
    if (missing_h) std::memcpy(holes + head.nh, h, missing_h);
    if (head.nb) std::memcpy(board, board_of(state, head.final_h), head.nb);
    if (missing_b) std::memcpy(board + head.nb, b, missing_b);
    return poker::detail::omaha(holes, head.final_h, board, head.final_b);
  }
  poker_card cards[52];
  if (head.nh) std::memcpy(cards, holes_of(state), head.nh);
  if (missing_h) std::memcpy(cards + head.nh, h, missing_h);
  return poker::detail::high(cards, head.final_h);
}
} // namespace

extern "C" {
const poker_backend_info* poker_backend(void) { return &info; }
poker_status poker_initialize(const char* backend, std::size_t budget) {
  if (backend && std::strcmp(backend, info.name)) return POKER_UNSUPPORTED_BACKEND;
  if (budget < info.shared_bytes) return POKER_INSUFFICIENT_MEMORY;
  poker::detail::initialize();
  return POKER_OK;
}
poker_rank poker_eval5(const poker_card* cards) { return poker::detail::five(cards); }
poker_rank poker_eval6(const poker_card* cards) { return poker::detail::high(cards, 6); }
poker_rank poker_eval7(const poker_card* cards) { return poker::detail::high(cards, 7); }
poker_rank poker_eval_high_unchecked(const poker_card* cards, std::size_t n) { return poker::detail::high(cards, n); }
poker_rank poker_eval_omaha_unchecked(const poker_card* h, std::size_t nh, const poker_card* b, std::size_t nb) {
  return poker::detail::omaha(h, nh, b, nb);
}
poker_status poker_eval_high(const poker_card* cards, std::size_t n, poker_rank* out) {
  if (!out) return POKER_INVALID_ARGUMENT;
  if (n < 5 || n > 52) return POKER_INVALID_COUNT;
  const auto status = validate_two(cards, n, nullptr, 0);
  if (status != POKER_OK) return status;
  *out = poker_eval_high_unchecked(cards, n);
  return POKER_OK;
}
poker_status poker_eval_omaha(const poker_card* h, std::size_t nh, const poker_card* b, std::size_t nb, poker_rank* out) {
  if (!out) return POKER_INVALID_ARGUMENT;
  if (nh < 2 || nb < 3 || nh > 49 || nb > 50 || nh + nb > 52) return POKER_INVALID_COUNT;
  const auto status = validate_two(h, nh, b, nb);
  if (status != POKER_OK) return status;
  *out = poker_eval_omaha_unchecked(h, nh, b, nb);
  return POKER_OK;
}
void poker_high_batch(const poker_card* cards, std::size_t n, std::size_t count, poker_rank* out) {
  // The hand size is fixed for the batch, so the dispatch moves out of the loop
  // and each case gets a loop that knows its own size. That is worth about four
  // percent on six and seven card batches.
  switch (n) {
    case 5:
      for (std::size_t i = 0; i < count; ++i) out[i] = poker::detail::five(cards + i * 5);
      return;
    case 6:
      for (std::size_t i = 0; i < count; ++i) {
        const poker_card* hand = cards + i * 6;
        const auto total = poker::detail::accumulate(hand, 6);
        out[i] = poker::detail::finish_six(total, hand, 6, nullptr, 0);
      }
      return;
    case 7:
      for (std::size_t i = 0; i < count; ++i) {
        const poker_card* hand = cards + i * 7;
        const auto total = poker::detail::accumulate(hand, 7);
        out[i] = poker::detail::finish_seven(total, hand, 7, nullptr, 0);
      }
      return;
    default:
      for (std::size_t i = 0; i < count; ++i) out[i] = poker_eval_high_unchecked(cards + i * n, n);
      return;
  }
}
void poker_omaha_batch(const poker_card* h, std::size_t nh, const poker_card* b, std::size_t nb, std::size_t count, poker_rank* out) {
  for (std::size_t i = 0; i < count; ++i) out[i] = poker_eval_omaha_unchecked(h + i * nh, nh, b + i * nb, nb);
}
void poker_omaha_board_batch(const poker_card* h, std::size_t nh, const poker_card* b, std::size_t nb, std::size_t count, poker_rank* out) {
  if (!count) return;
  if (nh > poker::detail::max_omaha_groups || nb > poker::detail::max_omaha_groups) {
    for (std::size_t i = 0; i < count; ++i) out[i] = poker_eval_omaha_unchecked(h + i * nh, nh, b, nb);
    return;
  }
  // The board is shared: its three-card groups and suit counts are built once.
  std::uint16_t triples[poker::detail::max_omaha_triples];
  const unsigned triple_count = poker::detail::omaha_triple_codes(b, nb, triples);
  const std::uint32_t board_lanes = poker::detail::suit_lanes(b, nb);
  for (std::size_t i = 0; i < count; ++i) {
    const poker_card* holes = h + i * nh;
    std::uint32_t pairs[poker::detail::max_omaha_pairs];
    const unsigned pair_count = poker::detail::omaha_pair_offsets(holes, nh, pairs);
    out[i] = poker::detail::omaha_from_groups(pairs, pair_count, triples, triple_count, holes, nh, b, nb,
                                              poker::detail::suit_lanes(holes, nh), board_lanes);
  }
}
void poker_holdem_board_batch(const poker_card* h, const poker_card* b, std::size_t nb, std::size_t count, poker_rank* out) {
  if (!count) return;
  // The board is shared by every row, so its rank masks are built once and each
  // row only adds its two hole cards.
  const auto board = poker::detail::accumulate(b, nb);
  // A two-card hand plus a board of three to five cards is five to seven cards,
  // which the perfect-hash tables cover; larger boards keep the arithmetic path.
  const std::size_t hand = nb + 2;
  for (std::size_t i = 0; i < count; ++i) {
    auto total = board;
    total.add(h[i * 2]);
    total.add(h[i * 2 + 1]);
    poker_rank result = poker::detail::classify_sized(total, hand);
    if (poker::detail::any_five_in_a_suit(total.h))
      result = std::min(result, poker::detail::suited_best(total, h + i * 2, 2, b, nb));
    out[i] = result;
  }
}
poker_status poker_prepare_omaha(void* state, std::size_t capacity, const poker_card* h,
  std::size_t nh, const poker_card* b, std::size_t nb, std::size_t fh, std::size_t fb, std::size_t budget) {
  if (!storage_ok(state, capacity)) return POKER_INVALID_ARGUMENT;
  if (fh < 2 || fb < 3 || fh > 49 || fb > 50 || fh + fb > 52 || nh > fh || nb > fb) return POKER_INVALID_COUNT;
  if (budget < info.shared_bytes + info.state_bytes) return POKER_INSUFFICIENT_MEMORY;
  const auto status = validate_two(h, nh, b, nb);
  if (status != POKER_OK) return status;
  const Header head{magic, 1, static_cast<std::uint8_t>(nh), static_cast<std::uint8_t>(nb),
    static_cast<std::uint8_t>(fh), static_cast<std::uint8_t>(fb)};
  if (nh) std::memcpy(holes_of(state), h, nh);
  if (nb) std::memcpy(holes_of(state) + fh, b, nb);
  write_header(state, head);
  return POKER_OK;
}
poker_status poker_prepare_high(void* state, std::size_t capacity, const poker_card* known,
  std::size_t count, std::size_t final_count, std::size_t budget) {
  if (!storage_ok(state, capacity)) return POKER_INVALID_ARGUMENT;
  if (final_count < 5 || final_count > 52 || count > final_count) return POKER_INVALID_COUNT;
  if (budget < info.shared_bytes + info.state_bytes) return POKER_INSUFFICIENT_MEMORY;
  const auto status = validate_two(known, count, nullptr, 0);
  if (status != POKER_OK) return status;
  const Header head{magic, 0, static_cast<std::uint8_t>(count), 0,
    static_cast<std::uint8_t>(final_count), 0};
  if (count) std::memcpy(holes_of(state), known, count);
  write_header(state, head);
  return POKER_OK;
}
poker_status poker_extend(const void* state, const poker_card* h, std::size_t nh,
  const poker_card* b, std::size_t nb, void* out, std::size_t capacity) {
  Header head{};
  if (!read_header(state, head)) return POKER_INVALID_STATE;
  if (!storage_ok(out, capacity)) return POKER_INVALID_ARGUMENT;
  if (nh > static_cast<std::size_t>(head.final_h - head.nh) || nb > static_cast<std::size_t>(head.final_b - head.nb)) return POKER_INVALID_COUNT;
  const poker_card* known_holes = holes_of(state);
  const poker_card* known_board = board_of(state, head.final_h);
  std::uint64_t used = 0;
  (void)validate(known_holes, head.nh, used);
  (void)validate(known_board, head.nb, used);
  auto status = validate(h, nh, used);
  if (status != POKER_OK) return status;
  status = validate(b, nb, used);
  if (status != POKER_OK) return status;
  poker_card* out_holes = holes_of(out);
  poker_card* out_board = out_holes + head.final_h;
  if (out != state) {
    if (head.nh) std::memcpy(out_holes, known_holes, head.nh);
    if (head.nb) std::memcpy(out_board, known_board, head.nb);
  }
  if (nh) std::memcpy(out_holes + head.nh, h, nh);
  if (nb) std::memcpy(out_board + head.nb, b, nb);
  head.nh = static_cast<std::uint8_t>(head.nh + nh);
  head.nb = static_cast<std::uint8_t>(head.nb + nb);
  write_header(out, head);
  return POKER_OK;
}
poker_status poker_clone(const void* state, void* out, std::size_t capacity) {
  // A clone changes no header field, so it needs neither the extension checks
  // nor a header write: validate what is there and copy it.
  Header head{};
  if (!read_header(state, head)) return POKER_INVALID_STATE;
  if (!storage_ok(out, capacity)) return POKER_INVALID_ARGUMENT;
  std::uint64_t used = 0;
  (void)validate(holes_of(state), head.nh, used);
  (void)validate(board_of(state, head.final_h), head.nb, used);
  if (out != state) {
    std::memcpy(out, state, header_bytes);
    if (head.nh) std::memcpy(holes_of(out), holes_of(state), head.nh);
    if (head.nb) std::memcpy(board_of(out, head.final_h), board_of(state, head.final_h), head.nb);
  }
  return POKER_OK;
}
poker_rank poker_complete_unchecked(const void* state, const poker_card* h, const poker_card* b) {
  return completed(state, h, b);
}
poker_status poker_complete(const void* state, const poker_card* h, const poker_card* b, poker_rank* out) {
  if (!out) return POKER_INVALID_ARGUMENT;
  Header head{};
  if (!read_header(state, head)) return POKER_INVALID_STATE;
  std::uint64_t used = 0;
  (void)validate(holes_of(state), head.nh, used);
  (void)validate(board_of(state, head.final_h), head.nb, used);
  auto status = validate(h, static_cast<std::size_t>(head.final_h - head.nh), used);
  if (status != POKER_OK) return status;
  status = validate(b, static_cast<std::size_t>(head.final_b - head.nb), used);
  if (status != POKER_OK) return status;
  *out = completed(state, h, b);
  return POKER_OK;
}
void poker_complete_batch(const void* state, const poker_card* h, const poker_card* b, std::size_t count, poker_rank* out) {
  if (!count) return;
  Header head{};
  std::memcpy(&head, state, header_bytes);
  const auto missing_h = static_cast<std::size_t>(head.final_h - head.nh);
  const auto missing_b = static_cast<std::size_t>(head.final_b - head.nb);
  if (!head.omaha) {
    // The known cards are the same for every row: accumulate them once and add
    // the missing cards of each completion.
    const poker_card* known = holes_of(state);
    const auto base = poker::detail::accumulate(known, head.nh);
    const auto hand = static_cast<std::size_t>(head.final_h);
    for (std::size_t i = 0; i < count; ++i) {
      auto total = base;
      const poker_card* row = h + i * missing_h;
      for (std::size_t j = 0; j < missing_h; ++j) total.add(row[j]);
      poker_rank result = poker::detail::classify_sized(total, hand);
      if (poker::detail::any_five_in_a_suit(total.h))
        result = std::min(result, poker::detail::suited_best(total, known, head.nh, row, missing_h));
      out[i] = result;
    }
    return;
  }
  if (head.final_h > poker::detail::max_omaha_groups || head.final_b > poker::detail::max_omaha_groups) {
    for (std::size_t i = 0; i < count; ++i)
      out[i] = completed(state, missing_h ? h + i * missing_h : nullptr, missing_b ? b + i * missing_b : nullptr);
    return;
  }
  // Omaha completion: a group that is fully known is prepared once for every
  // row; a group a row completes is rebuilt from the known cards plus the row.
  const poker_card* known_holes = holes_of(state);
  const poker_card* known_board = board_of(state, head.final_h);
  const auto final_h = static_cast<std::size_t>(head.final_h);
  const auto final_b = static_cast<std::size_t>(head.final_b);
  poker_card hole_row[poker::detail::max_omaha_groups];
  poker_card board_row[poker::detail::max_omaha_groups];
  const poker_card* holes = known_holes;
  const poker_card* board = known_board;
  std::uint32_t pairs[poker::detail::max_omaha_pairs];
  unsigned pair_count = 0;
  std::uint32_t hole_lanes = 0;
  if (!missing_h) {
    pair_count = poker::detail::omaha_pair_offsets(known_holes, final_h, pairs);
    hole_lanes = poker::detail::suit_lanes(known_holes, final_h);
  }
  std::uint16_t triples[poker::detail::max_omaha_triples];
  unsigned triple_count = 0;
  std::uint32_t board_lanes = 0;
  if (!missing_b) {
    triple_count = poker::detail::omaha_triple_codes(known_board, final_b, triples);
    board_lanes = poker::detail::suit_lanes(known_board, final_b);
  }
  for (std::size_t i = 0; i < count; ++i) {
    if (missing_h) {
      if (head.nh) std::memcpy(hole_row, known_holes, head.nh);
      std::memcpy(hole_row + head.nh, h + i * missing_h, missing_h);
      holes = hole_row;
      pair_count = poker::detail::omaha_pair_offsets(holes, final_h, pairs);
      hole_lanes = poker::detail::suit_lanes(holes, final_h);
    }
    if (missing_b) {
      if (head.nb) std::memcpy(board_row, known_board, head.nb);
      std::memcpy(board_row + head.nb, b + i * missing_b, missing_b);
      board = board_row;
      triple_count = poker::detail::omaha_triple_codes(board, final_b, triples);
      board_lanes = poker::detail::suit_lanes(board, final_b);
    }
    out[i] = poker::detail::omaha_from_groups(pairs, pair_count, triples, triple_count, holes, final_h, board,
                                              final_b, hole_lanes, board_lanes);
  }
}
} // extern C
