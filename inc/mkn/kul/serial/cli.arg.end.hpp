/**
Copyright (c) 2017, Philip Deegan.
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
#ifndef _MKN_KUL_SERIAL_CLI_ARG_END_HPP_
#define _MKN_KUL_SERIAL_CLI_ARG_END_HPP_

#if defined(_MKN_WITH_IO_CEREAL_)

template <class Archive>
void save(Archive& ar) const {
  auto convert_to_std_map =
      [](mkn::kul::hash::map::S2S const& s) -> std::unordered_map<std::string, std::string> {
    std::unordered_map<std::string, std::string> ret;
    for (auto const& p : s) ret.emplace(p.first, p.second);
    return ret;
  };
  ar(convert_to_std_map(vals));
}
template <class Archive>
void load(Archive& ar) {
  auto convert_to_kul_map =
      [](std::unordered_map<std::string, std::string> const& s) -> mkn::kul::hash::map::S2S {
    mkn::kul::hash::map::S2S ret;
    for (auto const& p : s) ret.insert(p.first, p.second);
    return ret;
  };
  std::unordered_map<std::string, std::string> _vals;
  ar(_vals);
  vals = convert_to_kul_map(_vals);
}

#endif  //  _MKN_WITH_IO_CEREAL_

#endif  // _MKN_KUL_SERIAL_CLI_ARG_END_HPP_
