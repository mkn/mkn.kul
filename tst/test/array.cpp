#include "test_common.hpp"

#include "mkn/kul/std/array.hpp"
#include "mkn/kul/std/vector.hpp"
#include "mkn/kul/random.hpp"
#include "mkn/kul/throw.hpp"

#include <array>
#include <vector>
#include <cstdint>
#include <stdexcept>

static_assert(mkn::kul::ConstArray<int, 3>(2) == std::array{2, 2, 2});
static_assert(mkn::kul::sized_array<2>(std::array{1, 2, 3}) == std::array{1, 2});
static_assert(mkn::kul::as_sized_array<1>(4, 5) == std::array{4});
static_assert(mkn::kul::array_equals(std::array{1, 2}, std::array{1, 2}));
static_assert(mkn::kul::array_minus<int>(std::array{3u, 5u}, std::array{4u, 1u}) ==
              std::array{-1, 4});

TEST(Array, ops) {
  std::array<int, 3> a{};
  mkn::kul::fill(4, a);
  EXPECT_EQ(a, (std::array{4, 4, 4}));
  mkn::kul::array_op<mkn::kul::PlusEquals<int>>(a, std::array{1, 2, 3});
  EXPECT_EQ(a, (std::array{5, 6, 7}));
  mkn::kul::array_op<mkn::kul::SetMax<int>>(a, 6);
  EXPECT_EQ(a, (std::array{6, 6, 7}));
  mkn::kul::array_op<mkn::kul::SetEqual<int>>(a, 1);
  EXPECT_EQ(a, (std::array{1, 1, 1}));

  int n = 0;
  EXPECT_EQ(mkn::kul::ConstArrayFrom<3>([&] { return n++; }), (std::array{0, 1, 2}));
  EXPECT_EQ(mkn::kul::displacementFrom(std::vector{2, 3, 1}), (std::vector{0, 2, 5}));

  std::array<int, 2> m{1, 2};
  auto const doubled = mkn::kul::generate_from([](int& e) { return e *= 2; }, m);
  EXPECT_EQ(doubled, (std::array{2, 4}));
  EXPECT_EQ(m, (std::array{2, 4}));
}

TEST(Random, splitmix) {
  // S. Vigna's splitmix64.c from seed 0
  mkn::kul::SplitMix64 rng{0};
  EXPECT_EQ(rng(), 0xe220a8397b1dcdafULL);
  EXPECT_EQ(rng(), 0x6e789e6aa1b965f4ULL);
  static_assert(mkn::kul::splitmix64_mix(0) == 0);
  static_assert(mkn::kul::hash_combine(3, 7) != mkn::kul::hash_combine(7, 3));
  static_assert(mkn::kul::fnv1a64("") == 0xcbf29ce484222325ULL);
  static_assert(mkn::kul::fnv1a64("a") == 0xaf63dc4c8601ec8cULL);
}

TEST(Throw, runtime_error) { EXPECT_THROW(mkn::kul::throw_runtime_error("x"), std::runtime_error); }
