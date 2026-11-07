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
#ifndef MKN_KUL_BON_HPP_
#define MKN_KUL_BON_HPP_

#include "mkn/kul/except.hpp"

#include "yaml-cpp/yaml.h"

#include <cstddef>
#include <string>
#include <string_view>

// Better Object Notation: json without quotes
//
//   {a: b, c: {d: e}}    map, only the first ':' separates key and value: {url: http://x}
//   {c{d: e}}            key directly followed by an object
//   {x, y, z}            bare entries are a list, a single one is a scalar: {a: {b}} is a: b
//   \, \: \{ \} \\       escapes, surrounding unescaped whitespace is trimmed
//
// Malformed input throws mkn::kul::bon::Exception

namespace mkn::kul::bon {

class Exception : public mkn::kul::Exception {
 public:
  Exception(char const* f, std::size_t const& l, std::string const& s)
      : mkn::kul::Exception(f, l, s) {}
};

namespace detail {

class Parser {
 public:
  explicit Parser(std::string const& s) : s{s} {}

  YAML::Node parse() {
    skip_ws();
    if (!at('{')) fail("expected '{'");
    auto node = object();
    skip_ws();
    if (i != s.size()) fail("unexpected input after '}'");
    return node;
  }

 private:
  [[noreturn]] void fail(std::string const& msg) const {
    KEXCEPT(Exception, "bon: ", msg, " at position ", i, " in: ", s);
  }

  bool at(char const c) const { return i < s.size() && s[i] == c; }

  static bool ws(char const c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }

  void skip_ws() {
    while (i < s.size() && ws(s[i])) ++i;
  }

  // text up to an unescaped char in stops, unescaped whitespace around it trimmed
  std::string text(std::string_view const stops) {
    skip_ws();
    std::string t;
    std::size_t keep = 0;  // length up to the last char not to trim
    for (; i < s.size() && stops.find(s[i]) == std::string_view::npos; ++i) {
      if (s[i] == '\\') {
        if (++i == s.size()) fail("escape at end of input");
        t += s[i];
        keep = t.size();
        continue;
      }
      t += s[i];
      if (!ws(s[i])) keep = t.size();
    }
    t.resize(keep);
    return t;
  }

  YAML::Node object() {
    ++i;  // '{'
    YAML::Node map{YAML::NodeType::Map}, list{YAML::NodeType::Sequence};
    skip_ws();
    if (at('}')) {
      ++i;
      return map;
    }
    while (true) {
      skip_ws();
      if (at('{')) fail("object without key");
      auto const key = text(":{,}");
      if (i == s.size()) fail("missing '}'");

      if (at(':') || at('{')) {
        if (key.empty()) fail("empty key");
        if (list.size()) fail("map entry in a list");
        if (at(':')) ++i;
        skip_ws();
        if (at('{'))
          map[key] = object();
        else {
          map[key] = text("{,}");
          if (at('{')) fail("unexpected '{' in value");
        }
      } else {
        if (key.empty()) fail("empty entry");
        if (map.size()) fail("list entry in a map");
        list.push_back(key);
      }

      skip_ws();
      if (i == s.size()) fail("missing '}'");
      if (at('}')) break;
      if (!at(',')) fail("expected ',' or '}'");
      ++i;
      skip_ws();
      if (at('}')) break;  // trailing comma
    }
    ++i;  // '}'
    if (list.size()) return list.size() == 1 ? list[0] : list;
    return map;
  }

  std::string const& s;
  std::size_t i = 0;
};

}  // namespace detail

inline YAML::Node from(std::string const& s) { return detail::Parser{s}.parse(); }

}  // namespace mkn::kul::bon

#endif  // MKN_KUL_BON_HPP_
