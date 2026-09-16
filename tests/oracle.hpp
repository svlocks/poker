#pragma once
#include <array>
#include <cstdint>
#include <span>

// Intentionally independent of production headers, encodings, tables and classifiers.
namespace oracle {
using Strength = std::array<int, 6>;
Strength describe(std::span<const std::uint8_t, 5> hand);
std::uint16_t five(std::span<const std::uint8_t, 5> hand);
std::uint16_t high(std::span<const std::uint8_t> hand);
std::uint16_t omaha(std::span<const std::uint8_t> holes, std::span<const std::uint8_t> board);
int category(std::uint16_t rank);
void initialize();
} // namespace oracle
