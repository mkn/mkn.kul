

#include "test_common.hpp"

#include "mkn/kul/for.hpp"

#include <vector>

TEST(For_Test, TestConst) {
  std::vector<int> const vec{1, 2, 3};
  auto const ret = mkn::kul::generate_from([](auto& e) { return e + 1; }, vec);

  EXPECT_EQ(ret[2], 4);
}
TEST(For_Test, TestNonConst) {
  std::vector<int> vec{1, 2, 3};
  auto const ret = mkn::kul::generate_from(
      [](auto& e) {
        e += 1;
        return e;
      },
      vec);

  EXPECT_EQ(ret[2], 4);
  EXPECT_EQ(vec[2], 4);
}

#include "mkn/kul/lang/cpp/compiler/def/unroll_loop.hpp"

#include <array>
#include <tuple>
#include <cstdint>
#include <type_traits>

TEST(For_Test, for_N_index_type) {
  mkn::kul::for_N<1>([](auto ic) {
    static_assert(std::is_same_v<decltype(ic), std::integral_constant<std::uint16_t, 0>>);
  });
}

TEST(For_Test, for_N_fold) {
  std::array<int, 3> const idx{10, 20, 30};
  auto const sum = mkn::kul::for_N_fold<3>([&](auto... ics) { return (idx[ics] + ...); });
  EXPECT_EQ(sum, 60);
  static_assert(mkn::kul::for_N_fold<4>([](auto... ics) { return sizeof...(ics); }) == 4);
}

TEST(For_Test, for_N_modes) {
  auto const arr = mkn::kul::for_N_make_array<3>([](auto ic) { return ic() * 2; });
  static_assert(std::is_same_v<decltype(arr), std::array<int, 3> const>);
  EXPECT_EQ(arr, (std::array<int, 3>{0, 2, 4}));

  auto const tup = mkn::kul::for_N<2>([](auto ic) {
    if constexpr (ic() == 0)
      return 1;
    else
      return 2.5;
  });
  EXPECT_EQ(std::get<0>(tup), 1);
  EXPECT_EQ(std::get<1>(tup), 2.5);

  std::array<int, 2> vals{1, 2};
  auto refs = mkn::kul::for_N<2, mkn::kul::for_N_R_mode::forward_tuple>(
      [&](auto ic) -> int& { return vals[ic]; });
  std::get<1>(refs) = 5;
  EXPECT_EQ(vals[1], 5);

  int n = 0;
  mkn::kul::for_N<4>([&](auto) { ++n; });
  EXPECT_EQ(n, 4);

  EXPECT_TRUE(mkn::kul::for_N_all<3>([](auto ic) { return ic() < 3; }));
  EXPECT_FALSE(mkn::kul::for_N_all<3>([](auto ic) { return ic() < 2; }));
  EXPECT_TRUE(mkn::kul::for_N_any<3>([](auto ic) { return ic() == 2; }));
  EXPECT_FALSE(mkn::kul::for_N_any<3>([](auto ic) { return ic() == 3; }));
}

TEST(For_Test, unroll_loop) {
  int sum = 0;
  MKN_KUL_UNROLL_LOOP(4)
  for (int i = 0; i < 8; ++i) sum += i;
  EXPECT_EQ(sum, 28);
}

TEST(For_Test, all_any_none) {
  std::array<int, 3> const a{1, 2, 3};
  static_assert(mkn::kul::all(std::array{true, true}) && !mkn::kul::all(std::array{true, false}));
  static_assert(mkn::kul::any(std::array{false, true}) && !mkn::kul::any(std::array{false, false}));
  static_assert(mkn::kul::none(std::array{false, false}) && !mkn::kul::none(std::array{true}));
  EXPECT_TRUE(mkn::kul::all(a, [](auto e) { return e > 0; }));
  EXPECT_FALSE(mkn::kul::any(a, [](auto e) { return e > 3; }));

  int calls = 0;  // stops at the first element that decides it
  EXPECT_FALSE(mkn::kul::all(a, [&](auto e) { return ++calls, e > 1; }));
  EXPECT_EQ(calls, 1);
  calls = 0;
  EXPECT_TRUE(mkn::kul::any(a, [&](auto e) { return ++calls, e == 2; }));
  EXPECT_EQ(calls, 2);
  EXPECT_TRUE(mkn::kul::all(std::array<bool, 0>{}));
  EXPECT_FALSE(mkn::kul::any(std::array<bool, 0>{}));
}
