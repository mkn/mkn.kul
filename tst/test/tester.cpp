
#include "test_common.hpp"

#include "mkn/kul/test.hpp"

#include <memory>
#include <string>

TEST(Tester, RecordsChecks) {
  mkn::kul::Tester tester{false};
  EXPECT_TRUE(tester.check(true, "passes"));
  EXPECT_FALSE(tester.check(false, "fails"));
  EXPECT_EQ(tester.size(), 2u);
  EXPECT_EQ(tester.failed(), 1u);
}

TEST(Tester, MacroChecksTheDeclaredTester) {
  mkn::kul::Tester _mkn_kul_tester{false};  // as MKN_KUL_TEST_DECLARE, without exiting
  std::unique_ptr<int> const none, one = std::make_unique<int>(1);
  EXPECT_TRUE(MKN_KUL_TEST_CHECK(one));  // explicit bool conversions
  EXPECT_FALSE(MKN_KUL_TEST_CHECK(none, "no pointer"));
  EXPECT_FALSE(MKN_KUL_TEST_CHECK(1 + 1 == 3));
  EXPECT_EQ(_mkn_kul_tester.failed(), 2u);
}

TEST(Tester, ExitsOneOnFailure) {
  EXPECT_EXIT(
      {
        mkn::kul::Tester tester;
        tester.check(false, "fails");
      },
      ::testing::ExitedWithCode(1), "1 checks, 0 passed, 1 failed");
}
