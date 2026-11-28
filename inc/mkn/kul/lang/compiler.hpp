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
#ifndef MKN_KUL_LANG_COMPILER_HPP
#define MKN_KUL_LANG_COMPILER_HPP

#include "mkn/kul/except.hpp"
#include "mkn/kul/lang/def.hpp"  // IWYU pragma: export
#include "mkn/kul/lang/cpp/compiler/def.hpp"
#include "mkn/kul/proc.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <initializer_list>
#include <string>
#include <unordered_map>
#include <vector>
#include <exception>
#include <utility>

namespace mkn::kul::lang {

class ProcessCapture : public mkn::kul::ProcessCapture {
 public:
  ProcessCapture() {}
  ProcessCapture(mkn::kul::AProcess& p) : mkn::kul::ProcessCapture(p) {}

  void exception(std::exception_ptr const& e) { ep = e; }
  std::exception_ptr const& exception() const { return ep; }

  void cmd(std::string const& cm) { this->c = cm; }
  std::string const& cmd() const { return c; }

  void file(std::string const& f) { this->f = f; }
  std::string const& file() const { return f; }

 private:
  std::exception_ptr ep;
  std::string c, f;
};

struct CompileDAO {
  Context const& ctx;
  std::string const& compiler;
  std::string const& in;
  std::string const& out;
  std::vector<std::string> const& args;
  Mode const& mode;
  bool dryRun = false;
};

struct LinkDAO {
  Context const& ctx;
  std::string const& linker;
  std::string const& linkerEnd;
  std::string const& out;
  std::vector<std::filesystem::path> stars;
  std::vector<std::string> const& objects;
  std::vector<std::string> const& libs;
  std::vector<std::string> const& libPaths;
  Mode const& mode;
  bool dryRun = false;
};

// A named part of a level's flags, e.g. {"lto", "-flto=auto"}, so a build can replace or drop it
//  by name, or a module leave it out of its own command
struct FlagPart {
  std::string name, flags;
};

// A level's flags as named parts, in the order they're joined into a command. A plain string is
//  one unnamed part, named after its kind (optimise / debug / warn) when read from a Compiler.
class Flags {
 public:
  Flags() = default;
  Flags(char const* const flags) : Flags(std::string(flags)) {}
  Flags(std::string flags) {
    if (!flags.empty()) _parts.push_back({"", std::move(flags)});
  }
  Flags(std::initializer_list<FlagPart> parts) : _parts(parts) {}

  std::vector<FlagPart> const& parts() const { return _parts; }

  // replaces the flags of the part called name, appended if there's none, empty flags drop it
  Flags& set(std::string const& name, std::string flags) {
    auto const it =
        std::find_if(_parts.begin(), _parts.end(), [&](auto const& p) { return p.name == name; });
    if (flags.empty()) {
      if (it != _parts.end()) _parts.erase(it);
    } else if (it != _parts.end()) {
      it->flags = std::move(flags);
    } else {
      _parts.push_back({name, std::move(flags)});
    }
    return *this;
  }
  // these flags less the part called name
  MKN_KUL_FN_SIG_RET Flags without(std::string const& name) const {
    return Flags(*this).set(name, "");
  }
  // unnamed parts named name
  MKN_KUL_FN_SIG_RET Flags named(std::string const& name) const {
    Flags f(*this);
    for (auto& p : f._parts)
      if (p.name.empty()) p.name = name;
    return f;
  }
  // the parts' flags, space separated, empty ones skipped
  MKN_KUL_FN_SIG_RET std::string str() const {
    std::string s;
    for (auto const& p : _parts) {
      if (p.flags.empty()) continue;
      if (!s.empty()) s += " ";
      s += p.flags;
    }
    return s;
  }

 private:
  std::vector<FlagPart> _parts;
};

class Compiler {
 public:
  virtual ~Compiler() {}
  virtual bool sourceIsBin() const = 0;

  virtual ProcessCapture compileSource(CompileDAO& dao) const KTHROW(mkn::kul::Exception) = 0;

  // the arguments taking inc (a directory, or a file to include), each one argument as it is
  virtual std::vector<std::string> includeArgs(std::string const& inc) const = 0;

