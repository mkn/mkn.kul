

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

TEST(CLI_Test, ParseWeirdPythonExample) {
  std::vector<std::string> v = mkn::kul::cli::asArgs("python3 -c lol\"import sys\"lol");
  ASSERT_EQ(3ull, v.size());
  EXPECT_EQ("python3", v[0]);
  EXPECT_EQ("-c", v[1]);
  EXPECT_EQ("lolimport syslol", v[2]);
}

namespace {
mkn::kul::cli::Args count_args() {
  using namespace mkn::kul::cli;
  return Args({Cmd("build")}, {Arg('v', "verbose", ArgType::FLAG),       //
                               Arg('d', "dependencies", ArgType::FLAG),  //
                               Arg('a', "args", ArgType::STRING),        //
                               Arg('O', "optimise", ArgType::MAYBE)});
}

mkn::kul::cli::Args process(std::vector<std::string> in) {
  in.insert(in.begin(), "mkn");
  std::vector<char*> argv;
  for (auto& s : in) argv.push_back(s.data());
  auto args = count_args();
  args.process(argv.size(), argv.data());
  return args;
}
}  // namespace

TEST(CLI_Test, ArgCountAbsent) {
  auto const args = process({"build"});
  EXPECT_FALSE(args.has("verbose"));
  EXPECT_EQ(0ull, args.count("verbose"));
}

TEST(CLI_Test, ArgCountSingle) {
  auto const args = process({"-v"});
  EXPECT_TRUE(args.has("verbose"));
  EXPECT_EQ(1ull, args.count("verbose"));
}

TEST(CLI_Test, ArgCountCombined) {
  auto const args = process({"-vvv"});
  EXPECT_TRUE(args.has("verbose"));
  EXPECT_EQ(3ull, args.count("verbose"));
}

TEST(CLI_Test, ArgCountCombinedWithOtherFlags) {
  auto const args = process({"-dvdv", "build"});
  EXPECT_EQ(2ull, args.count("verbose"));
  EXPECT_EQ(2ull, args.count("dependencies"));
  EXPECT_TRUE(args.has("build"));
}

TEST(CLI_Test, ArgCountSeparateAndLong) {
  auto const args = process({"-v", "--verbose", "-vv"});
  EXPECT_EQ(4ull, args.count("verbose"));
}

TEST(CLI_Test, ArgCountWithValues) {
  auto const args = process({"-a", "one", "--args", "two", "-a=three", "--args=four"});
  EXPECT_EQ(4ull, args.count("args"));
  EXPECT_EQ("four", args.get("args"));  // last value wins
}

TEST(CLI_Test, ArgCountCombinedEndingInValue) {
  auto const args = process({"-vva", "x"});
  EXPECT_EQ(2ull, args.count("verbose"));
  EXPECT_EQ(1ull, args.count("args"));
  EXPECT_EQ("x", args.get("args"));
}

TEST(CLI_Test, ArgCountMaybe) {
  auto const args = process({"-O", "-O", "3"});
  EXPECT_EQ(2ull, args.count("optimise"));
  EXPECT_EQ("3", args.get("optimise"));
}

TEST(CLI_Test, ArgCountErase) {
  auto args = process({"-vv"});
  EXPECT_TRUE(args.erase("verbose"));
  EXPECT_FALSE(args.has("verbose"));
  EXPECT_EQ(0ull, args.count("verbose"));
}
