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
#ifndef MKN_KUL_BON_YAML_HPP_
#define MKN_KUL_BON_YAML_HPP_

#include "mkn/kul/bon.hpp"
#include "yaml-cpp/yaml.h"
#include "mkn/kul/except.hpp"

#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace mkn::kul::bon {

inline YAML::Node to_yaml(Node const& node) {
  if (node.is_string()) return YAML::Node{node.str()};
  if (node.is_list()) {
    YAML::Node list{YAML::NodeType::Sequence};
    for (auto const& n : node.list()) list.push_back(to_yaml(n));
    return list;
  }
  YAML::Node map{YAML::NodeType::Map};
  for (auto const& [key, n] : node.map()) map[key] = to_yaml(n);
  return map;
}

// parse s to a YAML::Node
inline YAML::Node from(std::string const& s) { return to_yaml(parse(s)); }

namespace detail {

// active: the sequences and maps being converted on this path, an alias back to one is a cycle
inline Node from_yaml(YAML::Node const& node, std::vector<YAML::Node>& active) {
  if (!node.IsDefined()) return Node{};  // e.g. a missing key's lookup: an empty map
  if (node.IsScalar()) return Node{node.Scalar()};
  if (!node.IsSequence() && !node.IsMap()) return Node{};  // null: an empty map
  for (auto const& a : active)
    if (node.is(a)) KEXCEPT(Exception, "bon: cyclic YAML alias");
  active.push_back(node);
  Node out;
  if (node.IsSequence()) {
    Node::List list;
    list.reserve(node.size());
    for (auto const& n : node) list.emplace_back(from_yaml(n, active));
    out = Node{std::move(list)};
  } else {
    Node::Map map;
    for (auto const& kv : node) {
      if (!kv.first.IsScalar()) KEXCEPT(Exception, "bon: a YAML map key that is not a scalar");
      map.push_back({kv.first.Scalar(), from_yaml(kv.second, active)});
    }
    out = Node{std::move(map)};
  }
  active.pop_back();  // a throw leaves active with its caller, which discards it
  return out;
}

}  // namespace detail

// a YAML::Node as a bon::Node: scalars as strings, null / undefined as an empty map; throws a
//  bon::Exception for a cyclic alias or a non scalar map key
inline Node from_yaml(YAML::Node const& node) {
  std::vector<YAML::Node> active;
  return detail::from_yaml(node, active);
}

}  // namespace mkn::kul::bon

#endif  // MKN_KUL_BON_YAML_HPP_
