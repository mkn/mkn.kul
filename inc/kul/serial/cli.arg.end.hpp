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
#ifndef _KUL_SERIAL_CLI_ARG_END_HPP_
#define _KUL_SERIAL_CLI_ARG_END_HPP_

#if defined(_MKN_WITH_IO_CEREAL_)

template <class Archive>
void save(Archive& ar) const {
  // ar(dr, f, q, s, sh, st, u);

  // ar(de, dl, op, ts, wa);
  // ar(aa, al, la, ra, wi, wo);
  // ar(dep);
  // ar(no);

  // auto convert_to_std_set =
  //     [](const kul::hash::set::String& s) -> std::unordered_set<std::string> {
  //   std::unordered_set<std::string> ret;
  //   for (const auto& p : s) ret.emplace(p);
  //   return ret;
  // };
  // auto convert_to_std_map = [](const kul::hash::map::S2S& s)
  //     -> std::unordered_map<std::string, std::string> {
  //       std::unordered_map<std::string, std::string> ret;
  //       for (const auto& p : s) ret.emplace(p.first, p.second);
  //       return ret;
  //     };

  // ar(convert_to_std_set(cmds), convert_to_std_set(wop));
  // ar(convert_to_std_map(evs), convert_to_std_map(jas),
  //    convert_to_std_map(pks));
}
template <class Archive>
void load(Archive& ar) {
  // ar(dr, f, q, s, sh, st, u);

  // ar(de, dl, op, ts, wa);
  // ar(aa, al, la, ra, wi, wo);
  // ar(dep);
  // ar(no);

  // auto convert_to_kul_set =
  //     [](const std::unordered_set<std::string>& s) -> kul::hash::set::String {
  //   kul::hash::set::String ret;
  //   for (const auto& p : s) ret.insert(p);
  //   return ret;
  // };
  // auto convert_to_kul_map =
  //     [](const std::unordered_map<std::string, std::string>& s)
  //     -> kul::hash::map::S2S {
  //       kul::hash::map::S2S ret;
  //       for (const auto& p : s) ret.insert(p.first, p.second);
  //       return ret;
  //     };
  // std::unordered_set<std::string> _cmds, _wop;
  // ar(cmds, _wop);
  // cmds = convert_to_kul_set(_cmds);
  // wop = convert_to_kul_set(_wop);

  // std::unordered_map<std::string, std::string> _evs, _jas, _pks;
  // ar(_evs, _jas, _pks);

  // evs = convert_to_kul_map(_evs);
  // jas = convert_to_kul_map(_jas);
  // pks = convert_to_kul_map(_pks);
}

#endif  //  _MKN_WITH_IO_CEREAL_

#endif  // _KUL_SERIAL_CLI_ARG_END_HPP_