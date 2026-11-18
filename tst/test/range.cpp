#include "test_common.hpp"

#include "mkn/kul/range.hpp"

#include <ranges>
#include <vector>
#include <type_traits>

static_assert(std::ranges::forward_range<mkn::kul::Range<std::size_t>>);
static_assert(std::is_same_v<decltype(*mkn::kul::range(10).begin()), std::size_t>);
static_assert(std::is_same_v<decltype(*mkn::kul::range<int>(10).begin()), int>);
static_assert(mkn::kul::range(10).size() == 10);
static_assert(mkn::kul::range(5, 2).empty());

TEST(Range, stop) {
  std::vector<std::size_t> v;
  for (auto const i : mkn::kul::range(10)) v.push_back(i);
  EXPECT_EQ(v, (std::vector<std::size_t>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9}));
}

TEST(Range, startStop) {
  std::vector<int> v;
  for (auto const i : mkn::kul::range<int>(-2, 3)) v.push_back(i);
  EXPECT_EQ(v, (std::vector<int>{-2, -1, 0, 1, 2}));
}

TEST(Range, empty) {
  std::size_t n = 0;
  for ([[maybe_unused]] auto const i : mkn::kul::range(0)) ++n;
  for ([[maybe_unused]] auto const i : mkn::kul::range(5, 2)) ++n;
  for ([[maybe_unused]] auto const i : mkn::kul::range<int>(3, -3)) ++n;
  EXPECT_EQ(n, 0u);
}
