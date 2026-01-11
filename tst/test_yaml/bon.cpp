

#include "gtest/gtest.h"

#include "mkn/kul/bon.hpp"

TEST(Bon, parseString) {
  auto node = mkn::kul::bon::from("{rofl:{copter}}");

  for (auto const& el : node) {
    EXPECT_EQ(el.first.as<std::string>(), "rofl");
    EXPECT_EQ(el.second.as<std::string>(), "copter");
  }
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
