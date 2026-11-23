#include "test_common.hpp"

#include "mkn/kul/lang/cpp/compiler/def/inline.hpp"

namespace {
MKN_KUL_ALWAYS_INLINE int twice(int const i) { return 2 * i; }
}  // namespace

// each directive where it goes: a function, a lambda, a call
TEST(Inline, directivesCompileWhereTheyGo) {
  auto const add = [](int const a, int const b) MKN_KUL_LAMBDA_ALWAYS_INLINE { return a + b; };
  int sum = 0;
  MKN_KUL_INLINE_CALL sum = add(twice(1), 3);
  EXPECT_EQ(sum, 5);
}
