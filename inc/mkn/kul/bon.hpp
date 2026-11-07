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

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

// Better Object Notation: json without quotes
//
//   {a: b, c: {d: e}}    map, only the first ':' separates key and value: {url: http://x}
//   {c{d: e}}            key directly followed by an object
//   {x, y, z}            bare entries are a list, a single one is a string: {a: {b}} is a: b
//   \, \: \{ \} \\       escapes, surrounding unescaped whitespace is trimmed
//
// parse() returns a bon::Node, see mkn/kul/bon/yaml.hpp for YAML::Node
// Malformed input throws mkn::kul::bon::Exception

namespace mkn::kul::bon {

class Exception : public mkn::kul::Exception {
 public:
  Exception(char const* f, std::size_t const& l, std::string const& s)
      : mkn::kul::Exception(f, l, s) {}
};

struct Node {
  using List = std::vector<Node>;
  using Map = std::vector<std::pair<std::string, Node>>;  // in input order

  std::variant<Map, List, std::string> value;

  bool is_map() const { return std::holds_alternative<Map>(value); }
  bool is_list() const { return std::holds_alternative<List>(value); }
  bool is_string() const { return std::holds_alternative<std::string>(value); }

  Map const& map() const { return get<Map>("map"); }
  List const& list() const { return get<List>("list"); }
  std::string const& str() const { return get<std::string>("string"); }

  // entries of a map or list, 1 for a string
  std::size_t size() const {
    if (is_string()) return 1;
    return is_map() ? map().size() : list().size();
  }

  bool contains(std::string const& key) const { return is_map() && find(key); }

  Node const& operator[](std::string const& key) const {
    if (auto const* node = find(key)) return *node;
    KEXCEPT(Exception, "bon: no key: ", key);
  }

  Node const& operator[](std::size_t const i) const {
    if (i >= list().size()) KEXCEPT(Exception, "bon: index ", i, " out of range");
    return list()[i];
  }

 private:
  template <typename T>
  T const& get(char const* const type) const {
    if (auto const* v = std::get_if<T>(&value)) return *v;
    KEXCEPT(Exception, "bon: not a ", type);
  }

  Node const* find(std::string const& key) const {
    for (auto const& [k, node] : map())
      if (k == key) return &node;
    return nullptr;
  }
};

namespace detail {

class Parser {
 public:
  explicit Parser(std::string const& s) : s{s} {}

  Node parse() {
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

  // last wins for duplicate keys
  static void set(Node::Map& map, std::string const& key, Node&& node) {
    for (auto& [k, n] : map)
      if (k == key) {
        n = std::move(node);
        return;
      }
    map.emplace_back(key, std::move(node));
  }

  Node object() {
    ++i;  // '{'
    Node::Map map;
    Node::List list;
    skip_ws();
    if (at('}')) {
      ++i;
      return {std::move(map)};
    }
    while (true) {
      skip_ws();
      if (at('{')) fail("object without key");
      auto key = text(":{,}");
      if (i == s.size()) fail("missing '}'");

      if (at(':') || at('{')) {
        if (key.empty()) fail("empty key");
        if (list.size()) fail("map entry in a list");
        if (at(':')) ++i;
        skip_ws();
        if (at('{'))
          set(map, key, object());
        else {
          set(map, key, Node{text("{,}")});
          if (at('{')) fail("unexpected '{' in value");
        }
      } else {
        if (key.empty()) fail("empty entry");
        if (map.size()) fail("list entry in a map");
        list.push_back(Node{std::move(key)});
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
    if (list.size() == 1) return std::move(list[0]);
    if (list.size()) return {std::move(list)};
    return {std::move(map)};
  }

  std::string const& s;
  std::size_t i = 0;
};

}  // namespace detail

inline Node parse(std::string const& s) { return detail::Parser{s}.parse(); }

}  // namespace mkn::kul::bon

#endif  // MKN_KUL_BON_HPP_
