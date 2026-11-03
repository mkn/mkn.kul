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
#ifndef MKN_KUL_LANG_CPP_MSVC_HPP
#define MKN_KUL_LANG_CPP_MSVC_HPP

#include "mkn/kul/lang/cpp/gcc.hpp"

#include "mkn/kul/cli.hpp"
#include "mkn/kul/env.hpp"
#include "mkn/kul/string.hpp"
#include "mkn/kul/lang/cpp/msvc/def.hpp"

#include <fstream>

namespace mkn::kul::lang::cpp {

class WINCompiler : public CCompiler {
 public:
  WINCompiler(int const& v = 0) : CCompiler(v) {
    m_optimise_c.insert({{0, "-Od"},
                         {1, "-O1"},
                         {2, "-O2"},
                         {3, "-O2"},
                         {4, "-O2"},
                         {5, "-O2"},
                         {6, "-O2"},
                         {7, "-O2"},
                         {8, "-O2"},
                         {9, "-Ox -GA -Gw -Zc:inline -EHsc"}});
    m_debug_c.insert({{0, "-DNDEBUG"},
                      {1, ""},
                      {2, ""},
                      {3, ""},
                      {4, ""},
                      {5, ""},
                      {6, ""},
                      {7, ""},
                      {8, ""},
                      {9, "-Z7"}});
    m_optimise_l_bin.insert(
        {{0, ""}, {1, ""}, {2, ""}, {3, ""}, {4, ""}, {5, ""}, {6, ""}, {7, ""}, {8, ""}, {9, ""}});
    m_debug_l_bin.insert({{0, ""},
                          {1, ""},
                          {2, ""},
                          {3, ""},
                          {4, ""},
                          {5, ""},
                          {6, ""},
                          {7, ""},
                          {8, ""},
                          {9, "-DEBUG"}});
    m_debug_l_lib.insert(
        {{0, ""}, {1, ""}, {2, ""}, {3, ""}, {4, ""}, {5, ""}, {6, ""}, {7, ""}, {8, ""}, {9, ""}});
    m_warn_c.insert({{0, "-W0"},
                     {1, "-W1"},
                     {2, "-W2"},
                     {3, "-W3"},
                     {4, "-W4"},
                     {5, "-Wall"},
                     {6, "-Wall"},
                     {7, "-Wall"},
                     {8, "-Wall"},
                     {9, "-Wall"}});
  }
  virtual ~WINCompiler() {}

  std::string staticLib(std::string const& lib) const override { return lib + ".lib"; }

  ProcessCapture compileSource(CompileDAO& dao) const KTHROW(mkn::kul::Exception) override {
    auto const env = dao.ctx.state().compileEnv;
    auto &compiler = dao.compiler, &in = dao.in, &out = dao.out;
    auto& args = dao.args;
    auto& dryRun = dao.dryRun;

    std::string const fileType = in.substr(in.rfind(".") + 1);
    std::string cmd = mkn::kul::String::NO_CASE_CMP(fileType, "c") ? CC(compiler) : CXX(compiler);

    std::vector<std::string> bits;
    if (cmd == compiler && compiler.find(" ") != std::string::npos) {
      bits = mkn::kul::cli::asArgs(compiler);
      cmd = bits[0];
    }
    mkn::kul::Process p(cmd);
    for (size_t i = 1; i < bits.size(); i++) p.arg(bits[i]);
    p.arg("-nologo");
    for (std::string const& s : args) p.arg(s);
    p.arg("-c").arg("-Fo\"" + out + "\"").arg("\"" + in + "\"");
    ProcessCapture pc;
    if (!mkn::kul::LogMan::INSTANCE().inf()) pc.setProcess(p);

    try {
      if (!dryRun) p.set(env.envVars).start();
    } catch (mkn::kul::Exception const& e) {
      pc.exception(std::current_exception());
    }
    pc.file(out);
    pc.cmd(p.toString());
    return pc;
  }

  ProcessCapture buildExecutable(LinkDAO& dao) const KTHROW(mkn::kul::Exception) override {
    auto const env = dao.ctx.state().compileEnv;
    auto &objects = dao.objects, &libs = dao.libs, &libPaths = dao.libPaths;
    auto& dryRun = dao.dryRun;
    auto &linker = dao.linker, &linkerEnd = dao.linkerEnd, &out = dao.out;

    mkn::kul::hash::set::String dirs;
    for (auto const& d : dao.stars) dirs.insert(d.real());

    std::string exe = out + ".exe";
    mkn::kul::File out_file(exe);
    out_file.dir().mk();

    std::string cmd = LD(linker);
    std::vector<std::string> bits;
    if (cmd == linker && linker.find(" ") != std::string::npos) {
      bits = mkn::kul::cli::asArgs(linker);
      cmd = bits[0];
    }
    mkn::kul::Process p(cmd);
    for (unsigned int i = 1; i < bits.size(); i++) p.arg(bits[i]);
    p.arg("-OUT:\"" + out_file.escm() + "\"").arg("-nologo");
    for (std::string const& path : libPaths) p.arg("-LIBPATH:\"" + path + "\"");
    for (std::string const& d : dirs) p.arg(mkn::kul::File(oStar(env.compilationInfo), d).escm());
    for (std::string const& o : objects) p << mkn::kul::File(o).escm();
    for (std::string const& lib : libs) p.arg(staticLib(lib));
    for (std::string const& s : mkn::kul::cli::asArgs(linkerEnd)) p.arg(s);

    ProcessCapture pc;
    try {
      if (!dryRun) p.set(env.envVars).start();
    } catch (mkn::kul::proc::Exception const& e) {
      pc.exception(std::current_exception());
    }
    pc.file(exe);
    pc.cmd(p.toString());
    return pc;
  }

