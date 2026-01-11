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
#ifndef _MKN_KUL_FOR_HPP_
#define _MKN_KUL_FOR_HPP_

#include <array>
#include <tuple>
#include <vector>
#include <utility>
#include <cassert>
#include <algorithm>
#include <stdexcept>
#include <type_traits>
#include "mkn/kul/except.hpp"

namespace mkn::kul {

template <typename T, typename... Ts>
auto constexpr all_are(Ts&&... ts) {
  return ((std::is_same_v<T, std::decay_t<decltype(ts)>>) && ...);
}

auto constexpr any(auto... bools)
  requires(all_are<bool>(bools...))
{
  return (bools || ...);
}

auto constexpr all(auto... bools)
  requires(all_are<bool>(bools...))
{
  return (bools && ...);
}

// calls operator bool() or copies bool
auto constexpr static to_bool = [](auto const& v) { return bool{v}; };

template <typename Container, typename Fn = decltype(to_bool)>
auto constexpr all(Container const& container, Fn fn = to_bool) {
  return std::all_of(container.begin(), container.end(), fn);
}

template <typename Container, typename Fn = decltype(to_bool)>
auto constexpr any(Container const& container, Fn fn = to_bool) {
  return std::any_of(container.begin(), container.end(), fn);
}

template <typename Container, typename Fn = decltype(to_bool)>
auto constexpr none(Container const& container, Fn fn = to_bool) {
  return std::none_of(container.begin(), container.end(), fn);
}

template <typename T = std::size_t>
struct Apply {
  template <std::size_t i>
  constexpr auto operator()() {
    return std::integral_constant<T, i>{};
  }
};

template <typename Apply, std::size_t... Is>
constexpr auto apply_N(Apply& f, std::integer_sequence<std::size_t, Is...> const&) {
  if constexpr (!std::is_same_v<decltype(f.template operator()<0>()), void>)
    return std::make_tuple(f.template operator()<Is>()...);
  (f.template operator()<Is>(), ...);
}
template <std::size_t N, typename Apply>
constexpr auto apply_N(Apply&& f) {
  return apply_N(f, std::make_integer_sequence<std::size_t, N>{});
}

enum class for_N_R_mode {
  make_tuple = 0,
  make_array,
  forward_tuple,
};

template <std::size_t N, auto M = for_N_R_mode::make_tuple, typename Fn>
constexpr auto for_N(Fn& fn) {
  /*  // how to use
      for_N<2>([](auto ic) {
          constexpr auto i = ic();
          // ...
      });
  */

  static_assert(std::is_same_v<decltype(M), for_N_R_mode>);
  using return_type =
      std::decay_t<std::invoke_result_t<Fn, std::integral_constant<std::size_t, 0>>>;
  constexpr bool returns = !std::is_same_v<return_type, void>;

  if constexpr (returns) {
    return std::apply(
        [&](auto... ics) {
          if constexpr (M == for_N_R_mode::make_tuple)
            return std::make_tuple(fn(ics)...);
          else if constexpr (M == for_N_R_mode::make_array)
            return std::array{fn(ics)...};
          else if constexpr (M == for_N_R_mode::forward_tuple)
            return std::forward_as_tuple(fn(ics)...);
          else
            []<bool flag = false>() { static_assert(flag, "unknown return mode"); }();
        },
        apply_N<N>(Apply{}));
  } else
    std::apply([&](auto... ics) { (fn(ics), ...); }, apply_N<N>(Apply{}));
}

template <std::size_t N, auto M = for_N_R_mode::make_tuple, typename Fn>
constexpr auto for_N(Fn&& fn) {
  return for_N<N, M>(fn);
}

template <std::size_t N, typename Fn>
constexpr auto for_N_make_array(Fn&& fn) {
  return for_N<N, for_N_R_mode::make_array>(fn);
}

template <std::size_t N, typename Fn>
constexpr auto for_N_all(Fn&& fn) {
  return all(for_N<N, for_N_R_mode::make_array>(fn));
}

template <std::size_t N, typename Fn>
constexpr auto for_N_any(Fn&& fn) {
  return any(for_N<N, for_N_R_mode::make_array>(fn));
}

template <typename F>
auto generate_from(F&& f, std::size_t const from, std::size_t const to) {
  if (from > to)
    KEXCEPTION(
        "RuntimeError: mkn::kul::generate_from(F&&, from, to) - 'from' must be less than 'to'");
  using value_type = std::decay_t<std::invoke_result_t<F&, std::size_t const&>>;
  std::vector<value_type> v;
  std::size_t count = to - from;
  if (count > 0) v.reserve(count);
  for (std::size_t i = from; i < to; ++i) v.emplace_back(f(i));
  return v;
}

template <typename F>
auto generate_from(F&& f, std::size_t const count) {
  return generate_from(std::forward<F>(f), 0, count);
}

template <typename Type, typename F, typename Container>
auto _generate_container_from(F&& f, Container& container) {
  using value_type = std::decay_t<std::invoke_result_t<F&, Type>>;
  std::vector<value_type> vec;
  vec.reserve(container.size());
  for (auto& v : container) vec.emplace_back(f(v));
  return vec;
}

template <typename F, typename Container>
auto generate_from(F&& f, Container& container) {
  return _generate_container_from<typename Container::value_type&>(std::forward<F>(f), container);
}

template <typename F, typename Container>
auto generate_from(F&& f, Container const& container) {
  return _generate_container_from<typename Container::value_type const&>(std::forward<F>(f),
                                                                         container);
}

template <typename F, typename T>
auto generate_from(F&& f, std::vector<T>&& v) {
  return generate_from(std::forward<F>(f), v);
}

template <std::size_t Idx, typename F, typename Type, std::size_t Size>
auto constexpr generate_array__(F& f, std::array<Type, Size> const& arr) {
  return f(arr[Idx]);
}

template <typename Type, std::size_t Size, typename F, std::size_t... Is>
auto constexpr generate_array_(F& f, std::array<Type, Size> const& arr,
                               std::integer_sequence<std::size_t, Is...>) {
  return std::array{generate_array__<Is>(f, arr)...};
}

template <typename F, typename Type, std::size_t Size>
auto constexpr generate_from(F&& f, std::array<Type, Size> const& arr) {
  return generate_array_(f, arr, std::make_integer_sequence<std::size_t, Size>{});
}

}  // namespace mkn::kul

#endif /* _MKN_KUL_FOR_HPP_ */
