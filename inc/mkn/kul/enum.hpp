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
#ifndef MKN_KUL_ENUM_HPP_
#define MKN_KUL_ENUM_HPP_

#include "mkn/kul/for.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <variant>
#include <stdexcept>
#include <type_traits>
#include <initializer_list>
#include <iterator>

namespace mkn::kul {

// an enum ending with a count sentinel, so every value can be enumerated
template <typename Enum>
concept CountedEnum = std::is_enum_v<Enum> and requires { Enum::count; };

// declared only, its return type spells EnumConstantVariant_t
template <typename Enum, std::size_t... Is>
auto enum_constant_variant(std::index_sequence<Is...>)
    -> std::variant<std::integral_constant<Enum, static_cast<Enum>(Is)>...>;

// std::variant of a std::integral_constant per value of a CountedEnum
template <CountedEnum Enum>
using EnumConstantVariant_t = decltype(enum_constant_variant<Enum>(
    std::make_index_sequence<static_cast<std::size_t>(Enum::count)>{}));

// the compile-time constant of a runtime enum value, throws if value is not below Enum::count
template <CountedEnum Enum>
EnumConstantVariant_t<Enum> asEnumConstant(Enum const value) {
  return for_N_fold<static_cast<std::uint16_t>(Enum::count)>([&](auto... is) {
    EnumConstantVariant_t<Enum> result;
    bool found = false;
    (void)(((value == static_cast<Enum>(is()))
                ? (result = std::integral_constant<Enum, static_cast<Enum>(is())>{}, found = true,
                   true)
                : false) ||
           ...);
    if (!found) throw std::runtime_error("mkn::kul::asEnumConstant: enum value out of range");
    return result;
  });
}

// std::array indexed by Enum from {key, value} entries in any order, every key in
//  [0, Enum::count) must appear exactly once: in a constant expression a missing, duplicate or
//  out of range key is a compile error
template <CountedEnum Enum, typename Value>
constexpr auto make_enum_table(std::initializer_list<std::pair<Enum, Value>> const entries) {
  constexpr auto size = static_cast<std::size_t>(Enum::count);

  std::array<Value, size> table{};
  std::array<bool, size> isSet{};

  for (auto const& [key, value] : entries) {
    auto const idx = static_cast<std::size_t>(key);
    if (idx >= size) throw std::runtime_error("mkn::kul::make_enum_table: key out of range");
    if (isSet[idx]) throw std::runtime_error("mkn::kul::make_enum_table: duplicate key");
    table[idx] = value;
    isSet[idx] = true;
  }

  for (auto const set : isSet)
    if (!set) throw std::runtime_error("mkn::kul::make_enum_table: missing key");

  return table;
}

}  // namespace mkn::kul

#endif /* MKN_KUL_ENUM_HPP_ */
