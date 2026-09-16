#pragma once
#include <poker/poker.h>
#include <algorithm>
#include <array>
#include <cstdlib>
#include <cstdint>
#include <new>
#include <span>
#include <stdexcept>
#include <string_view>
#include <string>
#include <vector>

namespace test_support {
// Specified PRNG and rejection sampler: the same seed produces the same corpus
// across standard libraries. No std::uniform_int_distribution dependency.
struct Rng {
  std::uint64_t state;
  std::uint64_t next() {
    auto z = (state += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
  }
  std::size_t bounded(std::size_t bound) {
    const auto b = static_cast<std::uint64_t>(bound);
    const auto threshold = (std::uint64_t{0} - b) % b;
    while (true) { const auto x = next(); if (x >= threshold) return static_cast<std::size_t>(x % b); }
  }
  std::vector<poker_card> deal(std::size_t count, std::span<const poker_card> blocked = {}) {
    std::array<bool,52> used{};
    for (auto c : blocked) used.at(c) = true;
    std::vector<poker_card> deck;
    for (poker_card c = 0; c < 52; ++c) if (!used[c]) deck.push_back(c);
    if (count > deck.size()) throw std::invalid_argument("deal size");
    for (std::size_t i = 0; i < count; ++i) std::swap(deck[i], deck[i + bounded(deck.size() - i)]);
    deck.resize(count);
    return deck;
  }
};
inline std::uint64_t seed() {
  if (const char* value = std::getenv("POKER_TEST_SEED")) return std::stoull(value);
  return 0xd032913cc8f4671ULL;
}
inline std::vector<poker_card> cards(std::string_view text) {
  constexpr std::string_view ranks = "23456789TJQKA", suits = "cdhs";
  std::vector<poker_card> result;
  for (std::size_t i = 0; i < text.size();) {
    if (text[i] == ' ') { ++i; continue; }
    if (i + 1 >= text.size()) throw std::invalid_argument("card text");
    auto r = ranks.find(text[i++]); auto s = suits.find(text[i++]);
    if (r == ranks.npos || s == suits.npos) throw std::invalid_argument("card text");
    result.push_back(static_cast<poker_card>(r * 4 + s));
  }
  return result;
}
struct StateBuffer {
  std::size_t size = poker_backend()->state_bytes;
  std::size_t alignment = std::max(poker_backend()->state_alignment, alignof(std::max_align_t));
  void* data = ::operator new(size, std::align_val_t(alignment));
  StateBuffer() = default;
  StateBuffer(const StateBuffer&) = delete;
  StateBuffer& operator=(const StateBuffer&) = delete;
  ~StateBuffer() { ::operator delete(data, std::align_val_t(alignment)); }
};
inline constexpr std::size_t budget = 1024 * 1024;
} // namespace test_support
