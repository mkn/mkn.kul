#include "test_common.hpp"

#include "mkn/kul/lang/cpp/compiler/def/warning.hpp"

// -Werror would fail on the unused variable if the warning were not off
// clang-format off
MKN_KUL_DISABLE_WARNING(unused-variable, unused-variable, 4189)
// clang-format on
static int unused_variable_allowed() {
  int unused = 0;
  return 1;
}
// clang-format off
MKN_KUL_ENABLE_WARNING(unused-variable, unused-variable, 4189)
// clang-format on

TEST(Warning, disable_enable) { EXPECT_EQ(unused_variable_allowed(), 1); }
