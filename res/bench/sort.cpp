// #include "mkn/kul/log.hpp"

#include <thread>
#include <random>
#include <cstdlib>

#include "mkn/kul/sort.hpp"

#if __has_include("benchmark/benchmark.h")
#include "benchmark/benchmark.h"
#else
#include "benchmark/benchmark_api.h"
#endif

namespace mkn::kul {

static std::vector<std::int64_t> SIZES{/*100000,*/ 5000000};

template <typename T, bool std = true>
struct SortFixture : public benchmark::Fixture {
 public:
  void SetUp(::benchmark::State const& state) override {}

  void TearDown(::benchmark::State const& /*state*/) override {}

  void solve(::benchmark::State& state) {
    std::uint32_t size = state.range(0);

    auto gen = [&]() {
      std::random_device rd;
      std::seed_seq seed_seq{rd(), rd(), rd(), rd(), rd(), rd(), rd()};
      return std::mt19937_64(seed_seq);
    }();

    std::vector<T> vec(size);

    std::uniform_real_distribution<T> dist{0, 10000};
    for (std::size_t i = 0; i < size; ++i) vec[i] = dist(gen);

    for (auto _ : state)
      if constexpr (std)
        std::sort(vec.begin(), vec.end());
      else
        mkn::kul::sort(vec);
  }
};

BENCHMARK_TEMPLATE_DEFINE_F(SortFixture, std_double, double, true)(benchmark::State& state) {
  solve(state);
}
BENCHMARK_REGISTER_F(SortFixture, std_double)
    ->Unit(benchmark::kNanosecond)
    ->Threads(1)
    ->ArgsProduct({SIZES});

BENCHMARK_TEMPLATE_DEFINE_F(SortFixture, _double, double, false)(benchmark::State& state) {
  solve(state);
}
BENCHMARK_REGISTER_F(SortFixture, _double)
    ->Unit(benchmark::kNanosecond)
    ->Threads(1)
    ->ArgsProduct({SIZES});

}  // namespace mkn::kul

int main(int argc, char** argv) {
  ::benchmark::Initialize(&argc, argv);
  ::benchmark::RunSpecifiedBenchmarks();
}