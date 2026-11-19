

#include "test_common.hpp"

#include "mkn/kul/cli.hpp"
#include "mkn/kul/lang/cpp/compilers.hpp"
#include "mkn/kul/lang/cpp/gcc.hpp"
#include "mkn/kul/lang/cpp/nvcc.hpp"
#include "mkn/kul/lang/compiler.hpp"
#include "mkn/kul/lang/def.hpp"
#include "mkn/kul/io.hpp"
#include "mkn/kul/os.hpp"
#include "mkn/kul/proc.hpp"

#include <cstddef>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <string>
#include <vector>

namespace {
using mkn::kul::lang::CompilationInfo;
using mkn::kul::lang::Mode;
using mkn::kul::lang::cpp::CCompiler;
using mkn::kul::lang::cpp::CCompiler_Type;
using mkn::kul::lang::cpp::Compilers;
using mkn::kul::lang::cpp::GccCompiler;
using mkn::kul::lang::cpp::NvccCompiler;

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

GccCompiler const* gcc_like(std::string const& comp) {
  return dynamic_cast<GccCompiler const*>(Compilers::INSTANCE().get(comp));
}

// dry run, returns the command that would be run
std::string link(CompilationInfo const& ci, Mode const mode, std::vector<std::string> const& libs,
                 bool const lib, std::string const& linker = "clang++") {
  StubContext ctx;
  ctx.s.compileEnv.compilationInfo = ci;
  std::string const linkerEnd, out = "bin/test_link/out";
  std::vector<std::filesystem::path> stars;
  std::vector<std::string> objects, libPaths;
  mkn::kul::lang::LinkDAO dao{ctx,     linker, linkerEnd, out,  stars,
                              objects, libs,   libPaths,  mode, true};
  auto const* cc = gcc_like(linker);
  EXPECT_TRUE(cc);
  auto const pc = lib ? cc->buildLibrary(dao) : cc->buildExecutable(dao);
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

TEST(CompilerTests, NvccIsItsOwnCompiler) {
#ifdef _WIN32  // compiler paths with backslashes are not identified, see cli::asArgs
  std::vector<std::string> const comps{"nvcc"};
#else
  std::vector<std::string> const comps{"nvcc", "/usr/local/cuda/bin/nvcc"};
#endif
  for (auto const& comp : comps) {
    auto const* cc = dynamic_cast<NvccCompiler const*>(Compilers::INSTANCE().get(comp));
    ASSERT_TRUE(cc) << comp;
    EXPECT_EQ(CCompiler_Type::NVCC, cc->type());
  }
}

// nvcc rejects gcc options, host compiler options go through -Xcompiler
TEST(CompilerTests, NvccFlagsAreNvccOptions) {
  auto const* cc = gcc_like("nvcc");
  ASSERT_TRUE(cc);
  for (std::uint8_t i = 0; i < 10; ++i) {
    std::vector<std::string> const flags_set{cc->compilerOptimization(i), cc->compilerWarning(i)};
    for (auto const& flags : flags_set) {
      auto const args = mkn::kul::cli::asArgs(flags);
      for (std::size_t a = 0; a < args.size(); ++a)
        for (auto const* const gcc_only : {"-funroll-loops", "-march", "-Wall", "-pedantic"})
          if (args[a].rfind(gcc_only, 0) == 0) {
            EXPECT_TRUE(a > 0 && args[a - 1] == "-Xcompiler") << int{i} << ": " << flags;
          }
    }
  }
  EXPECT_EQ("-O3 -Xcompiler -funroll-loops,-march=native,-mtune=native",
            cc->compilerOptimization(9));
  EXPECT_EQ("-Xcompiler -Wall,-Wextra,-Werror -Werror all-warnings", cc->compilerWarning(9));
}

// nvcc rejects -Wl, linker options go through -Xlinker
TEST(CompilerTests, NvccLinkerOptions) {
#if !defined(_WIN32) && !defined(__APPLE__)
  auto const cmd = link(info("", ".cpython-test.so"), Mode::SHAR, {}, true, "nvcc");
  EXPECT_NE(std::string::npos, cmd.find("-Xlinker=-soname,out.cpython-test.so")) << cmd;
  EXPECT_EQ(std::string::npos, cmd.find("-Wl,")) << cmd;
#endif
}

// a linker's output is captured, not written to the terminal: parallel links print it whole
TEST(CompilerTests, LinkOutputIsCaptured) {
#if !defined(_WIN32)
  StubContext ctx;
  ctx.s.compileEnv.compilationInfo = info("lib", ".so");
  std::string const linkerEnd = "--mkn-kul-unknown-option", out = "bin/test_link/captured";
  std::vector<std::filesystem::path> stars;
  std::vector<std::string> objects, libs, libPaths;
  mkn::kul::lang::LinkDAO dao{ctx,     "g++", linkerEnd, out,        stars,
                              objects, libs,  libPaths,  Mode::NONE, false};
  auto const* cc = gcc_like("g++");
  ASSERT_TRUE(cc);
  auto const pc = cc->buildExecutable(dao);
  EXPECT_TRUE(pc.exception());
  EXPECT_NE(std::string::npos, pc.errs().find("mkn-kul-unknown-option")) << pc.errs();
  EXPECT_NE(std::string::npos, pc.cmd().find("mkn-kul-unknown-option")) << pc.cmd();
#endif
}

// the object files of a directory, not a "*.o" only a shell would expand
TEST(CompilerTests, ObjectFilesOfDir) {
  mkn::kul::Dir const d("bin/test_link/objs");
  d.mk();
  for (auto const* const n : {"b.o", "a.o", "c.txt"})
    mkn::kul::io::Writer(mkn::kul::File(n, d).full()) << "";
  auto const objs = CCompiler::objectFiles(info("lib", ".so"), d.real());
  ASSERT_EQ(2u, objs.size());
  EXPECT_NE(std::string::npos, objs[0].find("a.o")) << objs[0];
  EXPECT_NE(std::string::npos, objs[1].find("b.o")) << objs[1];
  EXPECT_TRUE(CCompiler::objectFiles(info("lib", ".so"), "bin/test_link/none").empty());
}

// a link of a directory's objects, started without a shell
TEST(CompilerTests, LinkObjectsOfDir) {
#if !defined(_WIN32)
  mkn::kul::Dir const d("bin/test_link/main_objs");
  d.mk();
  mkn::kul::File const src("main.cpp", d);
  mkn::kul::io::Writer(src.full()) << "int main() { return 0; }\n";
  mkn::kul::Process c("g++");
  c << "-c" << src.escm() << "-o" << mkn::kul::File("main.o", d).escm();
  c.start();

  StubContext ctx;
  ctx.s.compileEnv.compilationInfo = info("lib", ".so");
  std::string const linkerEnd, out = "bin/test_link/main_objs_exe";
  std::vector<std::filesystem::path> stars{d.path()};
  std::vector<std::string> objects, libs, libPaths;
  mkn::kul::lang::LinkDAO dao{ctx,     "g++", linkerEnd, out,        stars,
                              objects, libs,  libPaths,  Mode::NONE, false};
  auto const* cc = gcc_like("g++");
  ASSERT_TRUE(cc);
  auto const pc = cc->buildExecutable(dao);
  EXPECT_FALSE(pc.exception()) << pc.errs();
  EXPECT_TRUE(mkn::kul::File(out));
#endif
}

// a level's flags as named parts, joined into the same string as before
TEST(CompilerTests, FlagParts) {
  using mkn::kul::lang::Flags;
  auto const names = [](Flags const& f) {
    std::string s;
    for (auto const& p : f.parts()) s += p.name + ";";
    return s;
  };

  auto const* gcc = gcc_like("g++");
  ASSERT_TRUE(gcc);
  auto const o9 = gcc->compilerOptimizationFlags(9);
  EXPECT_EQ(o9.str(), gcc->compilerOptimization(9));
#if defined(__linux__)
  EXPECT_EQ("optimise;arch;lto;", names(o9));
  EXPECT_EQ("-O3 -funroll-loops -march=native -mtune=native -flto=auto", o9.str());
  EXPECT_EQ("-flto=auto", gcc->linkerOptimizationBin(9));
  EXPECT_EQ("lto;", names(gcc->linkerOptimizationLibFlags(9)));
#endif
  EXPECT_EQ("-O3 -funroll-loops -march=native -mtune=native", o9.without("lto").str());
  // a plain string level is one part, named after its kind
  EXPECT_EQ("optimise;", names(gcc->compilerOptimizationFlags(2)));
  EXPECT_EQ("warn;", names(gcc->compilerWarningFlags(9)));
  EXPECT_EQ("", names(gcc->compilerOptimizationFlags(0)));  // "", no parts

  Flags f = o9;
  f.set("optimise", "-O2").set("lto", "").set("extra", "-fno-strict-aliasing");
  EXPECT_EQ("-O2 -march=native -mtune=native -fno-strict-aliasing", f.str());
  EXPECT_EQ("optimise;arch;extra;", names(f));
}

// a Context that can't leave flag parts out gives the full command
TEST(CompilerTests, ContextCompileCommandWithoutDefaultsToFull) {
  struct NoParts : mkn::kul::lang::CompilerState {
    void add(mkn::kul::lang::AbstractCompilerInput const&) override {}
  };
  struct Ctx : mkn::kul::lang::Context {
    NoParts cs;
    mkn::kul::lang::ContextState state() const override { return {}; }
    mkn::kul::lang::CompilerState& compilerState() override { return cs; }
    void per_compiler_command(CompileHook) override {}
    std::string compileCommandFor(std::string const& in) const override { return "cc -flto " + in; }
  } ctx;
  mkn::kul::lang::Context const& c = ctx;
  EXPECT_EQ("cc -flto a.cpp", c.compileCommandFor("a.cpp", {"lto"}));
}
