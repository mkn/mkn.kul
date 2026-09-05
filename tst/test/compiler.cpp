

#include "test_common.hpp"

#include "mkn/kul/lang/cpp/compilers.hpp"

namespace {
using mkn::kul::lang::cpp::CCompiler;
using mkn::kul::lang::cpp::Compilers;
}  // namespace

TEST(CompilerTests, ClangResolvesToClangFamily) {
  EXPECT_EQ("clang", Compilers::INSTANCE().base("clang"));
  EXPECT_EQ("clang++", Compilers::INSTANCE().base("clang++"));
}

// clang on Windows targets the MSVC ABI: its driver resolves "-lfoo" by
// searching for "foo.lib" (COFF/MSVC convention), not "libfoo.a" (GNU
// convention) - the archive mkn produces for it must match, or linking fails
// even though the archive itself built fine.
TEST(CompilerTests, ClangStaticLibNameMatchesPlatformLinkConvention) {
  auto const* cc = dynamic_cast<CCompiler const*>(Compilers::INSTANCE().get("clang++"));
  ASSERT_TRUE(cc);
#ifdef _WIN32
  EXPECT_EQ("parse.yaml.lib", cc->staticLib("parse.yaml"));
#else
  EXPECT_EQ("libparse.yaml.a", cc->staticLib("parse.yaml"));
#endif
}

TEST(CompilerTests, GccStaticLibNameIsAlwaysGnuConvention) {
  auto const* cc = dynamic_cast<CCompiler const*>(Compilers::INSTANCE().get("g++"));
  ASSERT_TRUE(cc);
  EXPECT_EQ("libparse.yaml.a", cc->staticLib("parse.yaml"));
}
