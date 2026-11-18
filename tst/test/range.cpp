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

#include <array>
#include <string>
#include <utility>

TEST(Enumerate, lvalue) {
  std::vector<std::string> v{"a", "b", "c"};
  std::string joined;
  std::size_t n = 0;
  for (auto&& [i, o] : mkn::kul::enumerate(v)) {
    static_assert(std::is_same_v<decltype(i), std::size_t>);
    static_assert(std::is_same_v<decltype(o), std::string const&>);  // read only
    EXPECT_EQ(i, n++);
    joined += o + std::to_string(i);
  }
  EXPECT_EQ(joined, "a0b1c2");
}

TEST(Enumerate, constAndIndexType) {
  std::array<int, 3> const a{4, 5, 6};
  int sum = 0;
  for (auto const& [i, o] : mkn::kul::enumerate<int>(a)) {
    static_assert(std::is_same_v<std::remove_cvref_t<decltype(i)>, int>);
    static_assert(std::is_same_v<decltype(o), int const&>);
    sum += i * o;
  }
  EXPECT_EQ(sum, 0 * 4 + 1 * 5 + 2 * 6);
}

TEST(Enumerate, temporary) {
  std::size_t n = 0;
  for (auto&& [i, o] : mkn::kul::enumerate(std::vector<int>{7, 8})) {
    EXPECT_EQ(o, int(7 + i));
    ++n;
  }
  EXPECT_EQ(n, 2u);
  for ([[maybe_unused]] auto&& [i, o] : mkn::kul::enumerate(std::vector<int>{})) ++n;
  EXPECT_EQ(n, 2u);
}

static_assert([] {
  std::array<int, 3> const a{1, 2, 3};
  int sum = 0;
  for (auto&& [i, o] : mkn::kul::enumerate(a)) sum += o * static_cast<int>(i);
  return sum == 0 * 1 + 1 * 2 + 2 * 3;
}());