  virtual ProcessCapture buildExecutable(LinkDAO& dao) const KTHROW(mkn::kul::Exception) = 0;

  virtual ProcessCapture buildLibrary(LinkDAO& dao) const KTHROW(mkn::kul::Exception) = 0;

  // C++20 named modules: a compiler without them (the default) gives no arguments and no scan;
  //  a build provides each module from one unit, compiled with moduleOutputArgs, before the units
  //  importing it, which find it through prebuiltModulesArgs
  virtual bool namedModules() const { return false; }
  // the built module interface's file name of a module (e.g. "a:b", a partition)
  virtual std::string moduleFile(std::string const& logical) const { return logical; }
  // a unit providing a module: its built module interface written to bmi too
  virtual std::vector<std::string> moduleOutputArgs(std::string const& /*bmi*/) const { return {}; }
  // the built module interfaces in dir, for units importing them
  virtual std::vector<std::string> prebuiltModulesArgs(std::string const& /*dir*/) const {
    return {};
  }
  // P1689 JSON (outs()) of what each unit of the compilation database db provides and requires;
  //  compiler: the build's compiler string, for the scanner of the same toolchain
  virtual ProcessCapture scanModules(std::string const& /*compiler*/, std::string const& /*db*/,
                                     std::size_t /*threads*/) const KTHROW(mkn::kul::Exception) {
    KEXCEPTION("compiler without C++20 named modules");
  }

  virtual void preCompileHeader(std::vector<std::string> const& incs,
                                std::vector<std::string> const& args, std::string const& in,
                                std::string const& out, bool dryRun = false) const
      KTHROW(mkn::kul::Exception) = 0;

  // a level's flags in parts, unnamed parts named after their kind
  Flags compilerDebugFlags(uint8_t const& key) const { return at(m_debug_c, key, "debug"); }
  Flags compilerOptimizationFlags(uint8_t const& key) const {
    return at(m_optimise_c, key, "optimise");
  }
  Flags compilerWarningFlags(uint8_t const& key) const { return at(m_warn_c, key, "warn"); }
  Flags linkerDebugBinFlags(uint8_t const& key) const { return at(m_debug_l_bin, key, "debug"); }
  Flags linkerDebugLibFlags(uint8_t const& key) const { return at(m_debug_l_lib, key, "debug"); }
  Flags linkerOptimizationBinFlags(uint8_t const& key) const {
    return at(m_optimise_l_bin, key, "optimise");
  }
  Flags linkerOptimizationLibFlags(uint8_t const& key) const {
    return at(m_optimise_l_lib, key, "optimise");
  }

  // the same, joined
  std::string compilerDebug(uint8_t const& key) const { return compilerDebugFlags(key).str(); }
  std::string compilerOptimization(uint8_t const& key) const {
    return compilerOptimizationFlags(key).str();
  }
  std::string compilerWarning(uint8_t const& key) const { return compilerWarningFlags(key).str(); }
  std::string linkerDebugBin(uint8_t const& key) const { return linkerDebugBinFlags(key).str(); }
  std::string linkerDebugLib(uint8_t const& key) const { return linkerDebugLibFlags(key).str(); }
  std::string linkerOptimizationBin(uint8_t const& key) const {
    return linkerOptimizationBinFlags(key).str();
  }
  std::string linkerOptimizationLib(uint8_t const& key) const {
    return linkerOptimizationLibFlags(key).str();
  }

 protected:
  Compiler(int const& v) : version(v) {}
  int const version;
  std::unordered_map<uint8_t, Flags> m_optimise_c, m_optimise_l_bin, m_optimise_l_lib, m_debug_c,
      m_debug_l_bin, m_debug_l_lib, m_warn_c;

 private:
  static Flags at(std::unordered_map<uint8_t, Flags> const& map, uint8_t const key,
                  std::string const& kind) {
    auto const it = map.find(key);
    return it == map.end() ? Flags{} : it->second.named(kind);
  }
};

}  // namespace mkn::kul::lang

#endif  // MKN_KUL_LANG_COMPILER_HPP
