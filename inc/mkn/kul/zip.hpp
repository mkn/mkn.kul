/**
Copyright (c) 2023, Philip Deegan.
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
#ifndef _MKN_KUL_IO_HPP_
#define _MKN_KUL_IO_HPP_

#include <iostream>
#include <tuple>

namespace mkn::kul {

template <std::size_t Is, typename Tuple>
struct Element {
  using C = std::tuple_element_t<Is, Tuple>;
  using T = typename C::value_type;  // const containers do not have const value_types
  using value_type = std::conditional_t<std::is_const_v<C>, T const&, T&>;
};

template <typename Tuple, std::size_t... Is>
constexpr auto tuple_element_value_type_refs_(Tuple, std::index_sequence<Is...> const&&)
    -> std::tuple<typename Element<Is, Tuple>::value_type...>;

template <typename... Args>
auto constexpr tuple_element_value_type_refs()
    -> decltype(tuple_element_value_type_refs_(std::tuple<Args...>{},
                                               std::make_index_sequence<sizeof...(Args)>{}));

template <typename Tuple, std::size_t... Is>
constexpr auto get_tuple_element_value_type_refs_(Tuple& tup, std::size_t index,
                                                  std::index_sequence<Is...> const&&) {
  return std::forward_as_tuple(std::get<Is>(tup)[index]...);
}
template <typename Tuple>
auto constexpr get_tuple_element_value_type_refs(Tuple& tuple, std::size_t index) {
  return get_tuple_element_value_type_refs_(tuple, index,
                                            std::make_index_sequence<std::tuple_size_v<Tuple>>{});
}

template <typename... Args>
struct Zipit {
  auto operator*() { return get_tuple_element_value_type_refs(args, idx); }
  bool operator==(Zipit const& that) const { return idx == that.idx; }
  bool operator!=(Zipit const& that) const { return !(*(this) == that); }
  auto& operator++() {
    ++idx;
    return *this;
  }

  std::size_t idx = 0;
  std::tuple<Args...>& args;
};

template <typename... Args>
struct Zipper {
  using Iter = Zipit<Args...>;

  auto begin() { return Iter{0, tup}; }
  auto end() { return Iter{std::get<0>(tup).size(), tup}; }

  std::tuple<Args...> tup;
};

template <typename... Args>
auto zip(Args&... args) {
  return Zipper<Args...>{std::forward_as_tuple(args...)};
}

}  // namespace mkn::kul

#endif /* _MKN_KUL_IO_HPP_ */
