#pragma once
#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>

namespace poker::detail {
using card = std::uint8_t;
using rank = std::uint16_t;
inline constexpr std::size_t rank_count = 7462;

// Rank lookup without search. A hand is classified from the masks of ranks seen
// once, twice, three times and four times, then turned into the exact Cactus Kev
// ordinal from the category base plus a lexicographic tie-break index. Every
// hand size shares the same classification, so the tables stay small.
namespace numbers {
using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;

constexpr u32 choose(int n, int k) noexcept {
  if (k < 0 || n < 0 || k > n) return 0;
  u32 r = 1;
  for (int i = 1; i <= k; ++i) r = r * static_cast<u32>(n - k + i) / static_cast<u32>(i);
  return r;
}

// Descending-lexicographic index of a k-subset (bitmask) of `size` ranks.
constexpr u32 lex(u32 mask, int size, int k) noexcept {
  u32 colex = 0;
  int i = k;
  for (int b = size - 1; b >= 0; --b)
    if (mask & (u32{1} << b)) {
      colex += choose(b, i);
      --i;
    }
  return choose(size, k) - 1 - colex;
}

struct Tables {
  // Flush and high card share one order: descending rank sets with the ten
  // straights removed, since a straight is never a flush or a high card.
  std::array<u16, 8192> five{};  // 5 ranks of 13, straights excluded
  std::array<u16, 4096> three{}; // 3 ranks of 12
  std::array<u8, 4096> two12{};  // 2 ranks of 12
  std::array<u8, 8192> two13{};  // 2 ranks of 13
};

constexpr u32 straight_mask(int top) noexcept {
  u32 mask = 0;
  for (int r = top - 1; r <= top + 3; ++r)
    if (r >= 0) mask |= u32{1} << r;
  return top == 0 ? mask | (u32{1} << 12) : mask;
}

constexpr Tables make_tables() noexcept {
  Tables t{};
  std::array<u32, 10> straights{};
  for (int top = 0; top < 10; ++top) straights[static_cast<std::size_t>(top)] = lex(straight_mask(top), 13, 5);
  for (u32 m = 0; m < 8192; ++m) {
    const int k = std::popcount(m);
    if (k == 5) {
      const u32 index = lex(m, 13, 5);
      u32 stronger = 0;
      for (const auto straight : straights)
        if (straight < index) ++stronger;
      t.five[m] = static_cast<u16>(index - stronger);
    } else if (k == 2) t.two13[m] = static_cast<u8>(lex(m, 13, 2));
  }
  for (u32 m = 0; m < 4096; ++m) {
    const int k = std::popcount(m);
    if (k == 3) t.three[m] = static_cast<u16>(lex(m, 12, 3));
    else if (k == 2) t.two12[m] = static_cast<u8>(lex(m, 12, 2));
  }
  return t;
}

inline constexpr Tables tables = make_tables();

inline constexpr unsigned index_of(u16 mask) noexcept {
  return 31 - static_cast<unsigned>(std::countl_zero(static_cast<u32>(mask)));
}
inline constexpr u16 top_bit(u16 mask) noexcept { return static_cast<u16>(u16{1} << index_of(mask)); }
// Remove rank r from a mask and shift every higher rank down: the mask then
// enumerates exactly the combinations of the remaining 12 ranks.
inline constexpr u16 drop_rank(u16 mask, unsigned r) noexcept {
  return static_cast<u16>((mask & ((u16{1} << r) - 1)) | ((mask >> (r + 1)) << r));
}
// Top card of the best straight in the mask, 0..9 (0 = five-high), or none.
inline constexpr u16 straight_top(u16 mask) noexcept {
  const u32 wide = (static_cast<u32>(mask) << 1) | ((mask >> 12) & 1u);
  const u32 run = wide & (wide >> 1) & (wide >> 2) & (wide >> 3) & (wide >> 4);
  return run ? static_cast<u16>(31 - static_cast<unsigned>(std::countl_zero(run))) : u16{0xffff};
}
inline constexpr u16 top_five(u16 mask) noexcept {
  u16 out = 0;
  for (int i = 0; i < 5; ++i) {
    const u16 bit = top_bit(mask);
    out |= bit;
    mask &= static_cast<u16>(~bit);
  }
  return out;
}

// Category bases are the counts of strictly stronger hands, so adding one gives
// the published ordinal. Bases: straight flush 0, quads 10, full house 166,
// flush 322, straight 1599, trips 1609, two pair 2467, pair 3325, high card 6185.
inline constexpr u16 straight_flush_rank(u16 top) noexcept { return static_cast<u16>(1 + (9 - top)); }
inline constexpr u16 quads_rank(u16 quad, u16 kicker) noexcept {
  const unsigned q = index_of(quad);
  const unsigned k = index_of(kicker);
  return static_cast<u16>(11 + (12 - q) * 12 + ((12 - k) - (q > k ? 1 : 0)));
}
inline constexpr u16 full_house_rank(u16 trips, u16 pair) noexcept {
  const unsigned t = index_of(trips);
  const unsigned p = index_of(pair);
  return static_cast<u16>(167 + (12 - t) * 12 + ((12 - p) - (t > p ? 1 : 0)));
}
inline constexpr u16 flush_rank(u16 mask) noexcept { return static_cast<u16>(323 + tables.five[mask]); }
inline constexpr u16 straight_rank(u16 top) noexcept { return static_cast<u16>(1600 + (9 - top)); }
inline constexpr u16 trips_rank(u16 trips, u16 kickers) noexcept {
  const unsigned t = index_of(trips);
  return static_cast<u16>(1610 + (12 - t) * 66 + tables.two12[drop_rank(kickers, t)]);
}
inline constexpr u16 two_pair_rank(u16 pairs, u16 kicker) noexcept {
  const unsigned k = index_of(kicker);
  const unsigned above = static_cast<unsigned>(std::popcount(static_cast<u32>(pairs >> (k + 1))));
  return static_cast<u16>(2468 + tables.two13[pairs] * 11 + ((12 - k) - above));
}
inline constexpr u16 pair_rank(u16 pair, u16 kickers) noexcept {
  const unsigned p = index_of(pair);
  return static_cast<u16>(3326 + (12 - p) * 220 + tables.three[drop_rank(kickers, p)]);
}
inline constexpr u16 high_rank(u16 mask) noexcept { return static_cast<u16>(6186 + tables.five[mask]); }
// Five cards of one suit: a straight flush when the ranks run, otherwise a flush.
inline constexpr u16 suited_rank(u16 mask) noexcept {
  const u16 top = straight_top(mask);
  return top != 0xffff ? straight_flush_rank(top) : flush_rank(top_five(mask));
}

// Best hand that is not a flush, from the masks alone. `seen` carries at most
// five ranks unless a rank repeats, which is what the high-card order expects.
inline constexpr u16 classified(u16 seen, u16 pairs, u16 trips, u16 quads) noexcept {
  if (quads) {
    // The kicker may use another rank that also appears four times.
    const u16 quad = top_bit(quads);
    return quads_rank(quad, top_bit(static_cast<u16>(seen & ~quad)));
  }
  if (trips) {
    const u16 set = top_bit(trips);
    const u16 rest = static_cast<u16>(pairs & ~set);
    if (rest) return full_house_rank(set, top_bit(rest));
  }
  const u16 straight = straight_top(seen);
  if (straight != 0xffff) return straight_rank(straight);
  if (trips) {
    const u16 set = top_bit(trips);
    const u16 rest = static_cast<u16>(seen & ~set);
    const u16 high = top_bit(rest);
    return trips_rank(set, static_cast<u16>(high | top_bit(static_cast<u16>(rest & ~high))));
  }
  const u16 exact_two = static_cast<u16>(pairs & ~trips);
  if (exact_two) {
    const u16 first = top_bit(exact_two);
    const u16 other = static_cast<u16>(exact_two & ~first);
    if (other) {
      // With three or more pairs the unchosen pairs stay eligible as the kicker.
      const u16 second = top_bit(other);
      const u16 chosen = static_cast<u16>(first | second);
      return two_pair_rank(chosen, top_bit(static_cast<u16>(seen & ~chosen)));
    }
    const u16 rest = static_cast<u16>(seen & ~first);
    const u16 k1 = top_bit(rest);
    const u16 k2 = top_bit(static_cast<u16>(rest & ~k1));
    return pair_rank(first, static_cast<u16>(k1 | k2 | top_bit(static_cast<u16>(rest & ~(k1 | k2)))));
  }
  return high_rank(seen);
}

// Omaha selects exactly two hole cards and three board cards. The resulting rank
// depends only on the two rank multisets, so every legal combination can be
// answered by one lookup instead of a five-card evaluation.
//
// Two cards: 91 rank multisets (13 pairs, then 78 distinct pairs, colex ordered).
// Three cards: 455 rank multisets (286 distinct, 156 pair plus single, 13 trips).
inline constexpr u32 pair_codes = 91;
inline constexpr u32 triple_codes = 455;
constexpr std::array<u32, 13> make_binom(int k) noexcept {
  std::array<u32, 13> out{};
  for (int i = 0; i < 13; ++i) out[static_cast<std::size_t>(i)] = choose(i, k);
  return out;
}
inline constexpr std::array<u32, 13> binom2 = make_binom(2);
inline constexpr std::array<u32, 13> binom3 = make_binom(3);
constexpr u32 pair_code(int a, int b) noexcept {  // a <= b
  return a == b ? static_cast<u32>(a) : 13u + binom2[static_cast<std::size_t>(b)] + static_cast<u32>(a);
}
constexpr u32 triple_code(int a, int b, int c) noexcept {  // a <= b <= c
  if (a == c) return 442u + static_cast<u32>(a);
  // A repeated rank is stored as the pair rank and the single rank counted
  // without it, so the code stays inside 286..441.
  if (a == b) return 286u + static_cast<u32>(a) * 12u + static_cast<u32>(c > a ? c - 1 : c);
  if (b == c) return 286u + static_cast<u32>(b) * 12u + static_cast<u32>(a);
  return binom3[static_cast<std::size_t>(c)] + binom2[static_cast<std::size_t>(b)] + static_cast<u32>(a);
}
// A deck holds four cards of a rank, so enumerating two hole ranks against three
// board ranks can produce five of a rank. Those entries are unreachable and stay
// zero.
constexpr u16 merged_rank(int a, int b, int c, int d, int e) noexcept {
  const int ranks[5] = {a, b, c, d, e};
  u16 seen = 0, pairs = 0, trips = 0, quads = 0;
  for (int i = 0; i < 5; ++i) {
    const auto bit = static_cast<u16>(u16{1} << ranks[i]);
    quads = static_cast<u16>(quads | (trips & bit));
    trips = static_cast<u16>(trips | (pairs & bit));
    pairs = static_cast<u16>(pairs | (seen & bit));
    seen = static_cast<u16>(seen | bit);
  }
  // Five of one rank leaves that rank as the only one seen.
  if (seen == quads) return 0;
  return classified(seen, pairs, trips, quads);
}

// Building the whole table in one constant expression exceeds clang's default
// evaluation step budget, so each slice of seven hole-pair codes is its own
// constant and the final table only copies them together.
inline constexpr u32 pairs_per_slice = 7;
constexpr std::array<u16, pairs_per_slice * triple_codes> make_omaha_slice(u32 first_pair) noexcept {
  std::array<u16, pairs_per_slice * triple_codes> slice{};
  for (u32 index = 0; index < pairs_per_slice; ++index) {
    const u32 code = first_pair + index;
    int low = 0, high = 0;
    if (code < 13) low = high = static_cast<int>(code);
    else {
      // Invert the colex index: find the higher rank of the pair.
      const u32 offset = code - 13;
      for (int r = 12; r > 0; --r)
        if (binom2[static_cast<std::size_t>(r)] <= offset) {
          high = r;
          break;
        }
      low = static_cast<int>(offset - binom2[static_cast<std::size_t>(high)]);
    }
    for (int c = 0; c < 13; ++c)
      for (int d = c; d < 13; ++d)
        for (int e = d; e < 13; ++e)
          slice[index * triple_codes + triple_code(c, d, e)] = merged_rank(low, high, c, d, e);
  }
  return slice;
}
inline constexpr auto omaha_slice00 = make_omaha_slice(0 * pairs_per_slice);
inline constexpr auto omaha_slice01 = make_omaha_slice(1 * pairs_per_slice);
inline constexpr auto omaha_slice02 = make_omaha_slice(2 * pairs_per_slice);
inline constexpr auto omaha_slice03 = make_omaha_slice(3 * pairs_per_slice);
inline constexpr auto omaha_slice04 = make_omaha_slice(4 * pairs_per_slice);
inline constexpr auto omaha_slice05 = make_omaha_slice(5 * pairs_per_slice);
inline constexpr auto omaha_slice06 = make_omaha_slice(6 * pairs_per_slice);
inline constexpr auto omaha_slice07 = make_omaha_slice(7 * pairs_per_slice);
inline constexpr auto omaha_slice08 = make_omaha_slice(8 * pairs_per_slice);
inline constexpr auto omaha_slice09 = make_omaha_slice(9 * pairs_per_slice);
inline constexpr auto omaha_slice10 = make_omaha_slice(10 * pairs_per_slice);
inline constexpr auto omaha_slice11 = make_omaha_slice(11 * pairs_per_slice);
inline constexpr auto omaha_slice12 = make_omaha_slice(12 * pairs_per_slice);

constexpr std::array<u16, pair_codes * triple_codes> assemble_omaha_table() noexcept {
  const std::array<const u16*, 13> slices{omaha_slice00.data(), omaha_slice01.data(), omaha_slice02.data(),
    omaha_slice03.data(), omaha_slice04.data(), omaha_slice05.data(), omaha_slice06.data(), omaha_slice07.data(),
    omaha_slice08.data(), omaha_slice09.data(), omaha_slice10.data(), omaha_slice11.data(), omaha_slice12.data()};
  std::array<u16, pair_codes * triple_codes> table{};
  for (u32 pair = 0; pair < pair_codes; ++pair)
    for (u32 triple = 0; triple < triple_codes; ++triple)
      table[pair * triple_codes + triple] =
        slices[pair / pairs_per_slice][(pair % pairs_per_slice) * triple_codes + triple];
  return table;
}

// Group preparation happens once per evaluation and dominated the Omaha profile:
// sorting the ranks and running the combinatorial formulas for every pair and
// triple costs more than the combination sweep itself. These tables are indexed
// by the ranks in any order and do both steps in one load. The pair table
// already carries the row offset, so a combination's entry is one add away.
constexpr std::array<u16, 13 * 13> make_pair_layout() noexcept {
  std::array<u16, 13 * 13> out{};
  for (int a = 0; a < 13; ++a)
    for (int b = 0; b < 13; ++b) {
      const int low = a < b ? a : b;
      const int high = a < b ? b : a;
      out[static_cast<std::size_t>(a * 13 + b)] =
        static_cast<u16>(pair_code(low, high) * triple_codes);
    }
  return out;
}
constexpr std::array<u16, 13 * 13 * 13> make_triple_codes() noexcept {
  std::array<u16, 13 * 13 * 13> out{};
  for (int a = 0; a < 13; ++a)
    for (int b = 0; b < 13; ++b)
      for (int c = 0; c < 13; ++c) {
        int ranks[3] = {a, b, c};
        for (int x = 0; x < 2; ++x)
          for (int y = x + 1; y < 3; ++y)
            if (ranks[y] < ranks[x]) {
              const int swap = ranks[x];
              ranks[x] = ranks[y];
              ranks[y] = swap;
            }
        out[static_cast<std::size_t>((a * 13 + b) * 13 + c)] =
          static_cast<u16>(triple_code(ranks[0], ranks[1], ranks[2]));
      }
  return out;
}
inline constexpr std::array<u16, 13 * 13> pair_layout = make_pair_layout();
inline constexpr std::array<u16, 13 * 13 * 13> triple_codes_by_rank = make_triple_codes();

// Ranks of two hole cards plus three board cards, suits excluded; a suited
// combination is a flush or straight flush and always outranks these values.
inline constexpr std::array<u16, pair_codes * triple_codes> omaha_table = assemble_omaha_table();
// Five-card hands know the size of every kicker set, so the dependent bit-scan
// chains the general classifier needs are unnecessary: the mask of the remaining
// ranks is already the kicker set.
inline constexpr u16 classified_five(u16 seen, u16 pairs, u16 trips, u16 quads) noexcept {
  if (quads) {
    const u16 quad = top_bit(quads);
    return quads_rank(quad, static_cast<u16>(seen & ~quad));
  }
  if (trips) {
    const u16 set = top_bit(trips);
    const u16 rest = static_cast<u16>(pairs & ~set);
    if (rest) return full_house_rank(set, top_bit(rest));
  }
  const u16 straight = straight_top(seen);
  if (straight != 0xffff) return straight_rank(straight);
  if (trips) {
    const u16 set = top_bit(trips);
    return trips_rank(set, static_cast<u16>(seen & ~set));
  }
  const u16 exact_two = static_cast<u16>(pairs & ~trips);
  if (exact_two) {
    const u16 first = top_bit(exact_two);
    const u16 other = static_cast<u16>(exact_two & ~first);
    if (other) return two_pair_rank(static_cast<u16>(first | other),
                                    static_cast<u16>(seen & ~(first | other)));
    return pair_rank(first, static_cast<u16>(seen & ~first));
  }
  return high_rank(seen);
}

} // namespace numbers

// Perfect-hash classification for a hand of a fixed size. The non-flush ordinal
// is a pure function of the rank multiset (seen, pairs, trips, quads) and only a
// few thousand such multisets are reachable from five, six or seven cards, so a
// two-level "hash, displace and compress" table answers the classification with
// two multiplies and two loads.
//
// Every table is built at compile time. Compiler step budgets apply per top-level
// constant evaluation, so the enumeration is cut into one named slice per
// contiguous run of the depth-first leaf order, and the histogram, sort, hash and
// displacement stages are separate constants. Raw arrays instead of std::array
// and a bitmap for slot occupancy keep each evaluation inside the default budgets
// of both GCC and Clang.
// Three bits of rank count per rank (39 bits), with one five-bit suit lane above
// them. A sum over cards then holds every rank count and every suit count at once,
// so one add per card replaces four dependent mask updates. Lanes sit at 39 + 5*suit
// and cannot reach a neighbour: five cards put at most five in a lane, the flush
// test below adds 27, and a six-bit lane holds up to 63.
inline constexpr std::uint64_t mask_bits = (std::uint64_t{1} << 39) - 1;
inline constexpr std::uint64_t spaced_ones = 0x1249249249249ull;
inline constexpr std::uint64_t lane_high =
    0x20ull * (1ull + (1ull << 6) + (1ull << 12) + (1ull << 18));
inline constexpr std::uint64_t lane_bias =
    27ull * (1ull + (1ull << 6) + (1ull << 12) + (1ull << 18));

constexpr std::array<std::uint64_t, 256> make_spaced_bits() noexcept {
  std::array<std::uint64_t, 256> out{};
  for (unsigned value = 0; value < 256; ++value) {
    const unsigned r = (value >> 2) & 15u;
    const unsigned suit = value & 3u;
    out[value] = (std::uint64_t{1} << (3u * r)) | (std::uint64_t{1} << (39u + 6u * suit));
  }
  return out;
}
inline constexpr std::array<std::uint64_t, 256> spaced_bits = make_spaced_bits();

// A suit holding five or more sets bit 4 of its lane when 27 is added to it, and no
// lane of a five-card hand can carry past that bit.
inline bool any_five_in_a_suit(std::uint64_t h) noexcept {
  return ((((h >> 39) + lane_bias) & lane_high) != 0);
}

// Contiguous rank mask from one plane of the sum: bit r of the result is bit 3r
// of the plane. Only the five-card flush path uses this loop form, for about two
// hands in a thousand, and it stays as it is: the compiler vectorises it, so it
// costs the surrounding batch loop nothing.
constexpr numbers::u16 compress_ranks(std::uint64_t spaced) noexcept {
  numbers::u16 out = 0;
  for (unsigned r = 0; r < 13; ++r)
    out = static_cast<numbers::u16>(out | (((spaced >> (3u * r)) & 1u) << r));
  return out;
}

// The same gather without a loop, for the arithmetic classifier above seven
// cards, which needs three of them per hand. Three shift-and-mask folds bring
// pairs, then fours, then eights of neighbouring ranks together, and the last
// step drops rank twelve into place: sixteen operations.
inline constexpr std::uint64_t rank_ones = 0x1249249249ull;  // bit 3r for r < 13
constexpr numbers::u16 fold_ranks(std::uint64_t plane) noexcept {
  std::uint64_t x = plane & rank_ones;
  x = (x | (x >> 2)) & 0x30C30C30C3ull;  // ranks 2k, 2k+1 at bits 6k, 6k+1
  x = (x | (x >> 4)) & 0x100F00F00Full;  // fours at bits 0, 12, 24; rank 12 at 36
  x = (x | (x >> 8)) & 0x100F00FFull;    // eight at 0, four at 16; rank 12 at 28
  return static_cast<numbers::u16>(((x | (x >> 8)) & 0xFFFull) | ((x >> 16) & 0x1000ull));
}
static_assert(fold_ranks(rank_ones) == 0x1FFF && compress_ranks(rank_ones) == 0x1FFF, "every rank");
static_assert(fold_ranks(std::uint64_t{1} << 36) == 0x1000, "rank twelve alone");
static_assert(fold_ranks((std::uint64_t{1} << 3) | (std::uint64_t{1} << 33)) == 0x0802,
              "ranks one and eleven");
static_assert(fold_ranks(0x2492492492ull) == 0, "the middle plane is not the low plane");
static_assert(fold_ranks(0x1FFF'FFFF'FFFF'FFFFull) == compress_ranks(0x1FFF'FFFF'FFFF'FFFFull),
              "suit lanes are ignored");

// The four contiguous rank masks from the sum. Only the arithmetic classifier,
// used above seven cards, needs them. A count of one to four is the low, the
// middle, both, or the high bit of its three-bit field, so the masks come from
// the three bit planes.
struct RankMasks {
  numbers::u16 seen, pairs, trips, quads;
};
inline RankMasks masks_of(std::uint64_t h) noexcept {
  const numbers::u16 low = fold_ranks(h);
  const numbers::u16 middle = fold_ranks(h >> 1);
  const numbers::u16 high = fold_ranks(h >> 2);
  return RankMasks{static_cast<numbers::u16>(low | middle | high), static_cast<numbers::u16>(middle | high),
                   static_cast<numbers::u16>((low & middle) | high), high};
}

// The rank multiset as the sum encodes it: the key table5 is built over.
constexpr std::uint64_t spaced_key(numbers::u16 seen, numbers::u16 pairs, numbers::u16 trips,
                                   numbers::u16 quads) noexcept {
  std::uint64_t h = 0;
  for (unsigned r = 0; r < 13; ++r)
    if ((seen >> r) & 1u) {
      const unsigned count = 1u + ((pairs >> r) & 1u) + ((trips >> r) & 1u) + ((quads >> r) & 1u);
      h += static_cast<std::uint64_t>(count) << (3u * r);
    }
  return h;
}

// Contiguous rank mask from the sum. Only the flush path needs one - about two hands
// in ten thousand - so a short loop is the right shape here.
inline numbers::u16 contiguous_ranks(std::uint64_t h) noexcept {
  return compress_ranks((h | (h >> 1) | (h >> 2)) & spaced_ones);
}

namespace chd {

using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

// Sizes per hand size: 6175, 18395 and 49205 rank multisets respectively. The
// slot arrays are the smallest powers of two that hold the keys (75%, 56% and
// 75% full) with three to five keys per bucket. Buckets are placed largest
// first, which is what lets a table this full still find a displacement for
// every bucket. The footprint is what matters: a batch of fresh hands touches
// the table at random, so the table has to stay in cache, and these tables are
// a quarter, half and half the size of the previous ones.
//
// Two keys of one bucket with the same second-level hash collide under every
// displacement, and the fuller tables make such a pair likely. The slot
// multipliers were chosen by an offline search over random odd constants for a
// collision-free placement with the fewest displacement attempts.
struct Spec5 {
  static constexpr u64 bucket_mul = 0x9E3779B97F4A7C15ull;
  static constexpr u64 slot_mul = 0x74BD4A2CEC960C83ull;
  static constexpr u32 cards = 5;
  static constexpr u32 buckets_log2 = 11;
  static constexpr u32 slots_log2 = 13;
  static constexpr u32 piece_keys = 1600;
  static constexpr u32 piece_count = 4;
};
struct Spec6 {
  static constexpr u64 bucket_mul = 0x9E3779B97F4A7C15ull;
  static constexpr u64 slot_mul = 0x30EC546BABBCDC4Dull;
  static constexpr u32 cards = 6;
  static constexpr u32 buckets_log2 = 12;
  static constexpr u32 slots_log2 = 15;
  static constexpr u32 piece_keys = 1600;
  static constexpr u32 piece_count = 12;
};
struct Spec7 {
  static constexpr u64 bucket_mul = 0x9E3779B97F4A7C15ull;
  static constexpr u64 slot_mul = 0x971284CA21118ACFull;
  static constexpr u32 cards = 7;
  static constexpr u32 buckets_log2 = 14;
  static constexpr u32 slots_log2 = 16;
  static constexpr u32 piece_keys = 1600;
  static constexpr u32 piece_count = 32;
};

template <class S>
constexpr u32 buckets_of() noexcept {
  return 1u << S::buckets_log2;
}
template <class S>
constexpr u32 slots_of() noexcept {
  return 1u << S::slots_log2;
}
template <class S>
constexpr u32 first_bucket(u64 key) noexcept {
  return static_cast<u32>((key * S::bucket_mul) >> (64 - S::buckets_log2));
}
template <class S>
constexpr u32 second_hash(u64 key) noexcept {
  return static_cast<u32>((key * S::slot_mul) >> (64 - S::slots_log2));
}

// leaves[r][n] = count vectors for ranks r..12 summing to n, each count at most
// four. Shared by every hand size: only the starting sum differs.
constexpr std::array<std::array<u32, 8>, 14> make_leaves() noexcept {
  std::array<std::array<u32, 8>, 14> table{};
  table[13][0] = 1;
  for (int r = 12; r >= 0; --r)
    for (int n = 0; n <= 7; ++n) {
      u32 sum = 0;
      for (int c = 0; c <= 4 && c <= n; ++c) sum += table[r + 1][n - c];
      table[r][n] = sum;
    }
  return table;
}
inline constexpr auto leaves = make_leaves();
static_assert(leaves[0][5] == 6175, "five-card rank multiset count");
static_assert(leaves[0][6] == 18395, "six-card rank multiset count");
static_assert(leaves[0][7] == 49205, "seven-card rank multiset count");

template <class S>
struct Piece {
  u64 key[S::piece_keys];
  u16 rank[S::piece_keys];
  u32 count;
};

// Enumerates only the leaves of the depth-first order in
// [first, first + piece_keys); the subtree sizes above let every other branch be
// skipped in one step.
template <class S>
constexpr Piece<S> make_piece(u32 first) noexcept {
  namespace numbers = poker::detail::numbers;
  Piece<S> out{};
  const u32 want_to = first + S::piece_keys;
  u32 index = 0;
  bool done = false;
  auto walk = [&](auto&& self, u32 rank_index, u32 remaining, u16 seen, u16 pairs, u16 trips,
                  u16 quads) constexpr -> void {
    if (done) return;
    const u32 below = leaves[rank_index][remaining];
    if (below == 0) return;  // no rank assignment below this node
    if (index >= want_to) {  // range filled: nothing below can be in it
      done = true;
      return;
    }
    if (index + below <= first) {  // subtree lies before the range
      index += below;
      return;
    }
    if (rank_index == 13) {
      const u32 at = index - first;
      out.key[at] = spaced_key(seen, pairs, trips, quads);
      if (S::cards == 5) {
        out.rank[at] = numbers::classified_five(seen, pairs, trips, quads);
      } else {
        // A straight is decided on the full mask before the high-card order,
        // which only covers five-bit masks.
        u16 reduced = seen;
        if (!pairs) {
          const u16 straight = numbers::straight_top(seen);
          if (straight != 0xffff) {
            out.rank[at] = numbers::straight_rank(straight);
            ++index;
            return;
          }
          reduced = numbers::top_five(seen);
        }
        out.rank[at] = numbers::classified(reduced, pairs, trips, quads);
      }
      ++index;
      return;
    }
    const u16 bit = static_cast<u16>(1u << rank_index);
    for (u32 c = 0; c <= 4 && c <= remaining; ++c)
      self(self, rank_index + 1, remaining - c, c >= 1 ? static_cast<u16>(seen | bit) : seen,
           c >= 2 ? static_cast<u16>(pairs | bit) : pairs,
           c >= 3 ? static_cast<u16>(trips | bit) : trips,
           c >= 4 ? static_cast<u16>(quads | bit) : quads);
  };
  walk(walk, 0, S::cards, 0, 0, 0, 0);
  out.count = index > first ? (index < want_to ? index : want_to) - first : 0u;
  return out;
}

template <class S>
struct Mapping {
  u32 start[buckets_of<S>() + 1];
};

template <class S>
using Slices = std::array<const Piece<S>*, S::piece_count>;

template <class S>
struct Sorted {
  u64 key[S::piece_count * S::piece_keys];
  u16 rank[S::piece_count * S::piece_keys];
};

template <class S>
struct Hashes {
  u32 h[S::piece_count * S::piece_keys];
};

// Displacements never reach the search bound of 8192, so sixteen bits hold them
// and the displacement array is half the size it would be as u32.
template <class S>
struct Table {
  bool ok = true;
  u16 value[slots_of<S>()];
  u16 displacement[buckets_of<S>()];
};

// Buckets in the order they are placed: largest first, ties by bucket index.
template <class S>
struct Order {
  u16 bucket[buckets_of<S>()];
};

// Bucket histogram, then prefix sums.
template <class S>
constexpr Mapping<S> map_of(const Slices<S>& pieces) noexcept {
  Mapping<S> map{};
  const u32 bucket_count = buckets_of<S>();
  for (u32 b = 0; b <= bucket_count; ++b) map.start[b] = 0;
  for (const Piece<S>* piece : pieces)
    for (u32 i = 0; i < piece->count; ++i) ++map.start[first_bucket<S>(piece->key[i]) + 1];
  for (u32 b = 0; b < bucket_count; ++b) map.start[b + 1] += map.start[b];
  return map;
}

// Counting sort, so each bucket owns a contiguous run of keys.
template <class S>
constexpr Sorted<S> sorted_of(const Slices<S>& pieces, const Mapping<S>& map) noexcept {
  Sorted<S> out{};
  const u32 bucket_count = buckets_of<S>();
  u32 cursor[buckets_of<S>() + 1];
  for (u32 b = 0; b <= bucket_count; ++b) cursor[b] = map.start[b];
  for (const Piece<S>* piece : pieces)
    for (u32 i = 0; i < piece->count; ++i) {
      const u32 at = cursor[first_bucket<S>(piece->key[i])]++;
      out.key[at] = piece->key[i];
      out.rank[at] = piece->rank[i];
    }
  return out;
}

// The second-level hash, hoisted out of the displacement search.
template <class S>
constexpr Hashes<S> hashes_of(const Sorted<S>& sorted, const Mapping<S>& map) noexcept {
  Hashes<S> out{};
  const u32 total = map.start[buckets_of<S>()];
  for (u32 i = 0; i < total; ++i) out.h[i] = second_hash<S>(sorted.key[i]);
  return out;
}

// Counting sort of the buckets by size, largest first. A big bucket placed into
// a nearly empty table needs few attempts, and the many one-key buckets that
// come last fit anywhere there is a hole, so the table can be far fuller than
// index order allows without the search running away.
template <class S>
constexpr Order<S> order_of(const Mapping<S>& map) noexcept {
  Order<S> out{};
  const u32 bucket_count = buckets_of<S>();
  // No bucket comes near this many keys; a larger one would only sort as if it
  // had exactly this many, and placement would still reject it if it did not fit.
  constexpr u32 size_limit = 63;
  u32 first_of[size_limit + 1]{};
  for (u32 b = 0; b < bucket_count; ++b) ++first_of[std::min(map.start[b + 1] - map.start[b], size_limit)];
  u32 position = 0;
  for (u32 size = size_limit + 1; size-- > 0;) {
    const u32 count = first_of[size];
    first_of[size] = position;
    position += count;
  }
  for (u32 b = 0; b < bucket_count; ++b)
    out.bucket[first_of[std::min(map.start[b + 1] - map.start[b], size_limit)]++] = static_cast<u16>(b);
  return out;
}

// One displacement per bucket, keeping the second level collision free. The
// range [first, last) is a run of positions in the placement order, and ranges
// must be processed in order with the same occupancy bitmap: later buckets may
// use only slots that earlier buckets left free.
template <class S>
constexpr void place_range(Table<S>& table, u64* used, const Sorted<S>& sorted, const Hashes<S>& hashes,
                           const Mapping<S>& map, const Order<S>& order, u32 first, u32 last) noexcept {
  const u32 slot_mask = slots_of<S>() - 1;
  for (u32 position = first; position < last; ++position) {
    const u32 b = order.bucket[position];
    const u32 begin = map.start[b], end = map.start[b + 1];
    if (begin == end) continue;
    bool placed = false;
    for (u32 d = 0; d < 8192; ++d) {
      // Each slot is claimed in the bitmap as it is reached, so a displacement
      // that maps two keys of this bucket onto one slot is rejected the same way
      // as one that reaches a slot another bucket already owns. A rejected
      // attempt releases the slots it claimed before moving on.
      u32 i = begin;
      for (; i < end; ++i) {
        const u32 slot = (hashes.h[i] + d) & slot_mask;
        const u64 bit = 1ull << (slot & 63);
        if (used[slot >> 6] & bit) break;
        used[slot >> 6] |= bit;
      }
      if (i < end) {
        for (u32 k = begin; k < i; ++k) {
          const u32 slot = (hashes.h[k] + d) & slot_mask;
          used[slot >> 6] &= ~(1ull << (slot & 63));
        }
        continue;
      }
      for (u32 k = begin; k < end; ++k) table.value[(hashes.h[k] + d) & slot_mask] = sorted.rank[k];
      table.displacement[b] = static_cast<u16>(d);
      placed = true;
      break;
    }
    if (!placed) table.ok = false;
  }
}

template <class S>
constexpr Table<S> place(const Sorted<S>& sorted, const Hashes<S>& hashes, const Mapping<S>& map,
                         const Order<S>& order) noexcept {
  Table<S> table{};
  u64 used[(slots_of<S>() + 63) / 64]{};
  place_range(table, used, sorted, hashes, map, order, 0, buckets_of<S>());
  return table;
}

// Carry compile-time search state across separate constant evaluations. Only
// the final Table is used by the evaluator; occupancy is construction scratch.
template <class S>
struct Placement {
  Table<S> table{};
  u64 used[(slots_of<S>() + 63) / 64]{};
};

template <class S>
constexpr Placement<S> place_next(Placement<S> state, const Sorted<S>& sorted, const Hashes<S>& hashes,
                                  const Mapping<S>& map, const Order<S>& order, u32 first,
                                  u32 last) noexcept {
  place_range(state.table, state.used, sorted, hashes, map, order, first, last);
  return state;
}

}  // namespace chd

// The same table, addressed by the sum form of the rank multiset.
template <class S>
inline rank spaced_lookup(const chd::Table<S>& table, std::uint64_t key) noexcept {
  return table.value[(chd::second_hash<S>(key) + table.displacement[chd::first_bucket<S>(key)]) &
                     (chd::slots_of<S>() - 1)];
}

// Same answer as classify() or classified_five() for a hand of the size the
// table was built for.
template <class S>
inline rank chd_lookup(const chd::Table<S>& table, numbers::u16 seen, numbers::u16 pairs,
                       numbers::u16 trips, numbers::u16 quads) noexcept {
  const std::uint64_t key = static_cast<std::uint64_t>(seen) |
                            (static_cast<std::uint64_t>(pairs) << 13) |
                            (static_cast<std::uint64_t>(trips) << 26) |
                            (static_cast<std::uint64_t>(quads) << 39);
  return table.value[(chd::second_hash<S>(key) + table.displacement[chd::first_bucket<S>(key)]) &
                     (chd::slots_of<S>() - 1)];
}

// Each slice is its own constant evaluation, which is what keeps the build inside
// a compiler's per-evaluation budget.
inline constexpr chd::Piece<chd::Spec5> piece5_00 = chd::make_piece<chd::Spec5>(0 * chd::Spec5::piece_keys);
inline constexpr chd::Piece<chd::Spec5> piece5_01 = chd::make_piece<chd::Spec5>(1 * chd::Spec5::piece_keys);
inline constexpr chd::Piece<chd::Spec5> piece5_02 = chd::make_piece<chd::Spec5>(2 * chd::Spec5::piece_keys);
inline constexpr chd::Piece<chd::Spec5> piece5_03 = chd::make_piece<chd::Spec5>(3 * chd::Spec5::piece_keys);
inline constexpr chd::Slices<chd::Spec5> pieces5{
    &piece5_00,
    &piece5_01,
    &piece5_02,
    &piece5_03
};
inline constexpr chd::Mapping<chd::Spec5> map5 = chd::map_of<chd::Spec5>(pieces5);
inline constexpr chd::Sorted<chd::Spec5> sorted5 = chd::sorted_of<chd::Spec5>(pieces5, map5);
inline constexpr chd::Hashes<chd::Spec5> hashes5 = chd::hashes_of<chd::Spec5>(sorted5, map5);
inline constexpr chd::Order<chd::Spec5> order5 = chd::order_of<chd::Spec5>(map5);
inline constexpr chd::Table<chd::Spec5> table5 = chd::place<chd::Spec5>(sorted5, hashes5, map5, order5);
static_assert(table5.ok, "perfect hash placement failed for the 5-card table");

inline constexpr chd::Piece<chd::Spec6> piece6_00 = chd::make_piece<chd::Spec6>(0 * chd::Spec6::piece_keys);
inline constexpr chd::Piece<chd::Spec6> piece6_01 = chd::make_piece<chd::Spec6>(1 * chd::Spec6::piece_keys);
inline constexpr chd::Piece<chd::Spec6> piece6_02 = chd::make_piece<chd::Spec6>(2 * chd::Spec6::piece_keys);
inline constexpr chd::Piece<chd::Spec6> piece6_03 = chd::make_piece<chd::Spec6>(3 * chd::Spec6::piece_keys);
inline constexpr chd::Piece<chd::Spec6> piece6_04 = chd::make_piece<chd::Spec6>(4 * chd::Spec6::piece_keys);
inline constexpr chd::Piece<chd::Spec6> piece6_05 = chd::make_piece<chd::Spec6>(5 * chd::Spec6::piece_keys);
inline constexpr chd::Piece<chd::Spec6> piece6_06 = chd::make_piece<chd::Spec6>(6 * chd::Spec6::piece_keys);
inline constexpr chd::Piece<chd::Spec6> piece6_07 = chd::make_piece<chd::Spec6>(7 * chd::Spec6::piece_keys);
inline constexpr chd::Piece<chd::Spec6> piece6_08 = chd::make_piece<chd::Spec6>(8 * chd::Spec6::piece_keys);
inline constexpr chd::Piece<chd::Spec6> piece6_09 = chd::make_piece<chd::Spec6>(9 * chd::Spec6::piece_keys);
inline constexpr chd::Piece<chd::Spec6> piece6_10 = chd::make_piece<chd::Spec6>(10 * chd::Spec6::piece_keys);
inline constexpr chd::Piece<chd::Spec6> piece6_11 = chd::make_piece<chd::Spec6>(11 * chd::Spec6::piece_keys);
inline constexpr chd::Slices<chd::Spec6> pieces6{
    &piece6_00,
    &piece6_01,
    &piece6_02,
    &piece6_03,
    &piece6_04,
    &piece6_05,
    &piece6_06,
    &piece6_07,
    &piece6_08,
    &piece6_09,
    &piece6_10,
    &piece6_11
};
inline constexpr chd::Mapping<chd::Spec6> map6 = chd::map_of<chd::Spec6>(pieces6);
inline constexpr chd::Sorted<chd::Spec6> sorted6 = chd::sorted_of<chd::Spec6>(pieces6, map6);
inline constexpr chd::Hashes<chd::Spec6> hashes6 = chd::hashes_of<chd::Spec6>(sorted6, map6);
inline constexpr chd::Order<chd::Spec6> order6 = chd::order_of<chd::Spec6>(map6);
// The fuller six-card table takes more attempts per bucket than one constant
// evaluation may spend, so its placement is split into stages like the
// seven-card one below.
inline constexpr auto placement6_0 = chd::place_next<chd::Spec6>({}, sorted6, hashes6, map6, order6, 0, 2048);
inline constexpr auto placement6_1 =
    chd::place_next(placement6_0, sorted6, hashes6, map6, order6, 2048, chd::buckets_of<chd::Spec6>());
inline constexpr chd::Table<chd::Spec6> table6 = placement6_1.table;
static_assert(table6.ok, "perfect hash placement failed for the 6-card table");

inline constexpr chd::Piece<chd::Spec7> piece7_00 = chd::make_piece<chd::Spec7>(0 * chd::Spec7::piece_keys);
inline constexpr chd::Piece<chd::Spec7> piece7_01 = chd::make_piece<chd::Spec7>(1 * chd::Spec7::piece_keys);
inline constexpr chd::Piece<chd::Spec7> piece7_02 = chd::make_piece<chd::Spec7>(2 * chd::Spec7::piece_keys);
inline constexpr chd::Piece<chd::Spec7> piece7_03 = chd::make_piece<chd::Spec7>(3 * chd::Spec7::piece_keys);
inline constexpr chd::Piece<chd::Spec7> piece7_04 = chd::make_piece<chd::Spec7>(4 * chd::Spec7::piece_keys);
inline constexpr chd::Piece<chd::Spec7> piece7_05 = chd::make_piece<chd::Spec7>(5 * chd::Spec7::piece_keys);
inline constexpr chd::Piece<chd::Spec7> piece7_06 = chd::make_piece<chd::Spec7>(6 * chd::Spec7::piece_keys);
inline constexpr chd::Piece<chd::Spec7> piece7_07 = chd::make_piece<chd::Spec7>(7 * chd::Spec7::piece_keys);
inline constexpr chd::Piece<chd::Spec7> piece7_08 = chd::make_piece<chd::Spec7>(8 * chd::Spec7::piece_keys);
inline constexpr chd::Piece<chd::Spec7> piece7_09 = chd::make_piece<chd::Spec7>(9 * chd::Spec7::piece_keys);
inline constexpr chd::Piece<chd::Spec7> piece7_10 = chd::make_piece<chd::Spec7>(10 * chd::Spec7::piece_keys);
inline constexpr chd::Piece<chd::Spec7> piece7_11 = chd::make_piece<chd::Spec7>(11 * chd::Spec7::piece_keys);
inline constexpr chd::Piece<chd::Spec7> piece7_12 = chd::make_piece<chd::Spec7>(12 * chd::Spec7::piece_keys);
inline constexpr chd::Piece<chd::Spec7> piece7_13 = chd::make_piece<chd::Spec7>(13 * chd::Spec7::piece_keys);
inline constexpr chd::Piece<chd::Spec7> piece7_14 = chd::make_piece<chd::Spec7>(14 * chd::Spec7::piece_keys);
inline constexpr chd::Piece<chd::Spec7> piece7_15 = chd::make_piece<chd::Spec7>(15 * chd::Spec7::piece_keys);
inline constexpr chd::Piece<chd::Spec7> piece7_16 = chd::make_piece<chd::Spec7>(16 * chd::Spec7::piece_keys);
inline constexpr chd::Piece<chd::Spec7> piece7_17 = chd::make_piece<chd::Spec7>(17 * chd::Spec7::piece_keys);
inline constexpr chd::Piece<chd::Spec7> piece7_18 = chd::make_piece<chd::Spec7>(18 * chd::Spec7::piece_keys);
inline constexpr chd::Piece<chd::Spec7> piece7_19 = chd::make_piece<chd::Spec7>(19 * chd::Spec7::piece_keys);
inline constexpr chd::Piece<chd::Spec7> piece7_20 = chd::make_piece<chd::Spec7>(20 * chd::Spec7::piece_keys);
inline constexpr chd::Piece<chd::Spec7> piece7_21 = chd::make_piece<chd::Spec7>(21 * chd::Spec7::piece_keys);
inline constexpr chd::Piece<chd::Spec7> piece7_22 = chd::make_piece<chd::Spec7>(22 * chd::Spec7::piece_keys);
inline constexpr chd::Piece<chd::Spec7> piece7_23 = chd::make_piece<chd::Spec7>(23 * chd::Spec7::piece_keys);
inline constexpr chd::Piece<chd::Spec7> piece7_24 = chd::make_piece<chd::Spec7>(24 * chd::Spec7::piece_keys);
inline constexpr chd::Piece<chd::Spec7> piece7_25 = chd::make_piece<chd::Spec7>(25 * chd::Spec7::piece_keys);
inline constexpr chd::Piece<chd::Spec7> piece7_26 = chd::make_piece<chd::Spec7>(26 * chd::Spec7::piece_keys);
inline constexpr chd::Piece<chd::Spec7> piece7_27 = chd::make_piece<chd::Spec7>(27 * chd::Spec7::piece_keys);
inline constexpr chd::Piece<chd::Spec7> piece7_28 = chd::make_piece<chd::Spec7>(28 * chd::Spec7::piece_keys);
inline constexpr chd::Piece<chd::Spec7> piece7_29 = chd::make_piece<chd::Spec7>(29 * chd::Spec7::piece_keys);
inline constexpr chd::Piece<chd::Spec7> piece7_30 = chd::make_piece<chd::Spec7>(30 * chd::Spec7::piece_keys);
inline constexpr chd::Piece<chd::Spec7> piece7_31 = chd::make_piece<chd::Spec7>(31 * chd::Spec7::piece_keys);
inline constexpr chd::Slices<chd::Spec7> pieces7{
    &piece7_00,
    &piece7_01,
    &piece7_02,
    &piece7_03,
    &piece7_04,
    &piece7_05,
    &piece7_06,
    &piece7_07,
    &piece7_08,
    &piece7_09,
    &piece7_10,
    &piece7_11,
    &piece7_12,
    &piece7_13,
    &piece7_14,
    &piece7_15,
    &piece7_16,
    &piece7_17,
    &piece7_18,
    &piece7_19,
    &piece7_20,
    &piece7_21,
    &piece7_22,
    &piece7_23,
    &piece7_24,
    &piece7_25,
    &piece7_26,
    &piece7_27,
    &piece7_28,
    &piece7_29,
    &piece7_30,
    &piece7_31
};
inline constexpr chd::Mapping<chd::Spec7> map7 = chd::map_of<chd::Spec7>(pieces7);
inline constexpr chd::Sorted<chd::Spec7> sorted7 = chd::sorted_of<chd::Spec7>(pieces7, map7);
inline constexpr chd::Hashes<chd::Spec7> hashes7 = chd::hashes_of<chd::Spec7>(sorted7, map7);
inline constexpr chd::Order<chd::Spec7> order7 = chd::order_of<chd::Spec7>(map7);
// The complete seven-card placement exceeds the default constexpr step budget
// of MSVC and Clang. Each named stage continues the same search for 1/16 of
// the placement order, resetting the compiler's evaluation budget without
// changing the table.
inline constexpr auto placement7_00 =
    chd::place_next<chd::Spec7>({}, sorted7, hashes7, map7, order7, 0, 1024);
inline constexpr auto placement7_01 =
    chd::place_next(placement7_00, sorted7, hashes7, map7, order7, 1024, 2048);
inline constexpr auto placement7_02 =
    chd::place_next(placement7_01, sorted7, hashes7, map7, order7, 2048, 3072);
inline constexpr auto placement7_03 =
    chd::place_next(placement7_02, sorted7, hashes7, map7, order7, 3072, 4096);
inline constexpr auto placement7_04 =
    chd::place_next(placement7_03, sorted7, hashes7, map7, order7, 4096, 5120);
inline constexpr auto placement7_05 =
    chd::place_next(placement7_04, sorted7, hashes7, map7, order7, 5120, 6144);
inline constexpr auto placement7_06 =
    chd::place_next(placement7_05, sorted7, hashes7, map7, order7, 6144, 7168);
inline constexpr auto placement7_07 =
    chd::place_next(placement7_06, sorted7, hashes7, map7, order7, 7168, 8192);
inline constexpr auto placement7_08 =
    chd::place_next(placement7_07, sorted7, hashes7, map7, order7, 8192, 9216);
inline constexpr auto placement7_09 =
    chd::place_next(placement7_08, sorted7, hashes7, map7, order7, 9216, 10240);
inline constexpr auto placement7_10 =
    chd::place_next(placement7_09, sorted7, hashes7, map7, order7, 10240, 11264);
inline constexpr auto placement7_11 =
    chd::place_next(placement7_10, sorted7, hashes7, map7, order7, 11264, 12288);
inline constexpr auto placement7_12 =
    chd::place_next(placement7_11, sorted7, hashes7, map7, order7, 12288, 13312);
inline constexpr auto placement7_13 =
    chd::place_next(placement7_12, sorted7, hashes7, map7, order7, 13312, 14336);
inline constexpr auto placement7_14 =
    chd::place_next(placement7_13, sorted7, hashes7, map7, order7, 14336, 15360);
inline constexpr auto placement7_15 =
    chd::place_next(placement7_14, sorted7, hashes7, map7, order7, 15360, chd::buckets_of<chd::Spec7>());
inline constexpr chd::Table<chd::Spec7> table7 = placement7_15.table;
static_assert(table7.ok, "perfect hash placement failed for the 7-card table");


// Bytes of immutable tables shared by every evaluator instance.
// One load per card replaces a shift and an eight-times-multiply. Both the rank
// bit and the suit lane depend only on the card, and one 64-bit entry carries
// both: low half the rank bit, high half the suit lane. As two separate fields
// the compiler rematerialised the load three or four times per card; packed, a
// single load feeds both uses. Indexing by the whole card byte, 256 entries,
// removes the index mask as well and keeps every unchecked input inside the
// table.

inline constexpr std::size_t shared_bytes =
    sizeof(numbers::tables) + sizeof(numbers::omaha_table) + sizeof(spaced_bits) + sizeof(table5) +
    sizeof(table6) + sizeof(table7) + sizeof(numbers::pair_layout) + sizeof(numbers::triple_codes_by_rank);
inline void initialize() noexcept {
  (void)numbers::tables;
  (void)numbers::omaha_table;
  (void)table5;
  (void)table6;
  (void)table7;
}

inline rank five(const card* cards) noexcept {
  std::uint64_t h = 0;
  for (int i = 0; i < 5; ++i) h += spaced_bits[static_cast<unsigned char>(cards[i])];
  // Five cards of one suit means one lane holds all five.
  if (any_five_in_a_suit(h)) return numbers::suited_rank(contiguous_ranks(h));
  return spaced_lookup(table5, h & mask_bits);
}

// Rank masks and per-suit card counts for a group of cards. A group that many
// hands share (a board, or the known cards of a prepared state) is accumulated
// once and reused.
struct Accum {
  std::uint64_t h = 0;

