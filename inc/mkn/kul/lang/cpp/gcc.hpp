/**
Copyright (c) 2026, Philip Deegan.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

    * Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
copyright notice, this list of conditions and the following disclaimer
in the documentation and/or other materials provided with the
distribution.
    * Neither the name of Philip Deegan nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef MKN_KUL_LANG_CPP_GCC_HPP
#define MKN_KUL_LANG_CPP_GCC_HPP

#include "mkn/kul/lang/compiler.hpp"

#include "mkn/kul/string.hpp"

#include <unordered_set>

namespace mkn::kul::lang::cpp {

class Exception : public mkn::kul::Exception {
 public:
  Exception(char const* f, int const l, std::string s) : mkn::kul::Exception(f, l, s) {}
};

enum class CCompiler_Type : uint16_t { NON = 0, GCC = 1, CLANG = 2, ICC = 3, HCC = 4, WIN = 5 };

class CCompiler : public Compiler {
 public:
  virtual ~CCompiler() {}
  virtual std::string staticLib(std::string const& lib) const = 0;
  bool sourceIsBin() const override { return false; }

  // Throws for a fileType that is neither a recognised C nor C++ extension -
  // callers no longer need their own recognised/unrecognised branching.
  std::string executableFor(std::string const& fileType) const {
    if (isCSource(fileType)) return ccBinary();
    if (isCxxSource(fileType)) return cxxBinary();
    KEXCEPT(Exception, "Unrecognised file type: " + fileType);
  }

  static std::string libName(CompilationInfo const& info, std::string const& lib) {
    return info.lib_prefix + lib + info.lib_ext;
  }
  static std::string oType(CompilationInfo const& info) { return info.obj_ext; }
  static std::string oStar(CompilationInfo const& info) { return "*." + info.obj_ext; }

  virtual CCompiler_Type type() const = 0;

  static std::string CC(std::string deFault) {
    return mkn::kul::env::EXISTS("CC") ? mkn::kul::env::GET("CC") : deFault;
  }
  static std::string CXX(std::string deFault) {
    return mkn::kul::env::EXISTS("CXX") ? mkn::kul::env::GET("CXX") : deFault;
  }
  static std::string LD(std::string deFault) {
    return mkn::kul::env::EXISTS("LD") ? mkn::kul::env::GET("LD") : deFault;
  }

 protected:
  CCompiler(int const& v) : Compiler(v) {}

  virtual std::string ccBinary() const = 0;
  virtual std::string cxxBinary() const = 0;

  static bool isCSource(std::string const& fileType) {
    return mkn::kul::String::NO_CASE_CMP(fileType, "c") || mkn::kul::String::NO_CASE_CMP(fileType, "h");
  }
  static bool isCxxSource(std::string const& fileType) {
    for (auto const& ext : {"cpp", "cc", "cxx", "hpp", "hh", "hxx"})
      if (mkn::kul::String::NO_CASE_CMP(fileType, ext)) return true;
    return false;
  }
};

// compileSource/buildExecutable/buildLibrary are declared only - defined out
// of line in lang/cpp/compilers.hpp, since they resolve the linker/compiler
// binary via Compilers::INSTANCE(), which needs this class complete first.
class GccCompiler : public CCompiler {
 public:
  GccCompiler(int const& v = 0) : CCompiler(v) {
    m_optimise_c.insert({{0, ""},
                         {1, "-O1"},
                         {2, "-O2"},
                         {3, "-O3"},
                         {4, "-O3 -funroll-loops"},
                         {5, "-O3 -funroll-loops"},
                         {6, "-O3 -funroll-loops"},
                         {7, "-O3 -funroll-loops"},
                         {8, "-O3 -funroll-loops"},
                         {9, "-O3 -funroll-loops -march=native -mtune=native"}});
    m_debug_c.insert({{0, "-DNDEBUG"},
                      {1, "-g1"},
                      {2, "-g2"},
                      {3, "-g3"},
                      {4, "-g3"},
                      {5, "-g3"},
                      {6, "-g3"},
                      {7, "-g3"},
                      {8, "-g3"},
                      {9, "-g3 -pg -fno-omit-frame-pointer"}});
    m_optimise_l_bin.insert(
        {{0, ""}, {1, ""}, {2, ""}, {3, ""}, {4, ""}, {5, ""}, {6, ""}, {7, ""}, {8, ""}, {9, ""}});
    m_optimise_l_lib.insert(
        {{0, ""}, {1, ""}, {2, ""}, {3, ""}, {4, ""}, {5, ""}, {6, ""}, {7, ""}, {8, ""}, {9, ""}});
    m_debug_l_bin.insert(
        {{0, ""}, {1, ""}, {2, ""}, {3, ""}, {4, ""}, {5, ""}, {6, ""}, {7, ""}, {8, ""}, {9, ""}});
    m_debug_l_lib.insert(
        {{0, ""}, {1, ""}, {2, ""}, {3, ""}, {4, ""}, {5, ""}, {6, ""}, {7, ""}, {8, ""}, {9, ""}});
    m_warn_c.insert({{0, "-w"},
                     {1, "-Wall"},
                     {2, "-Wall"},
                     {3, "-Wall"},
                     {4, "-Wall"},
                     {5, "-Wall"},
                     {6, "-Wall"},
                     {7, "-Wall"},
                     {8, "-Wall -Wextra"},
                     {9, "-Wall -Wextra -pedantic -Werror"}});
  }
  virtual ~GccCompiler() {}

  std::string staticLib(std::string const& lib) const override { return "lib" + lib + ".a"; }

  ProcessCapture compileSource(CompileDAO& dao) const KTHROW(mkn::kul::Exception) override;

  ProcessCapture buildExecutable(LinkDAO& dao) const KTHROW(mkn::kul::Exception) override;

  ProcessCapture buildLibrary(LinkDAO& dao) const KTHROW(mkn::kul::Exception) override;

  void preCompileHeader(std::vector<std::string> const& incs, std::vector<std::string> const& args,
                        std::string const& in, std::string const& out, bool dryRun = false) const
      KTHROW(mkn::kul::Exception) override {
    if (in.rfind(".") == std::string::npos) KEXCEPT(Exception, "Unknown header type");

    std::string h = in.substr(in.rfind(".") + 1);
    std::string cmd = executableFor(h) + (isCSource(h) ? " -x c-header " : " -x c++-header ");
    cmd += in + " ";
    for (std::string const& s : args) cmd += s + " ";
    for (std::string const& s : incs) cmd += "-I" + s + " ";

    cmd += " -o " + out;
    if (!dryRun && mkn::kul::os::exec(cmd) != 0) KEXCEPT(Exception, "Failed to pre-compile header");
  }

  std::string includeArg(std::string const& inc) const override {
    mkn::kul::Dir const d(inc);
    return d ? ("-I" + inc) : ("-include " + inc);
  }

  CCompiler_Type type() const override { return CCompiler_Type::GCC; }

  void rpathing(CompilationInfo const& info, mkn::kul::Process& p,
                [[maybe_unused]] mkn::kul::File const& out, std::vector<std::string> const& libs,
                std::vector<std::string> const& libPaths) const {
    std::unordered_set<std::string> rpaths;
    for (std::string const& path : libPaths) {
      for (std::string const& lib : libs) {
        mkn::kul::File lib_file(libName(info, lib), path);
        if (lib_file) rpaths.emplace(lib_file.dir().real());
      }
    }
    for (auto const& rpath : rpaths) {
      mkn::kul::Dir path(rpath);
      std::stringstream loader;
#if defined(__APPLE__)
      loader << "-Wl,-rpath," << path.esc();
      mkn::kul::File tmp_out(out);
      tmp_out.mk();
      loader << " -Wl,-rpath,@loader_path/" << tmp_out.relative(path);
      tmp_out.rm();
#else
      loader << "-Wl,-rpath=" << path.esc();
#endif
      p << loader.str();
    }
  }

 protected:
  std::string ccBinary() const override { return CC("gcc"); }
  std::string cxxBinary() const override { return CXX("g++"); }
};

class ClangCompiler : public GccCompiler {
 public:
  ClangCompiler(int const& v = 0) : GccCompiler(v) {}
  CCompiler_Type type() const override { return CCompiler_Type::CLANG; }

  // On Windows, clang targets the MSVC ABI: its driver resolves "-lfoo" by
  // searching for "foo.lib" (COFF/MSVC convention), not "libfoo.a" (GNU
  // convention) - so the archive this produces must match that naming.
  std::string staticLib(std::string const& lib) const override {
    if (MKN_KUL_STR(MKN_KUL_OS) == std::string("win")) return lib + ".lib";
    return GccCompiler::staticLib(lib);
  }

 protected:
  std::string ccBinary() const override { return CC("clang"); }
  std::string cxxBinary() const override { return CXX("clang++"); }
};

class HccCompiler : public GccCompiler {
 public:
  HccCompiler(int const& v = 0) : GccCompiler(v) {}
  CCompiler_Type type() const override { return CCompiler_Type::HCC; }

 protected:
  std::string ccBinary() const override { return CC("hcc"); }
  std::string cxxBinary() const override { return CXX("h++"); }
};

class IntelCompiler : public GccCompiler {
 public:
  IntelCompiler(int const& v = 0) : GccCompiler(v) {}
  CCompiler_Type type() const override { return CCompiler_Type::ICC; }

 protected:
  std::string ccBinary() const override { return CC("icc"); }
  std::string cxxBinary() const override { return CXX("icpc"); }
};

}  // namespace mkn::kul::lang::cpp

#endif  // MKN_KUL_LANG_CPP_GCC_HPP
