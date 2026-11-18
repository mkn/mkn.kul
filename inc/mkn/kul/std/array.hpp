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
#ifndef MKN_KUL_STD_ARRAY_HPP_
#define MKN_KUL_STD_ARRAY_HPP_

#include "mkn/kul/for.hpp"
#include "mkn/kul/lang/cpp/compiler/def.hpp"

#include <array>
#include <cstddef>
#include <utility>
#include <algorithm>

namespace mkn::kul {

template <typename Type, std::size_t Size>
MKN_KUL_FN_SIG_ALL constexpr void fill(Type value, std::array<Type, Size>& array) {
  for (std::size_t i = 0; i < Size; i++) array[i] = value;
}

// the first To_Size values of from
template <std::size_t To_Size, std::size_t From_Size, typename Type>
MKN_KUL_FN_SIG_ALL_RET constexpr std::array<Type, To_Size> sized_array(
    std::array<Type, From_Size> const& from) {
  static_assert(To_Size <= From_Size, "invalid sized_array Size template, too large");

  if constexpr (To_Size == From_Size)
    return from;
  else {
    std::array<Type, To_Size> to{};
    for (std::size_t i = 0; i < to.size(); i++) to[i] = from[i];
    return to;
  }
}

template <std::size_t To_Size, typename... Args>
MKN_KUL_FN_SIG_ALL_RET constexpr auto as_sized_array(Args&&... args) {
  return sized_array<To_Size>(std::array{std::forward<Args>(args)...});
}

// size values of val
template <typename Type, std::size_t size>
MKN_KUL_FN_SIG_ALL_RET constexpr std::array<Type, size> ConstArray(Type val = 0) {
  std::array<Type, size> arr{};
  for (std::size_t i = 0; i < size; i++) arr[i] = val;
  return arr;
}

// size values from calling fn each time
template <std::size_t size, typename FN>
MKN_KUL_FN_SIG_ALL_RET constexpr auto ConstArrayFrom(FN fn) {
  std::array<decltype(fn()), size> arr{};
  for (std::size_t i = 0; i < size; i++) arr[i] = fn();
  return arr;
}

template <typename T0, typename T1, std::size_t... Is>
MKN_KUL_FN_SIG_ALL_RET constexpr bool _array_equals(T0 const& a, T1 const& b,
                                                    std::index_sequence<Is...> const&&) {
  return (... && (a[Is] == b[Is]));
}

// std::array operator== is not usable in device code
template <typename T, std::size_t S>
MKN_KUL_FN_SIG_ALL_RET constexpr bool array_equals(std::array<T, S> const& a,
                                                   std::array<T, S> const& b) {
  return _array_equals(a, b, std::make_index_sequence<S>{});
}

template <typename As, typename T, std::size_t... Is>
MKN_KUL_FN_SIG_ALL_RET constexpr auto _array_minus(T const& a, T const& b,
                                                   std::index_sequence<Is...> const&&) {
  std::array<As, sizeof...(Is)> arr{};
  ((arr[Is] = a[Is] - b[Is]), ...);
  return arr;
}

// a - b per element, as As
template <typename As, typename T, std::size_t S>
MKN_KUL_FN_SIG_ALL_RET constexpr auto array_minus(std::array<T, S> const& a,
                                                  std::array<T, S> const& b) {
  return _array_minus<As>(a, b, std::make_index_sequence<S>{});
}

// element operations for array_op, constructed on the destination then called with the source
template <typename D>
struct SetEqual {
  using value_type = D;
  MKN_KUL_FN_SIG_ALL void operator()(auto const& d0) { d = d0; }
  D& d;
};

template <typename D>
struct PlusEquals {
  using value_type = D;
  MKN_KUL_FN_SIG_ALL void operator()(auto const& d0) { d += d0; }
  D& d;
};

template <typename D>
struct SetMax {
  using value_type = D;
  MKN_KUL_FN_SIG_ALL void operator()(auto& d0) { d = (std::max)(d, d0); }
  D& d;
};

template <typename D>
struct PlusEqualsProduct {
  MKN_KUL_FN_SIG_ALL void operator()(auto& d0, auto& o) { d += d0 * o; }
  D& d;
};

// Op{a[i]}(b[i]) per element, e.g. array_op<PlusEquals<double>>(a, b)
template <typename Op, template <typename, std::size_t> typename A0,
          template <typename, std::size_t> typename A1, typename T, std::size_t S>
MKN_KUL_FN_SIG_ALL void array_op(A0<T, S>& a, A1<T, S> const& b) {
  for_N<S>([&](auto i) { Op{a[i]}(b[i]); });
}

// Op{a[i]}(b) per element
template <typename Op, template <typename, std::size_t> typename A0, typename T, std::size_t S>
MKN_KUL_FN_SIG_ALL void array_op(A0<T, S>& a, T const& b) {
  for_N<S>([&](auto i) { Op{a[i]}(b); });
}

}  // namespace mkn::kul

#endif /* MKN_KUL_STD_ARRAY_HPP_ */
