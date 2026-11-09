

#include "test_common.hpp"

#include "mkn/kul/lang/cpp/compilers.hpp"
#include "mkn/kul/lang/cpp/gcc.hpp"
#include "mkn/kul/lang/compiler.hpp"
#include "mkn/kul/lang/def.hpp"

#include <exception>
#include <string>
#include <vector>

namespace {
using mkn::kul::lang::CompilationInfo;
using mkn::kul::lang::Mode;
using mkn::kul::lang::cpp::CCompiler;
using mkn::kul::lang::cpp::Compilers;
using mkn::kul::lang::cpp::GccCompiler;

struct StubCompilerState : mkn::kul::lang::CompilerState {
  void add(mkn::kul::lang::AbstractCompilerInput const&) override {}
};

struct StubContext : mkn::kul::lang::Context {
  mkn::kul::lang::ContextState s;
  StubCompilerState cs;
  mkn::kul::lang::ContextState state() const override { return s; }
  mkn::kul::lang::CompilerState& compilerState() override { return cs; }
  void per_compiler_command(CompileHook) override {}
  std::string compileCommandFor(std::string const&) const override { return ""; }
};

CompilationInfo info(std::string const& prefix, std::string const& ext) {
  CompilationInfo i;
  i.lib_prefix = prefix;
  i.lib_ext = ext;
  i.obj_ext = "o";
  return i;
}

GccCompiler const& clang() {
  auto const* cc = dynamic_cast<GccCompiler const*>(Compilers::INSTANCE().get("clang++"));
  EXPECT_TRUE(cc);
  return *cc;
}

// dry run, returns the command that would be run
std::string link(CompilationInfo const& ci, Mode const mode, std::vector<std::string> const& libs,
                 bool const lib) {
  StubContext ctx;
  ctx.s.compileEnv.compilationInfo = ci;
  std::string const linker = "clang++", linkerEnd, out = "bin/test_link/out";
  std::vector<mkn::kul::Dir> stars;
  std::vector<std::string> objects, libPaths;
  mkn::kul::lang::LinkDAO dao{ctx, linker, linkerEnd, out, stars, objects, libs, libPaths, mode,
                              true};
  auto const pc = lib ? clang().buildLibrary(dao) : clang().buildExecutable(dao);
  if (pc.exception()) std::rethrow_exception(pc.exception());
  return pc.cmd();
}
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

// -l<name> finds lib<name>.so/.dylib only, except on Windows where it is the
// import lib <name>.lib whatever the dll is named.
TEST(CompilerTests, LinkableByName) {
  EXPECT_TRUE(GccCompiler::linkableByName(info("lib", ".so")));
  EXPECT_TRUE(GccCompiler::linkableByName(info("lib", ".dylib")));
#ifdef _WIN32
  EXPECT_TRUE(GccCompiler::linkableByName(info("", ".dll")));
  EXPECT_TRUE(GccCompiler::linkableByName(info("", ".cp313-win_amd64.pyd")));
#else
  EXPECT_FALSE(GccCompiler::linkableByName(info("", ".cpython-313-x86_64-linux-gnu.so")));
  EXPECT_FALSE(GccCompiler::linkableByName(info("lib", ".so.1")));
#endif
}

// A soname only on ELF and only for libs -l can't find (they are linked by
// path), lld-link on Windows takes "-soname" args as input files.
TEST(CompilerTests, SharedLibSoname) {
  EXPECT_EQ(std::string::npos, link(info("lib", ".so"), Mode::SHAR, {}, true).find("soname"));
#ifdef _WIN32
  EXPECT_EQ(std::string::npos, link(info("", ".dll"), Mode::SHAR, {}, true).find("soname"));
#elif !defined(__APPLE__)
  auto const cmd = link(info("", ".cpython-test.so"), Mode::SHAR, {}, true);
  EXPECT_NE(std::string::npos, cmd.find("-Wl,-soname,out.cpython-test.so")) << cmd;
#endif
}

TEST(CompilerTests, LinkLibsByPathOrName) {
  auto const cmd = link(info("lib", ".so"), Mode::SHAR, {"foo", "dir/bar.cpython-test.so"}, false);
  EXPECT_NE(std::string::npos, cmd.find("-lfoo")) << cmd;
  EXPECT_NE(std::string::npos, cmd.find("bar.cpython-test.so")) << cmd;
  EXPECT_EQ(std::string::npos, cmd.find("-ldir")) << cmd;
}
