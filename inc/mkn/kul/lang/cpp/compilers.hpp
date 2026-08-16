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
#ifndef MKN_KUL_LANG_CPP_COMPILERS_HPP
#define MKN_KUL_LANG_CPP_COMPILERS_HPP

#include "mkn/kul/lang/cs/msvc.hpp"
#include "mkn/kul/lang/cpp/gcc.hpp"
#include "mkn/kul/lang/cpp/msvc.hpp"

#include "mkn/kul/cli.hpp"

#include <cctype>
#include <memory>

namespace mkn::kul::lang::cpp {

class CompilerNotFoundException : public mkn::kul::Exception {
 public:
  CompilerNotFoundException(char const* f, int const l, std::string s)
      : mkn::kul::Exception(f, l, s) {}
};

struct CompilerIdentityDAO {
  std::vector<std::string> prefix;
  std::string binary;
  std::vector<std::string> trailing;
  std::string family;  // dispatch key resolved in cs/masks (e.g. "g++"); empty if unrecognized
};

class Compilers {
 public:
  static Compilers& INSTANCE() {
    static Compilers instance;
    return instance;
  }

  std::vector<std::string> keys() const {
    std::vector<std::string> ks;
    for (auto const& p : cs_) ks.push_back(p.first);
    return ks;
  }

  void addMask(std::string const& m, std::string const& c) KTHROW(CompilerNotFoundException) {
    std::string const k(key(c, cs_));
    if (cs_.count(m)) KEXCEPT(CompilerNotFoundException, "Mask cannot replace compiler");
    masks[m] = cs_[k];
    maskFamily[m] = k;
  }

  Compiler const* get(std::string const& comp) const KTHROW(CompilerNotFoundException) {
    auto k = key(comp, cs_);
    if (cs_.count(k)) return cs_[k];
    k = key(comp, masks);
    if (masks.count(k)) return masks[k];
    KEXCEPT(CompilerNotFoundException, "Key not found ", comp);
  }

  std::string base(std::string const& comp) const {
    auto k = key(comp, cs_);
    if (cs_.count(k)) return k;
    k = key(comp, masks);
    if (masks.count(k)) return maskFamily.count(k) ? maskFamily[k] : k;
    return k;
  }

  // Splits raw into {prefix, binary, trailing} and, independently, resolves
  // binary's compiler family (accounting for masks, version suffixes like
  // "g++-10", and target triples like "x86_64-linux-gnu-g++-12").
  CompilerIdentityDAO identify(std::string const& raw) const {
    CompilerIdentityDAO id = split(raw);
    auto k = key(id.binary, cs_);
    if (cs_.count(k)) {
      id.family = k;
    } else {
      k = key(id.binary, masks);
      if (masks.count(k)) id.family = maskFamily.count(k) ? maskFamily[k] : k;
    }
    return id;
  }

 private:
  Compilers() {
    clang = std::make_unique<ClangCompiler>();
    gcc = std::make_unique<GccCompiler>();
    hcc = std::make_unique<HccCompiler>();
    intel = std::make_unique<IntelCompiler>();
    winc = std::make_unique<WINCompiler>();
    wincs = std::make_unique<cs::WINCompiler>();

    cs_.insert(std::pair<std::string, Compiler*>("cl", winc.get()));
    cs_.insert(std::pair<std::string, Compiler*>("csc", wincs.get()));

    cs_.insert(std::pair<std::string, Compiler*>("clang", clang.get()));
    cs_.insert(std::pair<std::string, Compiler*>("clang++", clang.get()));

    cs_.insert(std::pair<std::string, Compiler*>("gcc", gcc.get()));
    cs_.insert(std::pair<std::string, Compiler*>("g++", gcc.get()));

    cs_.insert(std::pair<std::string, Compiler*>("hcc", hcc.get()));

    cs_.insert(std::pair<std::string, Compiler*>("icc", intel.get()));
    cs_.insert(std::pair<std::string, Compiler*>("icpc", intel.get()));

    cs_.insert(std::pair<std::string, Compiler*>("nvcc", gcc.get()));
  }

