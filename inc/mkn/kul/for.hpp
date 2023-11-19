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

template <typename T = std::size_t>
struct Apply {
  template <size_t i>
  constexpr auto operator()() {
    return std::integral_constant<T, i>{};
  }
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

  std::apply([&](auto... ics) { (fn(ics), ...); }, apply_N<N>(Apply{}));
}

template <typename F>
auto generate(F&& f, std::size_t from, std::size_t to) {
  using value_type = std::decay_t<std::result_of_t<F&(std::size_t const&)>>;
  std::vector<value_type> v;
  std::size_t count = to - from;
  if (count > 0) v.reserve(count);
  for (std::size_t i = from; i < to; ++i) v.emplace_back(f(i));
  return v;
}

template <typename F>
auto generate(F&& f, std::size_t count) {
  return generate(std::forward<F>(f), 0, count);
}

template <typename F, typename Container>
auto generate(F&& f, Container& container) {
  using T = typename Container::value_type;
  using value_type = std::decay_t<std::result_of_t<F&(T&)>>;
  std::vector<value_type> v1;
  if (container.size() > 0) v1.reserve(container.size());
  for (auto& v : container) v1.emplace_back(f(v));
  return v1;
}

template <std::size_t Idx, typename F, typename Type, std::size_t Size>
auto constexpr generate_array__(F& f, std::array<Type, Size>& arr) {
  return f(arr[Idx]);
}

template <typename Type, std::size_t Size, typename F, std::size_t... Is>
auto constexpr generate_array_(F& f, std::array<Type, Size>& arr,
                               std::integer_sequence<std::size_t, Is...>) {
  return std::array{generate_array__<Is>(f, arr)...};
}

template <typename F, typename Type, std::size_t Size>
auto constexpr generate(F&& f, std::array<Type, Size>& arr) {
  return generate_array_(f, arr, std::make_integer_sequence<std::size_t, Size>{});
}

template <typename F, typename Container>
auto generate(F&& f, Container&& v) {
  return generate(std::forward<F>(f), v);
}

}  // namespace kul
}  // namespace mkn

#endif /* _MKN_KUL_FOR_HPP_ */
