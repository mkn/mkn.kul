

#include "test_common.hpp"

#include "mkn/kul/zip.hpp"

#include <array>
#include <vector>
#include <numeric>

TEST(ZipOperations, works) {
  auto d0f = []() {
    return std::vector<double>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};  //   45
  };
  std::vector<double> const d1{10, 11, 12, 13, 14, 15, 16, 17, 18, 19};             //  145
  std::array<double, 10> const d2{0, 100, 200, 300, 400, 500, 600, 700, 800, 900};  // 4500 = 4690

  auto d0 = d0f();
  EXPECT_EQ(d0.size(), d1.size());
  EXPECT_EQ(d1.size(), d2.size());

  double result = 0;
  for (auto const& [a, b, c] : mkn::kul::zip(d0f(), d1, d2)) result += a + b + c;
  EXPECT_EQ(result, 4690);

  double expected = 0;
  for (auto const& v : d0) expected += v;
  for (auto const& v : d1) expected += v;
  for (auto const& v : d2) expected += v;

  EXPECT_EQ(result, expected);

  double acc = 0;
  acc += std::accumulate(d0.begin(), d0.end(), 0);
  acc += std::accumulate(d1.begin(), d1.end(), 0);
  acc += std::accumulate(d2.begin(), d2.end(), 0);
  EXPECT_EQ(result, acc);
}
