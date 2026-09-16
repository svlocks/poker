#include <poker/poker.hpp>
#include "oracle.hpp"
#include "support.hpp"
#include <benchmark/benchmark.h>
#include <chrono>
#include <cstring>
#include <barrier>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

using namespace test_support;
namespace {
enum class Kind { scalar, native, omaha, high_batch, omaha_batch, shared_high, shared_omaha,
  prepared_high, total_high, prepared_omaha, total_omaha, simulation_high, simulation_omaha,
  parallel_high, parallel_omaha, clone_high, extend_high, clone_omaha, extend_omaha };
struct Profile { std::string name; Kind kind; std::size_t h,b,batch,known; };
struct Job {
  std::vector<poker_card> h,b;
  std::vector<poker_rank> expected,out;
  std::unique_ptr<StateBuffer> state;
  std::unique_ptr<StateBuffer> destination;
};
std::uint64_t corpus_seed=0;
std::size_t records=512;
bool smoke=false;
bool failed=false;
std::vector<Profile> profiles() {
  std::vector<Profile> p;
  for(std::size_t n:{5U,6U,7U}) {
    p.push_back({"scalar/high/"+std::to_string(n),Kind::scalar,n,0,1,0});
    p.push_back({"native/high/"+std::to_string(n),Kind::native,n,0,1,0});
  }
  for(std::size_t n:{5U,6U,7U,9U}) for(std::size_t b:{1U,8U,31U,128U,1024U})
    p.push_back({"batch/high/"+std::to_string(n)+"/"+std::to_string(b),Kind::high_batch,n,0,b,0});
  for(std::size_t h=4;h<=6;++h) for(std::size_t b=3;b<=5;++b) {
    auto shape=std::to_string(h)+"/"+std::to_string(b);
    p.push_back({"scalar/omaha/"+shape,Kind::omaha,h,b,1,0});
    for(std::size_t n:{1U,8U,64U,1024U}) p.push_back({"batch/omaha/"+shape+"/"+std::to_string(n),Kind::omaha_batch,h,b,n,0});
    for(std::size_t n:{2U,6U}) p.push_back({"shared/omaha/"+shape+"/"+std::to_string(n),Kind::shared_omaha,h,b,n,0});
  }
  for(std::size_t b=3;b<=5;++b) for(std::size_t n:{2U,6U,9U})
    p.push_back({"shared/holdem/"+std::to_string(b)+"/"+std::to_string(n),Kind::shared_high,2,b,n,0});
  for(std::size_t known:{2U,5U,6U}) for(std::size_t reuse:{1U,8U,64U}) {
    auto shape=std::to_string(known)+"/"+std::to_string(reuse);
    p.push_back({"prepared/high/"+shape,Kind::prepared_high,7,0,reuse,known});
    p.push_back({"total/high/"+shape,Kind::total_high,7,0,reuse,known});
  }
  for(std::size_t h=4;h<=6;++h) for(std::size_t reuse:{1U,16U,128U}) {
    auto shape=std::to_string(h)+"/"+std::to_string(reuse);
    p.push_back({"prepared/omaha/"+shape,Kind::prepared_omaha,h,5,reuse,3});
    p.push_back({"total/omaha/"+shape,Kind::total_omaha,h,5,reuse,3});
  }
  for(std::size_t players:{2U,6U}) {
    p.push_back({"simulation/holdem/"+std::to_string(players),Kind::simulation_high,2,5,players,0});
    p.push_back({"simulation/omaha/"+std::to_string(players),Kind::simulation_omaha,4,5,players,0});
  }
  for(std::size_t threads:{1U,2U,4U}) {
    p.push_back({"parallel/high/7/"+std::to_string(threads),Kind::parallel_high,7,0,threads,0});
    p.push_back({"parallel/omaha/4/5/"+std::to_string(threads),Kind::parallel_omaha,4,5,threads,0});
  }
  p.push_back({"state/clone/high",Kind::clone_high,7,0,1,5});
  p.push_back({"state/extend/high",Kind::extend_high,7,0,1,5});
  p.push_back({"state/clone/omaha",Kind::clone_omaha,4,5,1,3});
  p.push_back({"state/extend/omaha",Kind::extend_omaha,4,5,1,3});
  return p;
}
bool state_operation(const Profile& p) {
  return p.kind==Kind::clone_high||p.kind==Kind::extend_high||p.kind==Kind::clone_omaha||p.kind==Kind::extend_omaha;
}
std::uint64_t name_hash(const std::string& s) {
  std::uint64_t h=14695981039346656037ULL;
  for(unsigned char c:s) h=(h^c)*1099511628211ULL;
  return h;
}
bool is_high(const Profile& p) {
  return p.kind==Kind::scalar||p.kind==Kind::native||p.kind==Kind::high_batch||p.kind==Kind::shared_high||
    p.kind==Kind::prepared_high||p.kind==Kind::total_high||p.kind==Kind::simulation_high||
    p.kind==Kind::parallel_high||p.kind==Kind::clone_high||p.kind==Kind::extend_high;
}
void prepare(const Profile& p,Job& job) {
  poker_status status=POKER_OK;
  if(is_high(p))
    status=poker_prepare_high(job.state->data,job.state->size,job.b.data(),p.known,p.h,budget);
  else status=poker_prepare_omaha(job.state->data,job.state->size,job.b.data(),p.h,job.b.data()+p.h,p.known,p.h,p.b,budget);
  if(status!=POKER_OK) throw std::runtime_error("preparation failed");
}
Job make_job(const Profile& p,Rng& rng) {
  Job j;
  const bool prep=p.kind==Kind::prepared_high||p.kind==Kind::total_high||p.kind==Kind::prepared_omaha||p.kind==Kind::total_omaha||state_operation(p);
  const bool shared=p.kind==Kind::shared_high||p.kind==Kind::shared_omaha;
  if(prep) { j.b=rng.deal(p.known+(is_high(p)?0:p.h)); j.state=std::make_unique<StateBuffer>(); }
  else if(shared) j.b=rng.deal(p.b);
  for(std::size_t i=0;i<p.batch;++i) {
    if(prep) {
      const auto extra=rng.deal(is_high(p)?p.h-p.known:p.b-p.known,j.b);
      j.h.insert(j.h.end(),extra.begin(),extra.end());
      auto all=j.b; all.insert(all.end(),extra.begin(),extra.end());
      j.expected.push_back(is_high(p)?oracle::high(all):oracle::omaha(std::span(all).first(p.h),std::span(all).subspan(p.h)));
    } else if(shared) {
      auto h=rng.deal(p.h,j.b); j.h.insert(j.h.end(),h.begin(),h.end());
      if(is_high(p)) { h.insert(h.end(),j.b.begin(),j.b.end()); j.expected.push_back(oracle::high(h)); }
      else j.expected.push_back(oracle::omaha(h,j.b));
    } else {
      auto all=rng.deal(p.h+p.b);
      j.h.insert(j.h.end(),all.begin(),all.begin()+static_cast<std::ptrdiff_t>(p.h));
      j.b.insert(j.b.end(),all.begin()+static_cast<std::ptrdiff_t>(p.h),all.end());
      j.expected.push_back(is_high(p)?oracle::high(all):oracle::omaha(std::span(all).first(p.h),std::span(all).subspan(p.h)));
    }
  }
  j.out.assign(p.batch,0);
  if(p.kind==Kind::prepared_high||p.kind==Kind::prepared_omaha||state_operation(p)) prepare(p,j);
  if(state_operation(p)) {
    j.destination=std::make_unique<StateBuffer>();
    std::memset(j.destination->data,0,j.destination->size);
  }
  return j;
}
poker_rank scalar(const Profile& p,const poker_card* h,const poker_card* b) {
  if(p.kind==Kind::omaha) return poker_eval_omaha_unchecked(h,p.h,b,p.b);
  if(p.kind==Kind::native) {
    if(p.h==5) { std::array<poker_card,5> v; std::copy_n(h,5,v.begin()); return poker::evaluate(v); }
    if(p.h==6) { std::array<poker_card,6> v; std::copy_n(h,6,v.begin()); return poker::evaluate(v); }
    std::array<poker_card,7> v; std::copy_n(h,7,v.begin()); return poker::evaluate(v);
  }
  if(p.h==5) return poker_eval5(h);
  if(p.h==6) return poker_eval6(h);
  return poker_eval7(h);
}
void execute(const Profile& p,Job& j) {
  switch(p.kind) {
    case Kind::clone_high: case Kind::clone_omaha:
      j.out[0]=static_cast<poker_rank>(poker_clone(j.state->data,j.destination->data,j.destination->size)); break;
    case Kind::extend_high:
      j.out[0]=static_cast<poker_rank>(poker_extend(j.state->data,j.h.data(),1,nullptr,0,j.destination->data,j.destination->size)); break;
    case Kind::extend_omaha:
      j.out[0]=static_cast<poker_rank>(poker_extend(j.state->data,nullptr,0,j.h.data(),1,j.destination->data,j.destination->size)); break;
    case Kind::high_batch: poker_high_batch(j.h.data(),p.h,p.batch,j.out.data()); break;
    case Kind::omaha_batch: poker_omaha_batch(j.h.data(),p.h,j.b.data(),p.b,p.batch,j.out.data()); break;
    case Kind::shared_high: poker_holdem_board_batch(j.h.data(),j.b.data(),p.b,p.batch,j.out.data()); break;
    case Kind::shared_omaha: poker_omaha_board_batch(j.h.data(),p.h,j.b.data(),p.b,p.batch,j.out.data()); break;
    case Kind::total_high: prepare(p,j); [[fallthrough]];
    case Kind::prepared_high: poker_complete_batch(j.state->data,j.h.data(),nullptr,p.batch,j.out.data()); break;
    case Kind::total_omaha: prepare(p,j); [[fallthrough]];
    case Kind::prepared_omaha: poker_complete_batch(j.state->data,nullptr,j.h.data(),p.batch,j.out.data()); break;
    default: j.out[0]=scalar(p,j.h.data(),j.b.data()); break;
  }
}
std::uint64_t simulate(const Profile& p,std::uint64_t rng_seed,std::size_t trials,bool reference,std::vector<poker_rank>& history) {
  Rng rng{rng_seed}; std::uint64_t tally=0;
  for(std::size_t t=0;t<trials;++t) {
    const auto all=rng.deal(p.h*p.batch+p.b);
    const auto board=std::span(all).last(p.b);
    std::vector<poker_rank> ranks(p.batch);
    for(std::size_t player=0;player<p.batch;++player) {
      auto h=std::span(all).subspan(player*p.h,p.h);
      if(is_high(p)) {
        std::array<poker_card,7> hand{}; std::copy(h.begin(),h.end(),hand.begin()); std::copy(board.begin(),board.end(),hand.begin()+2);
        ranks[player]=reference?oracle::high(hand):poker_eval7(hand.data());
      } else ranks[player]=reference?oracle::omaha(h,board):poker_eval_omaha_unchecked(h.data(),p.h,board.data(),p.b);
      history[t*p.batch+player]=ranks[player];
      tally=(tally^ranks[player])*1099511628211ULL;
    }
    const auto best=*std::min_element(ranks.begin(),ranks.end());
    for(std::size_t player=0;player<p.batch;++player) if(ranks[player]==best) tally=(tally^(player+1))*1099511628211ULL;
  }
  return tally;
}
void measure(benchmark::State& state,Profile p) {
  try {
    const auto run_seed=corpus_seed^name_hash(p.name);
    const auto target=smoke?8:records;
    if(p.kind==Kind::simulation_high||p.kind==Kind::simulation_omaha) {
      std::vector<poker_rank> expected_ranks(target*p.batch), actual_ranks(target*p.batch,0);
      const auto expected=simulate(p,run_seed,target,true,expected_ranks);
      std::uint64_t result=0;
      for(auto _:state) { (void)_; result=simulate(p,run_seed,target,false,actual_ranks); benchmark::DoNotOptimize(result); }
      if(result!=expected || actual_ranks!=expected_ranks) { failed=true; state.SkipWithError("simulation output mismatch"); return; }
      state.counters["verified_items"]=static_cast<double>(target*p.batch);
      state.counters["verified_trials"]=static_cast<double>(target);
      state.SetItemsProcessed(static_cast<std::int64_t>(target*p.batch));
      return;
    }
    const bool parallel=p.kind==Kind::parallel_high||p.kind==Kind::parallel_omaha;
    const auto workers=parallel?p.batch:1;
    Profile work=p;
    if(parallel) { work.batch=64; work.kind=is_high(p)?Kind::high_batch:Kind::omaha_batch; }
    const auto jobs_count=std::max<std::size_t>(workers,(target+work.batch-1)/work.batch);
    Rng rng{run_seed}; std::vector<Job> jobs;
    jobs.reserve(jobs_count);
    std::vector<std::vector<poker_card>> input_h,input_b;
    for(std::size_t i=0;i<jobs_count;++i) {
      jobs.push_back(make_job(work,rng)); input_h.push_back(jobs.back().h); input_b.push_back(jobs.back().b);
    }
    const bool serial=p.kind==Kind::scalar||p.kind==Kind::native||p.kind==Kind::omaha;
    std::barrier rendezvous(static_cast<std::ptrdiff_t>(workers+1));
    std::vector<std::thread> threads;
    if(parallel) for(std::size_t worker=0;worker<workers;++worker) threads.emplace_back([&,worker] {
      rendezvous.arrive_and_wait();
      for(std::size_t i=worker;i<jobs.size();i+=workers) execute(work,jobs[i]);
      rendezvous.arrive_and_wait();
    });
    for(auto _:state) {
      (void)_;
      if(parallel) { rendezvous.arrive_and_wait(); rendezvous.arrive_and_wait(); }
      else if(serial) {
        // The next address depends on the returned rank. Correct outputs visit
        // each fresh input exactly once. This prevents independent-call overlap
        // being mislabeled scalar latency. This dependency overhead is included.
        std::size_t cursor=0;
        for(std::size_t i=0;i<jobs.size();++i) {
          auto& j=jobs[cursor]; execute(p,j);
          cursor=(cursor+1+static_cast<std::size_t>(j.out[0]^j.expected[0]))%jobs.size();
        }
        benchmark::DoNotOptimize(cursor);
      } else for(auto& j:jobs) execute(p,j);
      benchmark::ClobberMemory();
    }
    for(auto& thread:threads) thread.join();
    for(std::size_t i=0;i<jobs.size();++i) {
      if(state_operation(p)) {
        auto& j=jobs[i];
        const std::size_t offset=p.kind==Kind::extend_high||p.kind==Kind::extend_omaha?1:0;
        poker_rank rank=0;
        if(j.out[0]!=POKER_OK || poker_complete(j.destination->data,is_high(p)?j.h.data()+offset:nullptr,
            is_high(p)?nullptr:j.h.data()+offset,&rank)!=POKER_OK) {
          failed=true; state.SkipWithError("state operation failed"); return;
        }
        j.out[0]=rank;
      }
      if(jobs[i].out!=jobs[i].expected) { failed=true; state.SkipWithError("timed output mismatch or missing output"); return; }
      if(jobs[i].h!=input_h[i]||jobs[i].b!=input_b[i]) { failed=true; state.SkipWithError("input buffer modified"); return; }
    }
    state.counters["verified_items"]=static_cast<double>(jobs_count*work.batch);
    state.counters["verified_batches"]=static_cast<double>(jobs_count);
    state.SetItemsProcessed(static_cast<std::int64_t>(jobs_count*work.batch));
  } catch(const std::exception& e) { failed=true; state.SkipWithError(e.what()); }
}
} // namespace

