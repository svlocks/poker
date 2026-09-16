#include <poker/poker.hpp>
#include "oracle.hpp"
#include "support.hpp"
#include <gtest/gtest.h>
#include <atomic>
#include <cstring>
#include <limits>
#include <numeric>
#include <thread>

using namespace test_support;

TEST(Rank, GoldenValuesAndKickers) {
  const std::pair<const char*, poker_rank> cases[]{
    {"Ac Kc Qc Jc Tc", 1}, {"Ah 2h 3h 4h 5h", 10},
    {"Ac Ad Ah As Kc", 11}, {"2c 2d 2h 2s 3c", 166},
    {"Ac Ad Ah Kc Kd", 167}, {"2c 2d 2h 3c 3d", 322},
    {"Ac Kc Qc Jc 9c", 323}, {"7c 5c 4c 3c 2c", 1599},
    {"Ac Kd Qh Js Tc", 1600}, {"Ac 2d 3h 4s 5c", 1609},
    {"Ac Ad Ah Kc Qd", 1610}, {"Ac Ad Kc Kd Qh", 2468},
    {"Ac Ad Kc Qd Jh", 3326}, {"Ac Kd Qh Js 9c", 6186},
    {"7c 5d 4h 3s 2c", 7462},
    {"9c 4c 4s 9d 4h Qc 6c", 292}, {"9c 4c 4s 9d 4h 2c 9h", 236}
  };
  for (const auto& [text, expected] : cases) {
    SCOPED_TRACE(text);
    const auto hand = cards(text);
    poker_rank checked = 0;
    EXPECT_EQ(poker_eval_high(hand.data(), hand.size(), &checked), POKER_OK);
    EXPECT_EQ(checked, expected);
    EXPECT_EQ(oracle::high(hand), expected);
  }
  EXPECT_NE(poker_eval5(cards("Qc Kd Ah 2s 3c").data()), 1609);
  EXPECT_LT(poker_eval5(cards("Ac Ad Kh Qs 9c").data()), poker_eval5(cards("Ah As Kd Qc 8h").data()));
}

TEST(Rank, RandomAndMetamorphic) {
  Rng rng{seed()};
  SCOPED_TRACE(seed());
  for (std::size_t n = 5; n <= 12; ++n) for (int trial = 0; trial < 96; ++trial) {
    auto hand = rng.deal(n);
    auto expected = oracle::high(hand);
    EXPECT_EQ(poker_eval_high_unchecked(hand.data(), n), expected);
    if (n == 5) EXPECT_EQ(poker_eval5(hand.data()), expected);
    if (n == 6) EXPECT_EQ(poker_eval6(hand.data()), expected);
    if (n == 7) EXPECT_EQ(poker_eval7(hand.data()), expected);
    std::reverse(hand.begin(), hand.end());
    EXPECT_EQ(poker_eval_high_unchecked(hand.data(), n), expected);
    std::array<int,4> suits{0,1,2,3};
    for (int i = 0; i < 4; ++i) std::swap(suits[static_cast<std::size_t>(i)], suits[rng.bounded(4)]);
    for (auto& c : hand) c = static_cast<poker_card>(4 * (c / 4) + suits[c % 4]);
    EXPECT_EQ(poker_eval_high_unchecked(hand.data(), n), expected);
    const auto extra = rng.deal(1, hand);
    hand.push_back(extra[0]);
    EXPECT_LE(poker_eval_high_unchecked(hand.data(), n + 1), expected);
  }
  // Exercise every declared count with fresh cards. Exhaustion is a separate tier.
  for (std::size_t n=13; n<=52; ++n) {
    auto hand = rng.deal(n);
    EXPECT_EQ(poker_eval_high_unchecked(hand.data(), n), oracle::high(hand));
  }
  const auto deck = rng.deal(52);
  EXPECT_EQ(poker_eval_high_unchecked(deck.data(), deck.size()), 1);
}

