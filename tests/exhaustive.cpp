#include <poker/poker.hpp>
#include "oracle.hpp"
#include <array>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

static std::uint64_t choose(unsigned n, unsigned k) {
  if(k>n) return 0;
  std::uint64_t r=1;
  for(unsigned i=1;i<=k;++i) r=r*(n-k+i)/i;
  return r;
}
int main(int argc,char** argv) {
  try {
    if(argc!=4) throw std::runtime_error("usage: poker_exhaustive CARDS SHARD SHARDS");
    const auto n=static_cast<unsigned>(std::stoul(argv[1]));
    const auto shard=static_cast<std::uint64_t>(std::stoull(argv[2]));
    const auto shards=static_cast<std::uint64_t>(std::stoull(argv[3]));
    if(n<5||n>7||!shards||shard>=shards||shards>choose(52,n)) throw std::runtime_error("invalid shard");
    const auto total=choose(52,n), begin=total*shard/shards, end=total*(shard+1)/shards;
    std::vector<poker_card> hand(n);
    auto index=begin; unsigned first=0;
    for(unsigned i=0;i<n;++i) for(unsigned c=first;c<52;++c) {
      auto block=choose(51-c,n-i-1);
      if(index<block) { hand[i]=static_cast<poker_card>(c); first=c+1; break; }
      index-=block;
    }
    std::array<poker_card,5> five{};
    std::array<poker_card,6> six{};
    std::array<poker_card,7> seven{};
    std::array<std::uint64_t,9> categories{};
    std::array<std::uint64_t,7463> ranks{};
    for(auto ordinal=begin;ordinal<end;++ordinal) {
      const auto expected=oracle::high(hand), actual=poker_eval_high_unchecked(hand.data(),n);
      poker_rank direct=0, native=0, checked=0;
      if(n==5) { std::copy_n(hand.begin(),5,five.begin()); direct=poker_eval5(hand.data()); native=poker::evaluate(five); }
      if(n==6) { std::copy_n(hand.begin(),6,six.begin()); direct=poker_eval6(hand.data()); native=poker::evaluate(six); }
      if(n==7) { std::copy_n(hand.begin(),7,seven.begin()); direct=poker_eval7(hand.data()); native=poker::evaluate(seven); }
      const auto status=poker_eval_high(hand.data(),n,&checked);
      if(actual!=expected || direct!=expected || native!=expected || status!=POKER_OK || checked!=expected) {
        std::cerr<<"mismatch at "<<ordinal<<": expected "<<expected<<", got "<<actual<<", direct "<<direct<<", native "<<native<<", checked "<<checked<<" cards:";
        for(auto c:hand) std::cerr<<' '<<static_cast<unsigned>(c);
        std::cerr<<'\n'; return 1;
      }
      ++ranks[actual]; ++categories[static_cast<std::size_t>(oracle::category(actual))];
      int p=static_cast<int>(n)-1;
      while(p>=0 && hand[static_cast<std::size_t>(p)]==52-n+static_cast<unsigned>(p)) --p;
      if(p>=0) {
        ++hand[static_cast<std::size_t>(p)];
        for(unsigned i=static_cast<unsigned>(p)+1;i<n;++i) hand[i]=static_cast<poker_card>(hand[i-1]+1);
      }
    }
    if(n==5&&shards==1) {
      const std::array<std::uint64_t,9> known{1302540,1098240,123552,54912,10200,5108,3744,624,40};
      const std::array<std::uint64_t,9> per_rank{1020,384,144,64,1020,4,24,4,4};
      if(categories!=known) throw std::runtime_error("five-card category census mismatch");
      for(unsigned r=1;r<=7462;++r) if(ranks[r]!=per_rank[static_cast<std::size_t>(oracle::category(static_cast<std::uint16_t>(r)))])
        throw std::runtime_error("five-card rank multiplicity mismatch");
    }
    std::cout<<"{\"cards\":"<<n<<",\"shard\":"<<shard<<",\"shards\":"<<shards<<",\"begin\":"<<begin<<",\"end\":"<<end<<",\"checked\":"<<end-begin<<",\"total\":"<<total<<"}\n";
    return 0;
  } catch(const std::exception& e) { std::cerr<<e.what()<<'\n'; return 2; }
}
