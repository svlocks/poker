#pragma once
#include <poker/poker.h>
#include <poker/detail/portable.hpp>
#include <array>
#include <cstddef>
#include <span>

namespace poker {
using card = poker_card;
using rank = poker_rank;
template<std::size_t N>
inline rank evaluate(const std::array<card, N>& cards) noexcept {
  static_assert(N >= 5 && N <= 52);
  if constexpr (N == 5) return detail::five(cards.data());
  else return detail::high(cards.data(), N);
}
template<std::size_t H, std::size_t B>
inline rank evaluate_omaha(const std::array<card, H>& holes, const std::array<card, B>& board) noexcept {
  static_assert(H >= 2 && B >= 3 && H + B <= 52);
  return detail::omaha(holes.data(), H, board.data(), B);
}
} // namespace poker