  void add(card value) noexcept { h += spaced_bits[static_cast<unsigned char>(value)]; }
};

inline Accum accumulate(const card* cards, std::size_t count) noexcept {
  Accum total;
  for (std::size_t i = 0; i < count; ++i) total.add(cards[i]);
  return total;
}


// Best hand that is not a flush. The straight is checked before the high-card
// order, whose table only covers five-bit masks, so a hand with six or more
// distinct ranks is reduced only after the straight has been ruled out.
inline rank classify(Accum total) noexcept {
  const RankMasks m = masks_of(total.h);
  if (!m.pairs) {
    const numbers::u16 top = numbers::straight_top(m.seen);
    if (top != 0xffff) return numbers::straight_rank(top);
    return numbers::classified(numbers::top_five(m.seen), m.pairs, m.trips, m.quads);
  }
  return numbers::classified(m.seen, m.pairs, m.trips, m.quads);
}

inline rank classify_five(const Accum& total) noexcept {
  return spaced_lookup(table5, total.h & mask_bits);
}
inline rank classify_six(const Accum& total) noexcept {
  return spaced_lookup(table6, total.h & mask_bits);
}
inline rank classify_seven(const Accum& total) noexcept {
  return spaced_lookup(table7, total.h & mask_bits);
}
// Non-flush ordinal for a hand of a known size: five, six and seven cards come
// from a table, every other size from the arithmetic classifier.
inline rank classify_sized(const Accum& total, std::size_t count) noexcept {
  switch (count) {
    case 5: return classify_five(total);
    case 6: return classify_six(total);
    case 7: return classify_seven(total);
    default: return classify(total);
  }
}


// Best flush or straight flush among the suits holding five or more cards. The
// lanes hold one count per suit; the scan below runs for about 3% of random hands.
#if defined(_MSC_VER)
#define POKER_NOINLINE __declspec(noinline)
#else
#define POKER_NOINLINE __attribute__((noinline))
#endif

// Deliberately out of line. The flush filter is a long branchy block that only
// about a seventh of seven-card hands and a fifth of six-card hands reach, and
// inlining it enlarges the hot loop for every hand rather than the minority that
// need it. Measured on batch/high: six-card -9.8%, seven-card -1.9%, five-card
// unchanged (it never calls this).
POKER_NOINLINE inline rank suited_best(const Accum& total, const card* first, std::size_t first_count,
                        const card* second, std::size_t second_count) noexcept {
  rank best = 7462;
  const numbers::u32 flushable = static_cast<numbers::u32>(((total.h >> 39) + lane_bias) & lane_high);
  for (unsigned suit = 0; flushable && suit < 4; ++suit) {
    if (!((flushable >> (6 * suit)) & 0x20u)) continue;
    numbers::u16 mask = 0;
    for (std::size_t i = 0; i < first_count; ++i)
      if ((first[i] & 3u) == suit) mask = static_cast<numbers::u16>(mask | (numbers::u16{1} << (first[i] >> 2)));
    for (std::size_t i = 0; i < second_count; ++i)
      if ((second[i] & 3u) == suit) mask = static_cast<numbers::u16>(mask | (numbers::u16{1} << (second[i] >> 2)));
    const rank candidate = numbers::suited_rank(mask);
    if (candidate < best) best = candidate;
  }
  return best;
}

// The out-of-line scan is only entered when some suit holds five or more cards.
// The test is one add and one mask on the sum that is already in a register, so
// the common hand skips the call, its argument spills and the scan's own retest.
inline rank finish(const Accum& total, const card* cards, std::size_t count) noexcept {
  const rank plain = classify(total);
  if (!any_five_in_a_suit(total.h)) return plain;
  return std::min(plain, suited_best(total, cards, count, nullptr, 0));
}

// Exactly seven cards: the table answers the non-flush part.
inline rank finish_seven(const Accum& total, const card* first, std::size_t first_count,
                         const card* second, std::size_t second_count) noexcept {
  const rank plain = classify_seven(total);
  if (!any_five_in_a_suit(total.h)) return plain;
  return std::min(plain, suited_best(total, first, first_count, second, second_count));
}
inline rank finish_six(const Accum& total, const card* first, std::size_t first_count,
                       const card* second, std::size_t second_count) noexcept {
  const rank plain = classify_six(total);
  if (!any_five_in_a_suit(total.h)) return plain;
  return std::min(plain, suited_best(total, first, first_count, second, second_count));
}

// Every hand size is classified from one pass over its cards. Exactly five cards
// take the specialized path, which knows every kicker set in full.
inline rank high(const card* cards, std::size_t count) noexcept {
  if (count == 5) return five(cards);
  const Accum total = accumulate(cards, count);
  if (count == 6) return finish_six(total, cards, 6, nullptr, 0);
  if (count == 7) return finish_seven(total, cards, 7, nullptr, 0);
  return finish(total, cards, count);
}

// Omaha: exactly two hole cards and three board cards. A combination that is
// not all one suit has the rank its two rank multisets give, so the unsuited
// answer is one table load per combination: a hole pair's row offset plus a
// board triple's code. Flushes are handled apart, and only for a suit that can
// make one: at least two hole cards and three board cards of that suit.

// Groups beyond this size use the subset path, which needs no prepared lists.
inline constexpr std::size_t max_omaha_groups = 8;
inline constexpr unsigned max_omaha_pairs = 28;    // C(8, 2)
inline constexpr unsigned max_omaha_triples = 56;  // C(8, 3)

// Row offsets of every two-card rank multiset among the holes. A fixed size lets
// the compiler unroll the triangular loop, which a runtime bound does not get.
template <std::size_t N>
inline unsigned omaha_pair_offsets_fixed(const card* holes, numbers::u32* out) noexcept {
  unsigned ranks[N];
  for (std::size_t i = 0; i < N; ++i) ranks[i] = holes[i] >> 2;
  unsigned count = 0;
  for (std::size_t i = 0; i < N; ++i)
    for (std::size_t j = i + 1; j < N; ++j) out[count++] = numbers::pair_layout[ranks[i] * 13u + ranks[j]];
  return count;
}
inline unsigned omaha_pair_offsets(const card* holes, std::size_t nh, numbers::u32* out) noexcept {
  switch (nh) {
    case 4: return omaha_pair_offsets_fixed<4>(holes, out);
    case 5: return omaha_pair_offsets_fixed<5>(holes, out);
    case 6: return omaha_pair_offsets_fixed<6>(holes, out);
    default: break;
  }
  unsigned count = 0;
  for (std::size_t i = 0; i < nh; ++i)
    for (std::size_t j = i + 1; j < nh; ++j)
      out[count++] = numbers::pair_layout[(holes[i] >> 2) * 13u + (holes[j] >> 2)];
  return count;
}

// Rank-multiset codes of every three-card group of the board. Each pair of
// board ranks is keyed once and every third rank is one add from it.
template <std::size_t N>
inline unsigned omaha_triple_codes_fixed(const card* board, numbers::u16* out) noexcept {
  unsigned ranks[N];
  for (std::size_t i = 0; i < N; ++i) ranks[i] = board[i] >> 2;
  unsigned count = 0;
  for (std::size_t i = 0; i < N; ++i)
    for (std::size_t j = i + 1; j < N; ++j) {
      const unsigned base = 13u * (ranks[i] * 13u + ranks[j]);
      for (std::size_t k = j + 1; k < N; ++k) out[count++] = numbers::triple_codes_by_rank[base + ranks[k]];
    }
  return count;
}
inline unsigned omaha_triple_codes(const card* board, std::size_t nb, numbers::u16* out) noexcept {
  switch (nb) {
    case 3: return omaha_triple_codes_fixed<3>(board, out);
    case 4: return omaha_triple_codes_fixed<4>(board, out);
    case 5: return omaha_triple_codes_fixed<5>(board, out);
    default: break;
  }
  unsigned count = 0;
  for (std::size_t i = 0; i < nb; ++i)
    for (std::size_t j = i + 1; j < nb; ++j) {
      const unsigned base = 13u * ((board[i] >> 2) * 13u + (board[j] >> 2));
      for (std::size_t k = j + 1; k < nb; ++k)
        out[count++] = numbers::triple_codes_by_rank[base + (board[k] >> 2)];
    }
  return count;
}

// Cards of each suit, one byte lane per suit. At most eight cards go in, so no
// lane can carry into the next.
inline numbers::u32 suit_lanes(const card* cards, std::size_t count) noexcept {
  numbers::u32 lanes = 0;
  for (std::size_t i = 0; i < count; ++i) lanes += numbers::u32{1} << (8u * (cards[i] & 3u));
  return lanes;
}
// Suits holding at least two hole cards and three board cards, as bit 5 of the
// suit's lane. Adding 30 to a hole lane sets bit 5 exactly when it holds two or
// more; adding 29 to a board lane, when it holds three or more. Lanes hold at
// most eight, so neither sum reaches the next lane.
inline numbers::u32 flushable_suits(numbers::u32 hole_lanes, numbers::u32 board_lanes) noexcept {
  return (hole_lanes + 0x1E1E1E1Eu) & (board_lanes + 0x1D1D1D1Du) & 0x20202020u;
}

// Minimum unsuited rank over every pair and triple. With a compile-time triple
// count the inner loop unrolls and the codes stay in registers; four
// accumulators keep the minimum from being one dependency chain through every
// load.
template <unsigned Triples>
inline rank sweep_fixed(const numbers::u32* pairs, unsigned pair_count,
                        const numbers::u16* triples) noexcept {
  numbers::u32 offsets[Triples];
  for (unsigned t = 0; t < Triples; ++t) offsets[t] = triples[t];
  rank acc[4] = {7462, 7462, 7462, 7462};
  for (unsigned p = 0; p < pair_count; ++p) {
    const numbers::u32 base = pairs[p];
    for (unsigned t = 0; t < Triples; ++t)
      acc[t & 3u] = std::min(acc[t & 3u], numbers::omaha_table[base + offsets[t]]);
  }
  return std::min(std::min(acc[0], acc[1]), std::min(acc[2], acc[3]));
}
inline rank sweep_any(const numbers::u32* pairs, unsigned pair_count, const numbers::u16* triples,
                      unsigned triple_count) noexcept {
  rank acc[2] = {7462, 7462};
  for (unsigned p = 0; p < pair_count; ++p) {
    const numbers::u32 base = pairs[p];
    unsigned t = 0;
    for (; t + 1 < triple_count; t += 2) {
      acc[0] = std::min(acc[0], numbers::omaha_table[base + triples[t]]);
      acc[1] = std::min(acc[1], numbers::omaha_table[base + triples[t + 1]]);
    }
    if (t < triple_count) acc[0] = std::min(acc[0], numbers::omaha_table[base + triples[t]]);
  }
  return std::min(acc[0], acc[1]);
}
inline rank omaha_sweep(const numbers::u32* pairs, unsigned pair_count, const numbers::u16* triples,
                        unsigned triple_count) noexcept {
  switch (triple_count) {
    case 10: return sweep_fixed<10>(pairs, pair_count, triples);
    case 4: return sweep_fixed<4>(pairs, pair_count, triples);
    case 1: return sweep_fixed<1>(pairs, pair_count, triples);
    default: return sweep_any(pairs, pair_count, triples, triple_count);
  }
}

// Five distinct ranks of one suit: a straight flush when they run, else a flush.
inline rank suited_five_rank(numbers::u16 mask) noexcept {
  const numbers::u16 top = numbers::straight_top(mask);
  return top != 0xffff ? numbers::straight_flush_rank(top) : numbers::flush_rank(mask);
}

// Best flush or straight flush from two hole cards and three board cards of one
// suit, over the suits `flushable` marks. Two hole cards and three board cards
// of one suit are five distinct ranks, so the flush table takes their mask as
// it is. The usual case, exactly two and three, is one lookup; more cards of
// the suit enumerate the subsets. Out of line: about one hand in fourteen gets
// here, and the loops would otherwise sit inside every batch loop.
POKER_NOINLINE inline rank omaha_flush_best(const card* holes, std::size_t nh, const card* board,
                                            std::size_t nb, numbers::u32 flushable) noexcept {
  rank best = 7462;
  for (unsigned suit = 0; suit < 4; ++suit) {
    if (!((flushable >> (8u * suit + 5u)) & 1u)) continue;
    // Ranks of the suit's cards, without a branch per card. Sixty-four bits keep
    // the shift defined for any card byte.
    std::uint64_t hole_mask = 0, board_mask = 0;
    for (std::size_t i = 0; i < nh; ++i)
      hole_mask |= static_cast<std::uint64_t>((holes[i] & 3u) == suit) << (holes[i] >> 2);
    for (std::size_t i = 0; i < nb; ++i)
      board_mask |= static_cast<std::uint64_t>((board[i] & 3u) == suit) << (board[i] >> 2);
    if (std::popcount(hole_mask) == 2 && std::popcount(board_mask) == 3) {
      best = std::min(best, suited_five_rank(static_cast<numbers::u16>(hole_mask | board_mask)));
      continue;
    }
    numbers::u16 hole_bits[max_omaha_groups];
    unsigned hole_count = 0;
    for (std::uint64_t m = hole_mask; m; m &= m - 1)
      hole_bits[hole_count++] = static_cast<numbers::u16>(m & (0 - m));
    numbers::u16 board_bits[max_omaha_groups];
    unsigned board_count = 0;
    for (std::uint64_t m = board_mask; m; m &= m - 1)
      board_bits[board_count++] = static_cast<numbers::u16>(m & (0 - m));
    for (unsigned a = 0; a < hole_count; ++a)
      for (unsigned b = a + 1; b < hole_count; ++b) {
        const numbers::u16 pair = static_cast<numbers::u16>(hole_bits[a] | hole_bits[b]);
        for (unsigned x = 0; x < board_count; ++x)
          for (unsigned y = x + 1; y < board_count; ++y)
            for (unsigned z = y + 1; z < board_count; ++z)
              best = std::min(best, suited_five_rank(static_cast<numbers::u16>(
                                        pair | board_bits[x] | board_bits[y] | board_bits[z])));
      }
  }
  return best;
}

// Rank from prepared groups and the cards behind them. The sweep ranks every
// combination as if unsuited; a suited combination is a flush or straight
// flush, which beats the unsuited rank of the same five cards, so the minimum
// with the flush path is exact. Hands that cannot make a flush, about thirteen
// in fourteen, never leave the sweep.
inline rank omaha_from_groups(const numbers::u32* pairs, unsigned pair_count, const numbers::u16* triples,
                              unsigned triple_count, const card* holes, std::size_t nh, const card* board,
                              std::size_t nb, numbers::u32 hole_lanes, numbers::u32 board_lanes) noexcept {
  const rank plain = omaha_sweep(pairs, pair_count, triples, triple_count);
  const numbers::u32 flushable = flushable_suits(hole_lanes, board_lanes);
  if (!flushable) return plain;
  return std::min(plain, omaha_flush_best(holes, nh, board, nb, flushable));
}

inline rank omaha(const card* holes, std::size_t nh, const card* board, std::size_t nb) noexcept {
  if (nh > max_omaha_groups || nb > max_omaha_groups) {
    rank best = 7462;
    for (std::size_t h1 = 0; h1 + 1 < nh; ++h1)
      for (std::size_t h2 = h1 + 1; h2 < nh; ++h2)
        for (std::size_t b1 = 0; b1 + 2 < nb; ++b1)
          for (std::size_t b2 = b1 + 1; b2 + 1 < nb; ++b2)
            for (std::size_t b3 = b2 + 1; b3 < nb; ++b3) {
              const card hand[5]{holes[h1], holes[h2], board[b1], board[b2], board[b3]};
              best = std::min(best, five(hand));
              if (best == 1) return best;
            }
    return best;
  }
  numbers::u32 pairs[max_omaha_pairs];
  numbers::u16 triples[max_omaha_triples];
  const unsigned pair_count = omaha_pair_offsets(holes, nh, pairs);
  const unsigned triple_count = omaha_triple_codes(board, nb, triples);
  return omaha_from_groups(pairs, pair_count, triples, triple_count, holes, nh, board, nb,
                           suit_lanes(holes, nh), suit_lanes(board, nb));
}
} // namespace poker::detail
