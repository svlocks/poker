#include "oracle.hpp"
#include <algorithm>
#include <functional>
#include <map>
#include <stdexcept>
#include <vector>

namespace oracle {
namespace {
int straight(const std::vector<int>& desc) {
  if (desc.size() != 5) return 0;
  if (desc == std::vector<int>{14, 5, 4, 3, 2}) return 5;
  for (std::size_t i = 1; i < desc.size(); ++i) if (desc[i - 1] != desc[i] + 1) return 0;
  return desc.front();
}
std::vector<Strength> make_classes() {
  std::vector<Strength> out;
  for (int a = 14; a >= 2; --a) for (int b = a - 1; b >= 2; --b)
    for (int c = b - 1; c >= 2; --c) for (int d = c - 1; d >= 2; --d)
      for (int e = d - 1; e >= 2; --e) {
        if (straight({a,b,c,d,e})) continue;
        out.push_back({0,a,b,c,d,e}); out.push_back({5,a,b,c,d,e});
      }
  for (int p = 2; p <= 14; ++p) {
    for (int a = 14; a >= 2; --a) if (a != p)
      for (int b = a - 1; b >= 2; --b) if (b != p) {
        out.push_back({3,p,a,b,0,0});
        for (int c = b - 1; c >= 2; --c) if (c != p) out.push_back({1,p,a,b,c,0});
      }
    for (int k = 2; k <= 14; ++k) if (k != p) {
      out.push_back({7,p,k,0,0,0}); out.push_back({6,p,k,0,0,0});
    }
    for (int q = 2; q < p; ++q) for (int k = 2; k <= 14; ++k)
      if (k != p && k != q) out.push_back({2,p,q,k,0,0});
  }
  for (int top = 5; top <= 14; ++top) {
    out.push_back({4,top,0,0,0,0}); out.push_back({8,top,0,0,0,0});
  }
  std::sort(out.begin(), out.end(), std::greater<Strength>{});
  if (out.size() != 7462 || std::adjacent_find(out.begin(), out.end()) != out.end()) throw std::logic_error("reference rank classes invalid");
  return out;
}
const std::vector<Strength>& classes() { static const auto value = make_classes(); return value; }
} // namespace

Strength describe(std::span<const std::uint8_t, 5> hand) {
  std::map<int, int> counts;
  std::vector<int> ranks;
  bool flush = true;
  for (const auto c : hand) {
    ++counts[2 + c / 4];
    flush = flush && c % 4 == hand.front() % 4;
  }
  std::vector<std::pair<int,int>> groups;
  for (const auto& [r, n] : counts) { groups.emplace_back(n, r); ranks.push_back(r); }
  std::sort(groups.begin(), groups.end(), std::greater<>{});
  std::sort(ranks.begin(), ranks.end(), std::greater<>{});
  const auto top = straight(ranks);
  if (flush && top) return {8, top, 0, 0, 0, 0};
  if (groups[0].first == 4) return {7, groups[0].second, groups[1].second, 0, 0, 0};
  if (groups[0].first == 3 && groups[1].first == 2) return {6, groups[0].second, groups[1].second, 0, 0, 0};
  if (flush) return {5, ranks[0], ranks[1], ranks[2], ranks[3], ranks[4]};
  if (top) return {4, top, 0, 0, 0, 0};
  if (groups[0].first == 3) return {3, groups[0].second, groups[1].second, groups[2].second, 0, 0};
  if (groups[0].first == 2 && groups[1].first == 2) return {2, groups[0].second, groups[1].second, groups[2].second, 0, 0};
  if (groups[0].first == 2) return {1, groups[0].second, groups[1].second, groups[2].second, groups[3].second, 0};
  return {0, ranks[0], ranks[1], ranks[2], ranks[3], ranks[4]};
}
std::uint16_t five(std::span<const std::uint8_t, 5> hand) {
  const auto key = describe(hand);
  const auto& order = classes();
  const auto it = std::lower_bound(order.begin(), order.end(), key, std::greater<Strength>{});
  if (it == order.end() || *it != key) throw std::logic_error("reference hand missing from rank classes");
  return static_cast<std::uint16_t>(1 + (it - order.begin()));
}
std::uint16_t high(std::span<const std::uint8_t> hand) {
  if (hand.size() < 5 || hand.size() > 52) throw std::invalid_argument("reference high count");
  std::array<std::size_t,5> idx{0,1,2,3,4};
  std::uint16_t best = 7462;
  while (true) {
    std::array<std::uint8_t,5> selected{};
    for (int i = 0; i < 5; ++i) selected[static_cast<std::size_t>(i)] = hand[idx[static_cast<std::size_t>(i)]];
    best = std::min(best, five(selected));
    int pos = 4;
    while (pos >= 0 && idx[static_cast<std::size_t>(pos)] == hand.size() - 5 + static_cast<std::size_t>(pos)) --pos;
    if (pos < 0) break;
    ++idx[static_cast<std::size_t>(pos)];
    for (int i = pos + 1; i < 5; ++i) idx[static_cast<std::size_t>(i)] = idx[static_cast<std::size_t>(i-1)] + 1;
  }
  return best;
}
std::uint16_t omaha(std::span<const std::uint8_t> h, std::span<const std::uint8_t> b) {
  if (h.size() < 2 || b.size() < 3 || h.size() + b.size() > 52) throw std::invalid_argument("reference Omaha count");
  std::uint16_t best = 7462;
  std::array<std::uint8_t,5> selected{};
  for (std::size_t i = 0; i < h.size(); ++i) for (std::size_t j = 0; j < i; ++j)
    for (std::size_t x = 0; x < b.size(); ++x) for (std::size_t y = 0; y < x; ++y)
      for (std::size_t z = 0; z < y; ++z) {
        selected = {h[j], h[i], b[z], b[y], b[x]};
        best = std::min(best, five(selected));
      }
  return best;
}
int category(std::uint16_t rank) { return classes().at(rank - 1)[0]; }
void initialize() { (void)classes(); }
} // namespace oracle