  static CompilerIdentityDAO split(std::string const& raw) {
    CompilerIdentityDAO id;
    auto const toks = mkn::kul::cli::asArgs(raw);
    if (toks.empty()) return id;

    size_t binaryIdx = 0;
    for (size_t i = 0; i < toks.size(); ++i) {
      if (!toks[i].empty() && toks[i][0] == '-') break;
      binaryIdx = i;
    }
    id.binary = toks[binaryIdx];
    for (size_t i = 0; i < binaryIdx; ++i) id.prefix.push_back(toks[i]);
    for (size_t i = binaryIdx + 1; i < toks.size(); ++i) id.trailing.push_back(toks[i]);
    return id;
  }

  static bool matchesFamily(std::string const& token, std::string const& family) {
    auto const endsWith = [](std::string const& s, std::string const& suf) {
      return s.size() >= suf.size() && s.compare(s.size() - suf.size(), suf.size(), suf) == 0;
    };
    if (token == family) return true;
    std::string const suffix = "-" + family;
    if (endsWith(token, suffix)) return true;
    // strip a trailing version suffix ("-10", "-17.0") and retry, so target-triple
    // prefixed + versioned binaries (e.g. "x86_64-linux-gnu-g++-12") also match.
    size_t i = token.size();
    while (i > 0 && (std::isdigit(static_cast<unsigned char>(token[i - 1])) || token[i - 1] == '.'))
      --i;
    if (i > 0 && i < token.size() && token[i - 1] == '-') {
      std::string const stripped = token.substr(0, i - 1);
      if (stripped == family || endsWith(stripped, suffix)) return true;
    }
    return false;
  }

  static std::string key(std::string comp, mkn::kul::hash::map::S2T<Compiler*> const& map) {
    mkn::kul::String::REPLACE_ALL(comp, ".exe", "");
    if (map.count(comp) > 0) return comp;

    std::string token = split(comp).binary;
    mkn::kul::String::REPLACE_ALL(token, ".exe", "");
    if (map.count(token) > 0) return token;

    if (std::string(mkn::kul::Dir(token).locl()).find(mkn::kul::Dir::SEP()) != std::string::npos) {
      std::string const base = token.substr(token.rfind(mkn::kul::Dir::SEP()) + 1);
      if (map.count(base) > 0) return base;
      token = base;
    }

    for (auto const& p : map)
      if (matchesFamily(token, p.first)) return p.first;

    return comp;
  }

