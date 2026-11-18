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
#ifndef MKN_KUL_CONSTEXPRIFIER_HPP_
#define MKN_KUL_CONSTEXPRIFIER_HPP_

#include "mkn/kul/enum.hpp"

#include <tuple>
#include <cstddef>
#include <utility>
#include <variant>
#include <concepts>
#include <type_traits>

namespace mkn::kul {

// a runtime bool as std::false_type / std::true_type in a variant, so std::visit can fan out both
//  cases and the visitor branch with if constexpr. same_as<bool> so nothing converts to bool
inline std::variant<std::false_type, std::true_type> asBoolConstant(
    std::same_as<bool> auto const value) {
  if (value) return std::true_type{};
  return std::false_type{};
}

// how Constexprifier lifts a runtime T into a std::variant of compile-time constants: specialize
//  with a static lift(T). A class template, not overloads, so specializations declared after this
//  header are found when Constexprifier is instantiated
template <typename T>
struct ConstexprLift;  // undefined: T is not liftable

template <>
struct ConstexprLift<bool> {
  static auto lift(bool const value) { return asBoolConstant(value); }
};

template <CountedEnum Enum>
struct ConstexprLift<Enum> {
  static auto lift(Enum const value) { return asEnumConstant(value); }
};

namespace detail {
template <typename T>
using ConstexprVariant_t = decltype(ConstexprLift<T>::lift(std::declval<T const&>()));

template <typename T>
constexpr std::size_t nbrConstexprCases = std::variant_size_v<ConstexprVariant_t<T>>;
}  // namespace detail

// lifts runtime bools / CountedEnums into compile-time constants, for runtime switches inside a
//  hot loop. The visitor is instantiated for the product of every argument's cases, capped at
//  MAX_CONSTEXPR_PERMUTATIONS
//
//   enum class Mode { A, B, C, count };
//   Constexprifier{aCondition, aMode}([&]<bool condition, Mode mode>() {
//     for (std::size_t i = 0; i < 10; ++i) {
//       if constexpr (condition) { ... }
//       if constexpr (mode == Mode::B) { ... }
//     }
//   });
template <typename... Args>
struct Constexprifier {
  static constexpr std::size_t MAX_CONSTEXPR_PERMUTATIONS = 64;
  static constexpr std::size_t permutations =
      (std::size_t{1} * ... * detail::nbrConstexprCases<Args>);
  static_assert(permutations <= MAX_CONSTEXPR_PERMUTATIONS,
                "Constexprifier: permutation budget exceeded");

  explicit Constexprifier(Args const&... args) : values{args...} {}

  void operator()(auto&& fn) const {
    std::apply(
        [&](auto const&... vs) {
          std::visit([&](auto... tags) { fn.template operator()<decltype(tags)::value...>(); },
                     ConstexprLift<Args>::lift(vs)...);
        },
        values);
  }

  std::tuple<Args...> values;
};

template <typename... Args>
Constexprifier(Args const&...) -> Constexprifier<Args...>;

}  // namespace mkn::kul

#endif /* MKN_KUL_CONSTEXPRIFIER_HPP_ */
