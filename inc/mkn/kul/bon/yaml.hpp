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

#include <string>

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

}  // namespace mkn::kul::bon

#endif  // MKN_KUL_BON_YAML_HPP_
