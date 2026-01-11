

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
