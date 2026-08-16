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
#ifndef MKN_KUL_LANG_DEF_HPP
#define MKN_KUL_LANG_DEF_HPP

#include "mkn/kul/env.hpp"
#include "mkn/kul/except.hpp"

#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace mkn::kul::lang {

enum class Mode { NONE = 0, STAT, SHAR };

inline Mode mode_from(std::string const& mode, std::string const& shared_str = "shared",
                       std::string const& static_str = "static") {
  if (mode == shared_str) return Mode::SHAR;
  if (mode == static_str) return Mode::STAT;
  return Mode::NONE;
}

struct CompilationInfo {
  std::string lib_prefix, lib_postfix, lib_ext, obj_ext;
  bool rpathing = true;
};

struct CompileCommand {
  std::string compiler;
  std::string in;
  std::string out;
  std::vector<std::string> args;
  bool dryRun = false;
};

// One input to a Compiler - a file plus any per-file extra args. Equality is
// by `in` only: a Source is re-added with new args by replacing, not by
// comparing args too.
struct Source {
  Source() = default;
  Source(std::string in) : in(std::move(in)) {}
  Source(std::string in, std::string args) : in(std::move(in)), args(std::move(args)) {}

  std::string in, args;
};

inline bool operator==(Source const& a, Source const& b) { return a.in == b.in; }

struct CompileEnv {
  std::vector<mkn::kul::env::Var> envVars;
  CompilationInfo compilationInfo;
};

class Exception : public mkn::kul::Exception {
 public:
  Exception(char const* f, uint16_t const& l, std::string const& s)
      : mkn::kul::Exception(f, l, s) {}
};

class Context;

// Read-only snapshot returned by Context::state() - one generator instead of
// one virtual per field (projectDir/includes/dependents/buildMode/sourceFiles
// /compileEnv).
struct ContextState {
  std::string projectDir;
  std::vector<std::pair<std::string, bool>> includes;
  std::vector<Context*> dependents;
  Mode buildMode = Mode::NONE;
  std::vector<std::string> sourceFiles;
  CompileEnv compileEnv;
};

// Tag base for CompilerState::add() - each concrete input is a plain-data
// description of one thing to add/set; CompilerState dynamic_casts to find
// out which. New input kinds are added by defining a new subtype, not a new
// virtual on Context or CompilerState.
struct AbstractCompilerInput {
  virtual ~AbstractCompilerInput() = default;
};

// AbstractCompilerInput's virtual destructor (needed for dynamic_cast in
// CompilerState::add()) makes these non-aggregates, so each needs a real
// constructor rather than brace-init.
struct IncludeInput : AbstractCompilerInput {
  IncludeInput(std::string path, bool is_public = true)
      : path(std::move(path)), is_public(is_public) {}
  std::string path;
  bool is_public;
};
struct LibPathInput : AbstractCompilerInput {
  LibPathInput(std::string path) : path(std::move(path)) {}
  std::string path;
};
struct LibInput : AbstractCompilerInput {
  LibInput(std::string name) : name(std::move(name)) {}
  std::string name;
};
struct LinkPrefixInput : AbstractCompilerInput {
  LinkPrefixInput(std::string value) : value(std::move(value)) {}
  std::string value;
};
struct BuildModeInput : AbstractCompilerInput {
  BuildModeInput(Mode mode) : mode(mode) {}
  Mode mode;
};
struct CompilationInfoInput : AbstractCompilerInput {
  CompilationInfoInput(CompilationInfo info) : info(std::move(info)) {}
  CompilationInfo info;
};

// Per-project mutable build state a Context hands out. Kept separate from
// Context itself so the read side (state()) and write side (add()) don't
// share one interface just because they're both "about the project".
class CompilerState {
 public:
  virtual ~CompilerState() = default;
  virtual void add(AbstractCompilerInput const& input) = 0;
};

// Surface a language Compiler backend (or a mkn.mod Module) is handed at each
// build phase. Deliberately few methods: everything crosses this boundary as
// a DAO (ContextState, AbstractCompilerInput) rather than one virtual per
// field/setter.
class Context {
 public:
  virtual ~Context() = default;

  virtual ContextState state() const = 0;
  virtual CompilerState& compilerState() = 0;

  using CompileHook = std::function<void(CompileCommand const&)>;
  virtual void per_compiler_command(CompileHook hook) = 0;

  // Resolved, ready-to-run compile command for an arbitrary (not necessarily
  // project-registered) source file - e.g. a module running a third-party
  // tool (iwyu) against the same flags real compilation would use.
  virtual std::string compileCommandFor(std::string const& in) const = 0;
};

}  // namespace mkn::kul::lang

#endif  // MKN_KUL_LANG_DEF_HPP
