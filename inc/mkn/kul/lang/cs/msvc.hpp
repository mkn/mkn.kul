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
#ifndef MKN_KUL_LANG_CS_MSVC_HPP
#define MKN_KUL_LANG_CS_MSVC_HPP

#include "mkn/kul/lang/compiler.hpp"

#include "mkn/kul/string.hpp"

namespace mkn::kul::lang::cs {

class Exception : public mkn::kul::Exception {
 public:
  Exception(char const* f, int const l, std::string s) : mkn::kul::Exception(f, l, s) {}
};

class WINCompiler : public Compiler {
 public:
  WINCompiler(int const& v = 0) : Compiler(v) {}
  virtual ~WINCompiler() {}

  bool sourceIsBin() const override { return true; }

  ProcessCapture compileSource(CompileDAO&) const override {
    KEXCEPTION("Method compileSource is not implemented in C Sharp");
  }

  std::string includeArg(std::string const&) const override {
    KEXCEPTION("Method includeArg is not implemented in C Sharp");
  }

  ProcessCapture buildExecutable(LinkDAO& dao) const KTHROW(mkn::kul::Exception) override {
    auto const env = dao.ctx.state().compileEnv;
    auto &objects = dao.objects, &libs = dao.libs, &libPaths = dao.libPaths;
    auto& dryRun = dao.dryRun;
    auto &linker = dao.linker, &linkerEnd = dao.linkerEnd, &out = dao.out;

    std::string exe = out + ".exe";
    std::string cmd = linker;
    std::vector<std::string> bits;
    if (linker.find(" ") != std::string::npos) {
      bits = mkn::kul::String::SPLIT(linker, ' ');
      cmd = bits[0];
    }
    mkn::kul::Process p(cmd);
    ProcessCapture pc(p);
    for (unsigned int i = 1; i < bits.size(); i++) p.arg(bits[i]);
    p.arg("/NOLOGO").arg("/OUT:" + exe);
    if (libs.size()) {
      std::stringstream ss;
      for (std::string const& path : libPaths) ss << path << ",";
      std::string s(ss.str());
      s.pop_back();
      p.arg("/LIB:" + s);
      ss.str(std::string());
      for (std::string const& lib : libs) ss << lib << ".dll,";
      s = ss.str();
      s.pop_back();
      p.arg("/REFERENCE:" + s);
    }

    for (std::string const& o : objects) p << o;
    if (linkerEnd.find(" ") != std::string::npos)
      for (std::string const& s : mkn::kul::String::SPLIT(linkerEnd, ' ')) p.arg(s);
    else
      p.arg(linkerEnd);

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
    auto& objects = dao.objects;
    auto& dryRun = dao.dryRun;
    auto &linker = dao.linker, &linkerEnd = dao.linkerEnd;

    mkn::kul::File out(dao.out);
    std::string dll = out.full() + ".dll";
    std::string cmd = linker;
    std::vector<std::string> bits;
    if (linker.find(" ") != std::string::npos) {
      bits = mkn::kul::String::SPLIT(linker, ' ');
      cmd = bits[0];
    }
    mkn::kul::Process p(cmd);
    p.arg("/target:library").arg("/OUT:" + dll).arg("/nologo");
    ProcessCapture pc(p);
    for (unsigned int i = 1; i < bits.size(); i++) p.arg(bits[i]);
    for (std::string const& o : objects) p << o;
    for (std::string const& s : mkn::kul::String::SPLIT(linkerEnd, ' ')) p.arg(s);
    try {
      if (!dryRun) p.set(env.envVars).start();
    } catch (mkn::kul::proc::Exception const& e) {
      pc.exception(std::current_exception());
    }
    pc.file(dll);
    pc.cmd(p.toString());
    return pc;
  }

  void preCompileHeader(std::vector<std::string> const&, std::vector<std::string> const&,
                        std::string const&, std::string const&, bool = false) const
      KTHROW(mkn::kul::Exception) override {
    KEXCEPTION("Method preCompileHeader is not implemented in C Sharp");
  }
};

}  // namespace mkn::kul::lang::cs

#endif  // MKN_KUL_LANG_CS_MSVC_HPP
