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
#ifndef _MKN_KUL_FOR_HPP_
#define _MKN_KUL_FOR_HPP_

#include <tuple>

namespace mkn {
namespace kul {

template<typename T = std::size_t>
struct Apply {
  template<size_t i>
  constexpr auto operator()(){ return std::integral_constant<T, i>{}; }
};

template <typename Apply, size_t... Is>
constexpr auto apply_N(Apply& f, std::integer_sequence<size_t, Is...> const&) {
  if constexpr (!std::is_same_v<decltype(f.template operator()<0>()), void>)
    return std::make_tuple(f.template operator()<Is>()...);
  (f.template operator()<Is>(), ...);
}
template <size_t N, typename Apply>
constexpr auto apply_N(Apply&& f) {
  return apply_N(f, std::make_integer_sequence<size_t, N>{});
}

template <size_t N, typename Fn>
constexpr void for_N(Fn&& fn) {
/*
    for_N<2>([](auto ic) {
        constexpr auto i = ic();
        // ...
    });
*/

  std::apply([&](auto ... ics)  { (fn(ics), ...);}, apply_N<N>(Apply{}));
}



}  // namespace kul
}  // namespace mkn

#endif /* _MKN_KUL_FOR_HPP_ */