  std::unique_ptr<Compiler> hcc, gcc, clang, intel, winc, wincs;
  mkn::kul::hash::map::S2T<Compiler*> cs_, masks;
  mkn::kul::hash::map::S2S maskFamily;
};

// Out-of-line: resolves the linker/compiler binary via Compilers::INSTANCE(),
// which needs GccCompiler complete, so these can't be defined in gcc.hpp.
inline ProcessCapture GccCompiler::compileSource(CompileDAO& dao) const
    KTHROW(mkn::kul::Exception) {
  auto const env = dao.ctx.state().compileEnv;
  auto &compiler = dao.compiler, &in = dao.in, &out = dao.out;
  auto& args = dao.args;
  auto& dryRun = dao.dryRun;

  std::string const fileType = in.substr(in.rfind(".") + 1);

  ProcessCapture pc;
  try {
    auto const id = Compilers::INSTANCE().identify(compiler);
    std::string const bin =
        mkn::kul::String::NO_CASE_CMP(fileType, "c") ? CC(id.binary) : CXX(id.binary);

    mkn::kul::Process p(id.prefix.empty() ? bin : id.prefix[0]);
    for (size_t i = 1; i < id.prefix.size(); i++) p.arg(id.prefix[i]);
    if (!id.prefix.empty()) p.arg(bin);
    for (std::string const& t : id.trailing) p.arg(t);
    for (std::string const& s : args) p.arg(s);
    p.arg("-o").arg(out).arg("-c").arg(in);
    if (!mkn::kul::LogMan::INSTANCE().inf()) pc.setProcess(p);

    if (!dryRun) p.set(env.envVars).start();
    pc.cmd(p.toString());
  } catch (std::exception const& e) {
    pc.exception(std::current_exception());
  }
  pc.file(out);
  return pc;
}

inline ProcessCapture GccCompiler::buildExecutable(LinkDAO& dao) const KTHROW(mkn::kul::Exception) {
  auto const env = dao.ctx.state().compileEnv;
  auto &objects = dao.objects, &libs = dao.libs, &libPaths = dao.libPaths;
  auto& dryRun = dao.dryRun;
  auto &linker = dao.linker, &linkerEnd = dao.linkerEnd, &out = dao.out;
  auto& mode = dao.mode;

  mkn::kul::hash::set::String dirs;
  for (auto const& d : dao.stars) dirs.insert(d.real());

  std::string exe = out;
  if (MKN_KUL_STR(MKN_KUL_OS) == std::string("win")) exe += ".exe";
  mkn::kul::File(exe).dir().mk();

  ProcessCapture pc;
  try {
    auto const id = Compilers::INSTANCE().identify(linker);
    std::string const bin = LD(id.binary);
    mkn::kul::Process p(id.prefix.empty() ? bin : id.prefix[0]);
    for (size_t i = 1; i < id.prefix.size(); i++) p.arg(id.prefix[i]);
    if (!id.prefix.empty()) p.arg(bin);
    for (std::string const& t : id.trailing) p.arg(t);
    for (std::string const& path : libPaths) p.arg("-L" + path);
    if (mode == Mode::STAT) p.arg("-static");
    if (env.compilationInfo.rpathing && (mode == Mode::SHAR || mode == Mode::NONE)) {
      mkn::kul::File file(exe);
      rpathing(env.compilationInfo, p, file, libs, libPaths);
    }
    p.arg("-o").arg(exe);
    for (std::string const& d : dirs) p << mkn::kul::File(oStar(env.compilationInfo), d).escm();
    for (std::string const& o : objects) p << mkn::kul::File(o).escm();
    for (std::string const& lib : libs) p.arg("-l" + lib);
    for (std::string const& s : mkn::kul::cli::asArgs(linkerEnd)) p << s;

    if (!dryRun) p.set(env.envVars).start();
    pc.cmd(p.toString());
  } catch (std::exception const& e) {
    pc.exception(std::current_exception());
  }
  pc.file(out);
  return pc;
}

inline ProcessCapture GccCompiler::buildLibrary(LinkDAO& dao) const KTHROW(mkn::kul::Exception) {
  auto const env = dao.ctx.state().compileEnv;
  auto &objects = dao.objects, &libs = dao.libs, &libPaths = dao.libPaths;
  auto& dryRun = dao.dryRun;
  auto &linker = dao.linker, &linkerEnd = dao.linkerEnd;
  auto& mode = dao.mode;

  mkn::kul::File out(dao.out);

  mkn::kul::hash::set::String dirs;
  for (auto const& d : dao.stars) dirs.insert(d.real());

  std::string lib = out.dir().join(libName(env.compilationInfo, out.name()));
  if (mode == Mode::STAT) lib = out.dir().join(staticLib(out.name()));

  mkn::kul::File out_file(lib);
  out_file.dir().mk();
  lib = out_file.esc();

  ProcessCapture pc;
  try {
    auto const id = Compilers::INSTANCE().identify(linker);
    std::string const bin = (mode == Mode::SHAR) ? LD(id.binary) : id.binary;
    mkn::kul::Process p(id.prefix.empty() ? bin : id.prefix[0]);
    for (size_t i = 1; i < id.prefix.size(); i++) p.arg(id.prefix[i]);
    if (!id.prefix.empty()) p.arg(bin);
    for (std::string const& t : id.trailing) p.arg(t);
    if (mode == Mode::SHAR) p.arg("-shared").arg("-o");
    p.arg(lib);
    for (std::string const& d : dirs) p.arg(mkn::kul::File(oStar(env.compilationInfo), d).escm());
    for (std::string const& o : objects) p << mkn::kul::File(o).escm();

    if (mode == Mode::SHAR) {
      for (std::string const& path : libPaths) p.arg("-L" + path);
      for (std::string const& lib : libs) p.arg("-l" + lib);
      if (env.compilationInfo.rpathing) rpathing(env.compilationInfo, p, out, libs, libPaths);
#if defined(__APPLE__)
      std::stringstream loader;
      loader << "-Wl,-install_name,@rpath/" << mkn::kul::File(lib).name();
      p << loader.str();
#endif
    }
    for (std::string const& s : mkn::kul::cli::asArgs(linkerEnd)) p.arg(s);

    if (!dryRun) p.set(env.envVars).start();
    pc.cmd(p.toString());
  } catch (std::exception const& e) {
    pc.exception(std::current_exception());
  }
  pc.file(lib);
  return pc;
}

}  // namespace mkn::kul::lang::cpp

#endif  // MKN_KUL_LANG_CPP_COMPILERS_HPP
