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
#ifndef MKN_KUL_META_HPP_
#define MKN_KUL_META_HPP_

#include "mkn/kul/lang/cpp/compiler/def.hpp"

#include <array>
#include <tuple>
#include <ranges>
#include <vector>
#include <cstddef>
#include <utility>
#include <concepts>
#include <type_traits>

namespace mkn::kul {

// false, but only once T is known: static_assert(dependent_false_v<T>) in a discarded branch
template <typename T>
inline constexpr bool dependent_false_v = false;

template <typename T, typename... Ts>
MKN_KUL_FN_SIG_RET auto constexpr all_are(Ts&&...) {
  return ((std::is_same_v<T, std::decay_t<Ts>>) && ...);
}

template <typename T, typename... Ts>
MKN_KUL_FN_SIG_RET auto constexpr any_are(Ts&&...) {
  return ((std::is_same_v<T, std::decay_t<Ts>>) || ...);
}

template <typename... Ts>
MKN_KUL_FN_SIG_RET auto constexpr is_any_of(auto const& t) {
  return ((std::is_same_v<Ts, std::decay_t<decltype(t)>>) || ...);
}

template <typename T, typename... Ts>
MKN_KUL_FN_SIG_RET auto constexpr is_any_of() {
  return ((std::is_same_v<Ts, T>) || ...);
}

// compiles only if every argument has the same type
template <typename T>
constexpr void allsame(T) {}

template <typename T, typename T2, typename... Ts>
  requires std::same_as<T, T2>
constexpr void allsame([[maybe_unused]] T arg, T2 arg2, Ts... args) {
  allsame(arg2, args...);
}

template <typename T, std::size_t... Is>
constexpr auto gft_helper(std::index_sequence<Is...> const&&)
    -> decltype(std::make_tuple((Is, std::declval<T>())...));

template <typename T, std::size_t N>
constexpr auto get_fixed_tuple() -> decltype(gft_helper<T>(std::make_index_sequence<N>{}));

// std::tuple of N T
template <typename T, std::size_t N>
using tuple_fixed_type = decltype(get_fixed_tuple<T, N>());

// brace initializes T from constructor arguments, so aggregates work with
//  std::make_shared / std::make_unique / emplace
template <typename T>
struct aggregate_adapter : public T {
  template <class... Args>
  aggregate_adapter(Args&&... args) : T{std::forward<Args>(args)...} {}
};

// the object, whether given a pointer to it or the object
template <typename Type>
MKN_KUL_FN_SIG_RET auto& deref(Type&& type) {
  if constexpr (std::is_pointer_v<std::decay_t<Type>>)
    return *type;
  else
    return type;
}

template <typename T>
struct DekayT {
  T t;
  using value_type = std::decay_t<decltype(t)>;
};

template <typename T>
DekayT(T t) -> DekayT<T>;

template <typename T>
concept IsStdVector =
    std::same_as<T, std::vector<typename T::value_type, typename T::allocator_type>>;

template <typename T>  // any size, tuple_size first so other types are false, not errors
concept IsStdArray = requires { std::tuple_size<T>::value; } and
                     std::same_as<T, std::array<typename T::value_type, std::tuple_size<T>::value>>;

template <typename T>
concept Iterable = std::ranges::range<T>;

template <typename T, typename Index = int>
concept Subscriptable = requires(T t, Index i) { t[i]; };

}  // namespace mkn::kul

#endif /* MKN_KUL_META_HPP_ */
