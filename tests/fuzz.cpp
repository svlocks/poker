#include <poker/poker.h>
#include "oracle.hpp"
#include "support.hpp"
#include <algorithm>
#include <array>
#include <cstdlib>
#include <span>

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
  if(size<6) return 0;
  const bool omaha=data[0]&1;
  const std::size_t n=std::min<std::size_t>(size-1,11);
  std::array<poker_card,11> hand{};
  std::copy_n(data+1,n,hand.begin());
  poker_rank actual=0;
  if(!omaha) {
    if(poker_eval_high(hand.data(),n,&actual)==POKER_OK && actual!=oracle::high(std::span(hand).first(n))) std::abort();
  } else {
    const std::size_t h=2+(data[0]>>1)%std::max<std::size_t>(1,n-4);
    if(poker_eval_omaha(hand.data(),h,hand.data()+h,n-h,&actual)==POKER_OK && actual!=oracle::omaha(std::span(hand).first(h),std::span(hand).subspan(h,n-h))) std::abort();
  }
  // Exercise a valid hand on every sufficiently long input as well as raw
  // invalid-byte validation. Otherwise most random bytes only test rejection.
  std::array<poker_card,52> deck{};
  for(std::size_t i=0;i<52;++i) deck[i]=static_cast<poker_card>(i);
  for(std::size_t i=0;i<n;++i) std::swap(deck[i],deck[i+data[i+1]%(52-i)]);
  poker_rank expected=0;
  test_support::StateBuffer state;
  if(!omaha) {
    expected=oracle::high(std::span(deck).first(n));
    if(poker_eval_high_unchecked(deck.data(),n)!=expected) std::abort();
    const std::size_t known=data[0]%n;
    if(poker_prepare_high(state.data,state.size,deck.data(),known,n,test_support::budget)!=POKER_OK) std::abort();
    if(poker_complete_unchecked(state.data,deck.data()+known,nullptr)!=expected) std::abort();
  } else {
    const std::size_t h=2+(data[0]>>1)%std::max<std::size_t>(1,n-4);
    expected=oracle::omaha(std::span(deck).first(h),std::span(deck).subspan(h,n-h));
    if(poker_eval_omaha_unchecked(deck.data(),h,deck.data()+h,n-h)!=expected) std::abort();
    if(poker_prepare_omaha(state.data,state.size,deck.data(),h,deck.data()+h,1,h,n-h,test_support::budget)!=POKER_OK) std::abort();
    if(poker_complete_unchecked(state.data,nullptr,deck.data()+h+1)!=expected) std::abort();
  }
  return 0;
}
