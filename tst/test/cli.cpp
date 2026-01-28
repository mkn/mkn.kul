

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

TEST(CLI_Test, ParseCommandPythonExample) {
  std::vector<std::string> v =
      mkn::kul::cli::asArgs("python3 -c \"import sys; print('lol ' + dir(sys)\"");
  ASSERT_EQ(3ull, v.size());
  EXPECT_EQ("python3", v[0]);
  EXPECT_EQ("-c", v[1]);
  EXPECT_EQ("import sys; print('lol ' + dir(sys)", v[2]);
}