int main(int argc,char** argv) {
  try {
    std::vector<char*> args{argv[0]}; bool have_seed=false;
    for(int i=1;i<argc;++i) {
      const std::string arg=argv[i];
      if(arg.rfind("--seed=",0)==0) { corpus_seed=std::stoull(arg.substr(7)); have_seed=true; }
      else if(arg.rfind("--records=",0)==0) records=std::stoull(arg.substr(10));
      else if(arg=="--smoke") smoke=true;
      else args.push_back(argv[i]);
    }
    if(!have_seed||records==0||records>1000000) throw std::runtime_error("provide --seed=UINT64 and 1..1000000 records");
    oracle::initialize();
    const auto start=std::chrono::steady_clock::now();
    if(poker_initialize("portable",budget)!=POKER_OK) throw std::runtime_error("backend initialization failed");
    const auto init_ns=std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now()-start).count();
    int count=static_cast<int>(args.size()); args.push_back(nullptr);
    benchmark::Initialize(&count,args.data());
    if(benchmark::ReportUnrecognizedArguments(count,args.data())) return 2;
    benchmark::AddCustomContext("poker_protocol","1");
    benchmark::AddCustomContext("poker_seed",std::to_string(corpus_seed));
    benchmark::AddCustomContext("poker_records",std::to_string(smoke?8:records));
    benchmark::AddCustomContext("poker_backend",poker_backend()->name);
    benchmark::AddCustomContext("poker_shared_bytes",std::to_string(poker_backend()->shared_bytes));
    benchmark::AddCustomContext("poker_state_bytes",std::to_string(poker_backend()->state_bytes));
    benchmark::AddCustomContext("poker_init_ns",std::to_string(init_ns));
    benchmark::AddCustomContext("poker_smoke",smoke?"true":"false");
    for(const auto& p:profiles()) benchmark::RegisterBenchmark(p.name.c_str(),[p](benchmark::State& s){measure(s,p);})->Iterations(1)->UseRealTime();
    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    return failed?1:0;
  } catch(const std::exception& e) { std::cerr<<e.what()<<'\n'; return 2; }
}
