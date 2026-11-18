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
#ifndef MKN_KUL_FOR_HPP_
#define MKN_KUL_FOR_HPP_

#include "mkn/kul/lang/cpp/compiler/def.hpp"
#include "mkn/kul/meta.hpp"
#include "mkn/kul/except.hpp"

#include <array>
#include <tuple>
#include <vector>
#include <utility>
#include <type_traits>
#include <cstddef>
#include <cstdint>

namespace mkn::kul {

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

// a equals every / any of ts
MKN_KUL_FN_SIG_ALL_RET auto constexpr all_in(auto const a, auto&&... ts) {
  return ((a == ts) && ...);
}

MKN_KUL_FN_SIG_ALL_RET auto constexpr any_in([[maybe_unused]] auto const a, auto&&... ts) {
  return ((a == ts) || ...);
}

// calls operator bool() or copies bool
auto constexpr static to_bool = [](auto const& v) { return bool{v}; };

// fn(element) for every / any / no element, stopping at the first that decides it: loops, not
//  std::all_of ..., so device code can use them
template <typename Container, typename Fn = decltype(to_bool)>
MKN_KUL_FN_SIG_ALL_RET constexpr bool all(Container const& container, Fn fn = to_bool) {
  for (auto const& e : container)
    if (!fn(e)) return false;
  return true;
}

template <typename Container, typename Fn = decltype(to_bool)>
MKN_KUL_FN_SIG_ALL_RET constexpr bool any(Container const& container, Fn fn = to_bool) {
  for (auto const& e : container)
    if (fn(e)) return true;
  return false;
}

template <typename Container, typename Fn = decltype(to_bool)>
MKN_KUL_FN_SIG_ALL_RET constexpr bool none(Container const& container, Fn fn = to_bool) {
  return !any(container, fn);
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

template <std::uint16_t N, typename Fn>
MKN_KUL_FN_SIG_ALL constexpr decltype(auto) for_N_fold(Fn&& fn) {
  /*  // how to use: fn gets every index at once, as a pack
      for_N_fold<3>([&](auto... ics) { return array(idx[ics]...); });
  */

  return [&]<std::uint16_t... Is>(std::integer_sequence<std::uint16_t, Is...>) -> decltype(auto) {
    return fn(std::integral_constant<std::uint16_t, Is>{}...);
  }(std::make_integer_sequence<std::uint16_t, N>{});
}

enum class for_N_R_mode {
  make_tuple = 0,
  make_array,
  forward_tuple,
};

template <std::uint16_t N, auto M = for_N_R_mode::make_tuple, typename Fn>
MKN_KUL_FN_SIG_ALL constexpr auto for_N(Fn&& fn) {
  /*  // how to use
      for_N<2>([](auto ic) {
          constexpr auto i = ic();
          // ...
      });
  */

  static_assert(std::is_same_v<decltype(M), for_N_R_mode>);
  using return_type =
      std::decay_t<std::invoke_result_t<Fn&, std::integral_constant<std::uint16_t, 0>>>;

  return for_N_fold<N>([&](auto... ics) {
    if constexpr (std::is_void_v<return_type>)
      (fn(ics), ...);
    else if constexpr (M == for_N_R_mode::make_tuple)
      return std::make_tuple(fn(ics)...);
    else if constexpr (M == for_N_R_mode::make_array)
      // no CTAD: HIP sees std::array's deduction guide as host only in device code
      return std::array<return_type, sizeof...(ics)>{fn(ics)...};
    else if constexpr (M == for_N_R_mode::forward_tuple)
      return std::forward_as_tuple(fn(ics)...);
    else
      []<bool flag = false>() { static_assert(flag, "unknown return mode"); }();
  });
}

template <std::uint16_t N, typename Fn>
MKN_KUL_FN_SIG_ALL_RET constexpr auto for_N_make_array(Fn&& fn) {
  return for_N<N, for_N_R_mode::make_array>(fn);
}

// fn(ic) for every / any index, stopping at the first false / true: a fold over the indices, not
//  all / any over an array, nvcc 12 errors on all / any's constrained overload
template <std::uint16_t N, typename Fn>
MKN_KUL_FN_SIG_ALL_RET constexpr auto for_N_all(Fn&& fn) {
  return for_N_fold<N>([&](auto... ics) { return (static_cast<bool>(fn(ics)) && ...); });
}

template <std::uint16_t N, typename Fn>
MKN_KUL_FN_SIG_ALL_RET constexpr auto for_N_any(Fn&& fn) {
  return for_N_fold<N>([&](auto... ics) { return (static_cast<bool>(fn(ics)) || ...); });
}

template <typename F>
auto generate_from(F&& f, std::size_t const from, std::size_t const to) {
  if (from > to)
    KEXCEPTION(
        "RuntimeError: mkn::kul::generate_from(F&&, from, to) - 'from' must be less than or equal "
        "to 'to'");
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

template <std::size_t Idx, typename F>
auto constexpr generate_array_at_(F& f, auto&& arr) {
  return f(arr[Idx]);
}

template <typename F, std::size_t... Is>
auto constexpr generate_array_(F& f, auto&& arr, std::integer_sequence<std::size_t, Is...>) {
  return std::array{generate_array_at_<Is>(f, arr)...};
}

template <typename F, typename Type, std::size_t Size>
auto constexpr generate_from(F&& f, std::array<Type, Size> const& arr) {
  return generate_array_(f, arr, std::make_integer_sequence<std::size_t, Size>{});
}

template <typename F, typename Type, std::size_t Size>
auto constexpr generate_from(F&& f, std::array<Type, Size>& arr) {
  return generate_array_(f, arr, std::make_integer_sequence<std::size_t, Size>{});
}

}  // namespace mkn::kul

#endif /* MKN_KUL_FOR_HPP_ */
