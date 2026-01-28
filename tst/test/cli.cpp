

#include "test_common.hpp"

#include "mkn/kul/cli.hpp"

TEST(CLI_Test, ParseCommandLineArguments) {
  std::vector<std::string> v;
  mkn::kul::cli::asArgs("/path/to \"words 'in' quotes\" words\\ not\\ in\\ quotes end", v);
  ASSERT_EQ(4ull, v.size());
  EXPECT_EQ("/path/to", v[0]);
  EXPECT_EQ("words 'in' quotes", v[1]);
  EXPECT_EQ("words not in quotes", v[2]);
  EXPECT_EQ("end", v[3]);
}
