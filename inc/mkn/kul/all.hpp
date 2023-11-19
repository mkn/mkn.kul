/**
Copyright (c) 2020, Philip Deegan.
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
#ifndef _MKN_KUL_TUPLE_HPP_
#define _MKN_KUL_TUPLE_HPP_

#include <tuple>
#include <vector>
#include <numeric>
#include <algorithm>

#include "mkn/kul/span.hpp"

namespace mkn {
namespace kul {

template <typename Container, typename Function>
void for_each(Container& container, Function&& function) {
  std::for_each(std::begin(container), std::end(container), function);
}

template <typename Container, typename Function>
bool any_of(Container const& container, Function&& function) {
  return std::any_of(std::begin(container), std::end(container), function);
}
template <typename Container, typename Function>
bool any_of(Container&& container, Function&& function) {
  return std::any_of(std::begin(container), std::end(container), function);
}

template <typename Container, typename Function>
bool all_of(Container const& container, Function&& function) {
  return std::all_of(std::begin(container), std::end(container), function);
}
template <typename Container, typename Function>
bool all_of(Container&& container, Function&& function) {
  return all_of(container, function);
}

template <typename T, typename OP, typename Container>
typename std::enable_if_t<is_span_like_v<Container>, bool> compare_to(T const& t, OP const& op,
                                                                      Container const& container) {
  return all_of(container, [&](auto const& arg) { return op(arg, t); });
}

template <typename T, typename OP, typename... Args>
bool compare_to(T const& t, OP const& op, Args const&... args) {
  return ((op(args, t)) && ...);
}
template <typename T, typename OP, typename... Args>
bool compare_to(T&& t, OP&& op, Args&&... args) {
  return compare_to(t, op, args...);
}

template <typename T, typename OP, typename... TupleElements>
bool compare_to(T const& t, OP const& op, std::tuple<TupleElements...> const& tuple) {
  static_assert((... && std::is_arithmetic_v<TupleElements>));
  return std::apply([&](auto const&... args) { return compare_to(t, op, args...); }, tuple);
}
template <typename T, typename OP, typename... TupleElements>
bool compare_to(T&& t, OP&& op, std::tuple<TupleElements...>&& tuple) {
  return compare_to(t, op, tuple);
}

template <typename... Args>
class All {
 public:
  All(Args const&... args) : tuple{args...} {}

  template <typename T>
  bool operator==(T const& t) {
    return are(t, std::equal_to<T>{});
  }
  template <typename T>
  bool operator==(T&& t) {
    return are(t, std::equal_to<T>{});
  }

  template <typename T>
  bool operator!=(T const& t) {
    return are(t, std::not_equal_to<T>{});
  }
  template <typename T>
  bool operator!=(T&& t) {
    return are(t, std::not_equal_to<T>{});
  }

  template <typename T>
  bool operator>(T const& t) {
    return are(t, std::greater<T>{});
  }
  template <typename T>
  bool operator>(T&& t) {
    return are(t, std::greater<T>{});
  }

  template <typename T>
  bool operator<(T const& t) {
    return are(t, std::less<T>{});
  }
  template <typename T>
  bool operator<(T&& t) {
    return are(t, std::less<T>{});
  }

  template <typename T>
  bool operator>=(T const& t) {
    return are(t, std::greater_equal<T>{});
  }
  template <typename T>
  bool operator>=(T&& t) {
    return are(t, std::greater_equal<T>{});
  }

  template <typename T>
  bool operator<=(T const& t) {
    return are(t, std::less_equal<T>{});
  }
  template <typename T>
  bool operator<=(T&& t) {
    return are(t, std::less_equal<T>{});
  }

 private:
  template <typename T, typename OP>
  bool are(T const& t, OP const op) {
    return std::apply([&](auto const&... arg) { return ((compare_to(t, op, arg)) && ...); }, tuple);
  }

  std::tuple<Args...> tuple;
};

}  // namespace kul
}  // namespace mkn

#endif /* _MKN_KUL_TUPLE_HPP_ */