TEST(Omaha, SelectionRestrictionsAndAllSizes) {
  auto holes = cards("Ah Kc Qd Js"), board = cards("Kh 9h 6h 2h Qc");
  auto pooled = holes; pooled.insert(pooled.end(), board.begin(), board.end());
  EXPECT_EQ(oracle::category(poker_eval_omaha_unchecked(holes.data(),4,board.data(),5)), 2); // kings and queens
  EXPECT_EQ(oracle::category(poker_eval_high_unchecked(pooled.data(),pooled.size())), 5);
  board = cards("Ac Kc Qc Jc Tc"); holes = cards("2d 3h 4s 5d");
  EXPECT_NE(poker_eval_omaha_unchecked(holes.data(),4,board.data(),5), 1);
  Rng rng{seed() ^ 0x1020304};
  for (std::size_t nh = 2; nh <= 9; ++nh) for (std::size_t nb = 3; nb <= 7; ++nb)
    for (int trial = 0; trial < 24; ++trial) {
      const auto all = rng.deal(nh + nb);
      auto h = std::vector<poker_card>(all.begin(), all.begin() + static_cast<std::ptrdiff_t>(nh));
      auto b = std::vector<poker_card>(all.begin() + static_cast<std::ptrdiff_t>(nh), all.end());
      const auto expected = oracle::omaha(h,b);
      poker_rank got = 0;
      ASSERT_EQ(poker_eval_omaha(h.data(),nh,b.data(),nb,&got), POKER_OK);
      EXPECT_EQ(got,expected);
      std::rotate(h.begin(),h.begin()+1,h.end()); std::rotate(b.begin(),b.begin()+1,b.end());
      EXPECT_EQ(poker_eval_omaha_unchecked(h.data(),nh,b.data(),nb),expected);
      const auto more = rng.deal(1, all);
      h.push_back(more[0]);
      EXPECT_LE(poker_eval_omaha_unchecked(h.data(),h.size(),b.data(),nb),expected);
    }
  for (const auto [nh,nb] : {std::pair{2U,50U}, std::pair{49U,3U}}) {
    const auto all = rng.deal(52);
    EXPECT_EQ(poker_eval_omaha_unchecked(all.data(),nh,all.data()+nh,nb),
      oracle::omaha(std::span(all).first(nh),std::span(all).subspan(nh)));
  }
  for (int trial=0; trial<48; ++trial) {
    const std::size_t nh=2+rng.bounded(48);
    const std::size_t nb=3+rng.bounded(50-nh);
    const auto all=rng.deal(nh+nb);
    EXPECT_EQ(poker_eval_omaha_unchecked(all.data(),nh,all.data()+nh,nb),
      oracle::omaha(std::span(all).first(nh),std::span(all).subspan(nh)));
  }
}

TEST(Validation, RejectsInvalidInputsWithoutChangingOutput) {
  auto hand = cards("Ac Kd Qh Js 9c");
  poker_rank out = 999;
  EXPECT_EQ(poker_eval_high(nullptr,5,&out),POKER_INVALID_ARGUMENT);
  EXPECT_EQ(poker_eval_high(hand.data(),5,nullptr),POKER_INVALID_ARGUMENT);
  for (const std::size_t n : {0U,1U,4U,53U}) EXPECT_EQ(poker_eval_high(hand.data(),n,&out),POKER_INVALID_COUNT);
  EXPECT_EQ(poker_eval_high(hand.data(),std::numeric_limits<std::size_t>::max(),&out),POKER_INVALID_COUNT);
  for (unsigned invalid = 52; invalid <= 255; ++invalid) {
    auto bad = hand; bad[0] = static_cast<poker_card>(invalid);
    EXPECT_EQ(poker_eval_high(bad.data(),5,&out),POKER_INVALID_CARD);
  }
  auto duplicate = hand; duplicate[4] = duplicate[1];
  EXPECT_EQ(poker_eval_high(duplicate.data(),5,&out),POKER_DUPLICATE_CARD);
  EXPECT_EQ(poker_eval_omaha(hand.data(),2,hand.data(),3,&out),POKER_DUPLICATE_CARD);
  EXPECT_EQ(poker_eval_omaha(hand.data(),std::numeric_limits<std::size_t>::max(),hand.data(),3,&out),POKER_INVALID_COUNT);
  EXPECT_EQ(out,999);
}