  ProcessCapture buildLibrary(LinkDAO& dao) const KTHROW(mkn::kul::Exception) override {
    auto const env = dao.ctx.state().compileEnv;
    auto &objects = dao.objects, &libs = dao.libs, &libPaths = dao.libPaths;
    auto& dryRun = dao.dryRun;
    auto &linker = dao.linker, &linkerEnd = dao.linkerEnd;
    auto& mode = dao.mode;

    mkn::kul::File out(dao.out);
    mkn::kul::hash::set::String dirs;
    for (auto const& d : dao.stars) dirs.insert(d.real());

    std::string lib = out.dir().join(staticLib(out.name()));
    if (mode == Mode::SHAR) lib = out.dir().join(libName(env.compilationInfo, out.name()));
    std::string imp = out.dir().join(staticLib(out.name()));

    std::string cmd = linker;
    if (mode == Mode::SHAR) cmd = LD(linker);
    std::vector<std::string> bits;
    if (cmd == linker && linker.find(" ") != std::string::npos) {
      bits = mkn::kul::cli::asArgs(linker);
      cmd = bits[0];
    }
    mkn::kul::Process p(cmd);

    auto out_file = mkn::kul::File{lib};
    out_file.dir().mk();
    auto imp_file = mkn::kul::File{imp};
    imp_file.dir().mk();

    for (unsigned int i = 1; i < bits.size(); i++) p.arg(bits[i]);
    p.arg("-nologo");
    if (mode == Mode::STAT) p.arg("-LTCG");
    p.arg("-OUT:\"" + out_file.escm() + "\"");
    if (mode == Mode::SHAR) {
#if defined(_WIN32)
      if (!dryRun && export_all_symbols())
        p.arg("-DEF:\"" + write_export_def(objects, dirs, env.compilationInfo, imp_file) + "\"");
#endif
      p.arg("-IMPLIB:\"" + imp_file.escm() + "\"").arg("-DLL");
      for (std::string const& path : libPaths) p.arg("-LIBPATH:\"" + path + "\"");
      for (std::string const& lib : libs) p.arg(staticLib(lib));
    }
    for (std::string const& d : dirs) p.arg(mkn::kul::File(oStar(env.compilationInfo), d).escm());
    for (std::string const& o : objects) p << mkn::kul::File(o).escm();
    for (std::string const& s : mkn::kul::cli::asArgs(linkerEnd)) p.arg(s);

    ProcessCapture pc;
    try {
      if (!dryRun) p.set(env.envVars).start();
    } catch (mkn::kul::proc::Exception const& e) {
      pc.exception(std::current_exception());
    }
    pc.file(lib);
    pc.cmd(p.toString());
    return pc;
  }

  void preCompileHeader(std::vector<std::string> const&, std::vector<std::string> const&,
                        std::string const&, std::string const&, bool = false) const
      KTHROW(mkn::kul::Exception) override {
    KEXCEPTION("Method is not implemented");
  }

  std::string includeArg(std::string const& inc) const override { return "-I\"" + inc + "\""; }

  CCompiler_Type type() const override { return CCompiler_Type::WIN; }

 protected:
  std::string ccBinary() const override { return CC("cl"); }
  std::string cxxBinary() const override { return CXX("cl"); }

#if defined(_WIN32)
 private:
  // MKN_WINDOWS_EXPORT_ALL_SYMBOLS=1: DLLs export every external symbol of their objects via a
  // generated .def file, no __declspec(dllexport) needed
  static bool export_all_symbols() {
    std::string const v = mkn::kul::env::GET("MKN_WINDOWS_EXPORT_ALL_SYMBOLS");
    return !v.empty() && mkn::kul::String::BOOL(v);
  }

  // .def for the explicit objects and those linked via "<dir>/*.<obj_ext>", next to the import
  // lib, returns its escaped path
  static std::string write_export_def(std::vector<std::string> const& objects,
                                      mkn::kul::hash::set::String const& dirs,
                                      CompilationInfo const& info, mkn::kul::File const& imp) {
    msvc::ExportDef exports;
    auto const add = [&](std::string const& obj) {
      auto const result = exports.add(obj);
      if (result == msvc::ExportDef::Result::Failed)
        KEXCEPT(mkn::kul::Exception, "MKN_WINDOWS_EXPORT_ALL_SYMBOLS: " + exports.error());
      if (result == msvc::ExportDef::Result::LLVMBitcode)
        KERR << "MKN_WINDOWS_EXPORT_ALL_SYMBOLS: skipping " << exports.error();
    };
    for (auto const& o : objects) add(mkn::kul::File(o).real());
    std::string const ext = "." + info.obj_ext;
    for (auto const& d : dirs)
      for (auto const& f : mkn::kul::Dir(d).files())
        if (f.name().size() > ext.size() && f.name().ends_with(ext)) add(f.real());

    std::string const impName = imp.name();
    mkn::kul::File const def{impName.substr(0, impName.rfind('.')) + ".def", imp.dir()};
    std::ofstream out{def.full(), std::ios::binary | std::ios::trunc};
    exports.write(out);
    out.close();
    if (!out)
      KEXCEPT(mkn::kul::Exception, "MKN_WINDOWS_EXPORT_ALL_SYMBOLS: can't write " + def.full());
    return def.escm();
  }
#endif
};

}  // namespace mkn::kul::lang::cpp

#endif  // MKN_KUL_LANG_CPP_MSVC_HPP
