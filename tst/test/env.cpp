

#include "test_common.hpp"

#include "mkn/kul/env.hpp"

TEST(PushEnvOperations, SetsThenRestoresPreviouslyUnset) {
  mkn::kul::env::SET("MKN_KUL_TEST_ENV_A", nullptr);
  ASSERT_FALSE(mkn::kul::env::EXISTS("MKN_KUL_TEST_ENV_A"));
  {
    mkn::kul::env::PushEnv push("MKN_KUL_TEST_ENV_A", "1");
    EXPECT_EQ("1", mkn::kul::env::GET("MKN_KUL_TEST_ENV_A"));
  }
  EXPECT_FALSE(mkn::kul::env::EXISTS("MKN_KUL_TEST_ENV_A"));
}

TEST(PushEnvOperations, SetsThenRestoresPreviousValue) {
  mkn::kul::env::SET("MKN_KUL_TEST_ENV_B", "orig");
  {
    mkn::kul::env::PushEnv push("MKN_KUL_TEST_ENV_B", "1");
    EXPECT_EQ("1", mkn::kul::env::GET("MKN_KUL_TEST_ENV_B"));
  }
  EXPECT_EQ("orig", mkn::kul::env::GET("MKN_KUL_TEST_ENV_B"));
  mkn::kul::env::SET("MKN_KUL_TEST_ENV_B", nullptr);
}

TEST(PushEnvOperations, UnsetsThenRestores) {
  mkn::kul::env::SET("MKN_KUL_TEST_ENV_C", "orig");
  {
    mkn::kul::env::PushEnv push(std::string("MKN_KUL_TEST_ENV_C"));
    EXPECT_FALSE(mkn::kul::env::EXISTS("MKN_KUL_TEST_ENV_C"));
  }
  EXPECT_EQ("orig", mkn::kul::env::GET("MKN_KUL_TEST_ENV_C"));
  mkn::kul::env::SET("MKN_KUL_TEST_ENV_C", nullptr);
}

TEST(PushEnvOperations, FromVarPrependsThenRestores) {
  mkn::kul::env::SET("MKN_KUL_TEST_ENV_D", "existing");
  {
    mkn::kul::env::Var var("MKN_KUL_TEST_ENV_D", "new", mkn::kul::env::Var::Mode::PREP);
    mkn::kul::env::PushEnv push(var);
    std::string const sep(1, mkn::kul::env::SEP());
    EXPECT_EQ("new" + sep + "existing", mkn::kul::env::GET("MKN_KUL_TEST_ENV_D"));
  }
  EXPECT_EQ("existing", mkn::kul::env::GET("MKN_KUL_TEST_ENV_D"));
  mkn::kul::env::SET("MKN_KUL_TEST_ENV_D", nullptr);
}

TEST(PushEnvOperations, FromVectorOfVarsRestoresEach) {
  mkn::kul::env::SET("MKN_KUL_TEST_ENV_E", nullptr);
  mkn::kul::env::SET("MKN_KUL_TEST_ENV_F", "orig_f");
  {
    std::vector<mkn::kul::env::Var> vars{
        {"MKN_KUL_TEST_ENV_E", "e", mkn::kul::env::Var::Mode::REPL},
        {"MKN_KUL_TEST_ENV_F", "f", mkn::kul::env::Var::Mode::REPL}};
    mkn::kul::env::PushEnv push(vars);
    EXPECT_EQ("e", mkn::kul::env::GET("MKN_KUL_TEST_ENV_E"));
    EXPECT_EQ("f", mkn::kul::env::GET("MKN_KUL_TEST_ENV_F"));
  }
  EXPECT_FALSE(mkn::kul::env::EXISTS("MKN_KUL_TEST_ENV_E"));
  EXPECT_EQ("orig_f", mkn::kul::env::GET("MKN_KUL_TEST_ENV_F"));
  mkn::kul::env::SET("MKN_KUL_TEST_ENV_F", nullptr);
}

TEST(PushEnvOperations, FromVectorOfPairsRestoresEach) {
  mkn::kul::env::SET("MKN_KUL_TEST_ENV_G", nullptr);
  {
    std::vector<std::pair<std::string, std::string>> vars{{"MKN_KUL_TEST_ENV_G", "g"}};
    mkn::kul::env::PushEnv push(vars);
    EXPECT_EQ("g", mkn::kul::env::GET("MKN_KUL_TEST_ENV_G"));
  }
  EXPECT_FALSE(mkn::kul::env::EXISTS("MKN_KUL_TEST_ENV_G"));
}

TEST(PushEnvOperations, FromVectorOfNamesUnsetsEach) {
  mkn::kul::env::SET("MKN_KUL_TEST_ENV_H", "h");
  mkn::kul::env::SET("MKN_KUL_TEST_ENV_I", "i");
  {
    mkn::kul::env::PushEnv push(std::vector<std::string>{"MKN_KUL_TEST_ENV_H", "MKN_KUL_TEST_ENV_I"});
    EXPECT_FALSE(mkn::kul::env::EXISTS("MKN_KUL_TEST_ENV_H"));
    EXPECT_FALSE(mkn::kul::env::EXISTS("MKN_KUL_TEST_ENV_I"));
  }
  EXPECT_EQ("h", mkn::kul::env::GET("MKN_KUL_TEST_ENV_H"));
  EXPECT_EQ("i", mkn::kul::env::GET("MKN_KUL_TEST_ENV_I"));
  mkn::kul::env::SET("MKN_KUL_TEST_ENV_H", nullptr);
  mkn::kul::env::SET("MKN_KUL_TEST_ENV_I", nullptr);
}

TEST(PushEnvOperations, MoveTransfersRestoreResponsibility) {
  mkn::kul::env::SET("MKN_KUL_TEST_ENV_J", "orig");
  {
    mkn::kul::env::PushEnv outer("MKN_KUL_TEST_ENV_J", "1");
    {
      mkn::kul::env::PushEnv inner(std::move(outer));
      EXPECT_EQ("1", mkn::kul::env::GET("MKN_KUL_TEST_ENV_J"));
    }
    EXPECT_EQ("orig", mkn::kul::env::GET("MKN_KUL_TEST_ENV_J"));
  }
  EXPECT_EQ("orig", mkn::kul::env::GET("MKN_KUL_TEST_ENV_J"));
  mkn::kul::env::SET("MKN_KUL_TEST_ENV_J", nullptr);
}