TEST(Batch, PackedAndSharedBoardWithCanaries) {
  Rng rng{seed() ^ 777};
  for (std::size_t n : {5U,6U,7U,9U}) for (std::size_t size : {0U,1U,3U,17U,65U}) {
    std::vector<poker_card> input;
    std::vector<poker_rank> expected;
    for (std::size_t i=0;i<size;++i) { auto h=rng.deal(n); expected.push_back(oracle::high(h)); input.insert(input.end(),h.begin(),h.end()); }
    const auto copy=input;
    std::vector<poker_rank> out(size+2,65535);
    poker_high_batch(input.data(),n,size,out.data()+1);
    EXPECT_EQ(out.front(),65535); EXPECT_EQ(out.back(),65535); EXPECT_EQ(input,copy);
    EXPECT_TRUE(std::equal(expected.begin(),expected.end(),out.begin()+1));
  }
  for (std::size_t nh=4;nh<=6;++nh) for(std::size_t nb=3;nb<=5;++nb) {
    const auto shared=rng.deal(nb);
    for (std::size_t size : {0U,1U,7U,31U}) {
      std::vector<poker_card> holes,boards;
      std::vector<poker_rank> expect(size),out(size+2,65535);
      for(std::size_t i=0;i<size;++i) {
        auto h=rng.deal(nh,shared); holes.insert(holes.end(),h.begin(),h.end());
        expect[i]=oracle::omaha(h,shared);
      }
      poker_omaha_board_batch(holes.data(),nh,shared.data(),nb,size,out.data()+1);
      EXPECT_TRUE(std::equal(expect.begin(),expect.end(),out.begin()+1));
      for(std::size_t i=0;i<size;++i) {
        auto h=std::span(holes).subspan(i*nh,nh); auto b=rng.deal(nb,h);
        boards.insert(boards.end(),b.begin(),b.end()); expect[i]=oracle::omaha(h,b);
      }
      poker_omaha_batch(holes.data(),nh,boards.data(),nb,size,out.data()+1);
      EXPECT_TRUE(std::equal(expect.begin(),expect.end(),out.begin()+1));
      EXPECT_EQ(out.front(),65535); EXPECT_EQ(out.back(),65535);
    }
  }
  for (std::size_t nb=3;nb<=5;++nb) {
    auto board=rng.deal(nb); std::vector<poker_card> holes; std::vector<poker_rank> expect;
    for(int i=0;i<33;++i) { auto h=rng.deal(2,board); holes.insert(holes.end(),h.begin(),h.end()); h.insert(h.end(),board.begin(),board.end()); expect.push_back(oracle::high(h)); }
    std::vector<poker_rank> out(33); poker_holdem_board_batch(holes.data(),board.data(),nb,33,out.data()); EXPECT_EQ(out,expect);
  }
  poker_high_batch(nullptr,7,0,nullptr); poker_omaha_batch(nullptr,4,nullptr,5,0,nullptr);
  poker_omaha_board_batch(nullptr,4,nullptr,5,0,nullptr); poker_holdem_board_batch(nullptr,nullptr,5,0,nullptr);
  poker_complete_batch(nullptr,nullptr,nullptr,0,nullptr);
}

TEST(Prepared, AllSplitPositionsAndBranching) {
  Rng rng{seed() ^ 42};
  for(std::size_t n=5;n<=9;++n) for(std::size_t known=0;known<=n;++known) {
    auto hand=rng.deal(n); const auto expected=oracle::high(hand); StateBuffer state,copy;
    ASSERT_EQ(poker_prepare_high(state.data,state.size,hand.data(),known,n,budget),POKER_OK);
    ASSERT_EQ(poker_clone(state.data,copy.data,copy.size),POKER_OK);
    poker_rank got=0;
    EXPECT_EQ(poker_complete(state.data,hand.data()+known,nullptr,&got),POKER_OK); EXPECT_EQ(got,expected);
    EXPECT_EQ(poker_complete_unchecked(copy.data,hand.data()+known,nullptr),expected);
    if(known<n) {
      EXPECT_EQ(poker_extend(copy.data,hand.data()+known,1,nullptr,0,copy.data,copy.size),POKER_OK);
      EXPECT_EQ(poker_complete_unchecked(copy.data,hand.data()+known+1,nullptr),expected);
      EXPECT_EQ(poker_complete_unchecked(state.data,hand.data()+known,nullptr),expected);
    }
  }
  for(std::size_t nh=4;nh<=6;++nh) for(std::size_t nb=3;nb<=5;++nb)
    for(std::size_t kh=0;kh<=nh;++kh) for(std::size_t kb=0;kb<=nb;++kb) {
      auto all=rng.deal(nh+nb); StateBuffer state,branch;
      auto h=std::span(all).first(nh), b=std::span(all).subspan(nh);
      const auto expected=oracle::omaha(h,b);
      ASSERT_EQ(poker_prepare_omaha(state.data,state.size,h.data(),kh,b.data(),kb,nh,nb,budget),POKER_OK);
      poker_rank got=0; ASSERT_EQ(poker_complete(state.data,h.data()+kh,b.data()+kb,&got),POKER_OK); EXPECT_EQ(got,expected);
      ASSERT_EQ(poker_extend(state.data,h.data()+kh,nh-kh,b.data()+kb,nb-kb,branch.data,branch.size),POKER_OK);
      EXPECT_EQ(poker_complete_unchecked(branch.data,nullptr,nullptr),expected);
    }
}

