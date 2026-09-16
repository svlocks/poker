#include <poker/poker.hpp>
#include <array>

int main() {
  const std::array<poker_card, 7> hand{48, 44, 40, 36, 32, 1, 6};
  return poker::evaluate(hand) == 1 && poker_eval7(hand.data()) == 1 ? 0 : 1;
}