TEST(Prepared, BatchesBudgetsAndFailures) {
  Rng rng{seed()}; StateBuffer state,copy;
  const auto fixed=rng.deal(7); // 4 holes + flop
  ASSERT_EQ(poker_prepare_omaha(state.data,state.size,fixed.data(),4,fixed.data()+4,3,4,5,budget),POKER_OK);
  std::vector<poker_card> completions; std::vector<poker_rank> expected;
  for(int i=0;i<71;++i) {
    auto runout=rng.deal(2,fixed); completions.insert(completions.end(),runout.begin(),runout.end());
    auto b=std::vector<poker_card>(fixed.begin()+4,fixed.end()); b.insert(b.end(),runout.begin(),runout.end());
    expected.push_back(oracle::omaha(std::span(fixed).first(4),b));
  }
  std::vector<poker_rank> out(expected.size());
  poker_complete_batch(state.data,nullptr,completions.data(),out.size(),out.data()); EXPECT_EQ(out,expected);
  const auto snapshot=std::vector<unsigned char>(static_cast<unsigned char*>(state.data),static_cast<unsigned char*>(state.data)+state.size);
  poker_rank rank=999;
  EXPECT_EQ(poker_complete(state.data,nullptr,fixed.data(),&rank),POKER_DUPLICATE_CARD); EXPECT_EQ(rank,999);
  EXPECT_EQ(poker_extend(state.data,nullptr,0,fixed.data(),1,state.data,state.size),POKER_DUPLICATE_CARD);
  EXPECT_EQ(std::memcmp(snapshot.data(),state.data,state.size),0);
  EXPECT_EQ(poker_prepare_high(state.data,state.size,fixed.data(),4,7,0),POKER_INSUFFICIENT_MEMORY);
  EXPECT_EQ(poker_prepare_high(state.data,0,fixed.data(),4,7,budget),POKER_INVALID_ARGUMENT);
  EXPECT_EQ(poker_prepare_high(state.data,state.size,fixed.data(),8,7,budget),POKER_INVALID_COUNT);
  std::memset(copy.data,0,copy.size);
  EXPECT_EQ(poker_clone(copy.data,state.data,state.size),POKER_INVALID_STATE);
  EXPECT_EQ(poker_initialize("gpu",budget),POKER_UNSUPPORTED_BACKEND);
  EXPECT_EQ(poker_initialize("portable",0),POKER_INSUFFICIENT_MEMORY);
  for(const auto cap : {1U<<20,32U<<20,512U<<20}) EXPECT_EQ(poker_initialize("portable",cap),POKER_OK);
}

TEST(Native, FixedSizeEntryPoints) {
  Rng rng{seed()};
  for(int i=0;i<100;++i) {
    auto v=rng.deal(11);
    std::array<poker_card,5> five; std::copy_n(v.begin(),5,five.begin());
    std::array<poker_card,6> six; std::copy_n(v.begin(),6,six.begin());
    std::array<poker_card,7> seven; std::copy_n(v.begin(),7,seven.begin());
    EXPECT_EQ(poker::evaluate(five),oracle::high(five)); EXPECT_EQ(poker::evaluate(six),oracle::high(six)); EXPECT_EQ(poker::evaluate(seven),oracle::high(seven));
    std::array<poker_card,4> holes; std::copy_n(v.begin()+5,4,holes.begin());
    EXPECT_EQ(poker::evaluate_omaha(holes,five),oracle::omaha(holes,five));
  }
}

TEST(Concurrency, ImmutableStateAndIndependentWorkers) {
  StateBuffer state; auto fixed=cards("Ac Kd Qh Js Tc");
  ASSERT_EQ(poker_prepare_high(state.data,state.size,fixed.data(),3,7,budget),POKER_OK);
  std::atomic<bool> good{true}; std::vector<std::thread> workers;
  for(std::uint64_t t=0;t<8;++t) workers.emplace_back([&,t] {
    Rng rng{seed()+t};
    for(int i=0;i<100;++i) {
      auto extra=rng.deal(4,std::span(fixed).first(3)); auto h=std::vector<poker_card>(fixed.begin(),fixed.begin()+3); h.insert(h.end(),extra.begin(),extra.end());
      if(poker_complete_unchecked(state.data,extra.data(),nullptr)!=oracle::high(h)) good=false;
    }
  });
  for(auto& worker:workers) worker.join(); EXPECT_TRUE(good);
}
