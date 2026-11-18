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
#ifndef MKN_KUL_RANGE_HPP_
#define MKN_KUL_RANGE_HPP_

#include "mkn/kul/lang/cpp/compiler/def.hpp"

#include <tuple>
#include <cstddef>
#include <concepts>
#include <iterator>
#include <utility>
#include <type_traits>

namespace mkn::kul {

// Half-open [start, stop) integer sequence for range-based for loops
//   for (auto const i : mkn::kul::range(10)) // 0..9 as std::size_t
//   for (auto const i : mkn::kul::range<int>(-2, 3)) // -2..2 as int
// stop < start is an empty range.
template <std::integral T>
class Range {
 public:
  class iterator {
   public:
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::forward_iterator_tag;

    MKN_KUL_FN_SIG constexpr iterator() = default;
    MKN_KUL_FN_SIG constexpr explicit iterator(T const v) : v_{v} {}

    MKN_KUL_FN_SIG constexpr T operator*() const { return v_; }
    MKN_KUL_FN_SIG constexpr iterator& operator++() {
      ++v_;
      return *this;
    }
    MKN_KUL_FN_SIG constexpr iterator operator++(int) {
      auto const copy = *this;
      ++v_;
      return copy;
    }
    MKN_KUL_FN_SIG constexpr bool operator==(iterator const&) const = default;

   private:
    T v_{};
  };

  MKN_KUL_FN_SIG constexpr Range(T const start, T const stop)
      : start_{start}, stop_{stop < start ? start : stop} {}

  MKN_KUL_FN_SIG constexpr iterator begin() const { return iterator{start_}; }
  MKN_KUL_FN_SIG constexpr iterator end() const { return iterator{stop_}; }
  MKN_KUL_FN_SIG constexpr std::size_t size() const {
    return static_cast<std::size_t>(stop_ - start_);
  }
  MKN_KUL_FN_SIG constexpr bool empty() const { return start_ == stop_; }

 private:
  T start_, stop_;
};

// T is not deduced from the arguments, so range(10) is std::size_t, range<int>(10) is int.
//  std::size_t by default: 32-bit indices can stop loops autovectorizing
template <std::integral T = std::size_t>
MKN_KUL_FN_SIG_RET constexpr auto range(std::type_identity_t<T> const stop) {
  return Range<T>{T{0}, stop};
}

template <std::integral T = std::size_t>
MKN_KUL_FN_SIG_RET constexpr auto range(std::type_identity_t<T> const start,
                                        std::type_identity_t<T> const stop) {
  return Range<T>{start, stop};
}

// Each element of a container with its index, for range-based for loops
//   for (auto&& [i, o] : mkn::kul::enumerate(objects)) // i: std::size_t, o: objects[i]
//   for (auto&& [i, o] : mkn::kul::enumerate<int>(objects)) // i: int
// o refers to the element, so it can be assigned through. A temporary container is moved into
//  the view and lives for the loop.
template <typename Container, std::integral Index = std::size_t>
class Enumerate {
  using base_iterator = decltype(std::begin(std::declval<Container&>()));
  using base_sentinel = decltype(std::end(std::declval<Container&>()));

 public:
  class sentinel {
   public:
    MKN_KUL_FN_SIG constexpr explicit sentinel(base_sentinel e) : end_{e} {}

   private:
    friend class Enumerate;
    base_sentinel end_;
  };

  class iterator {
   public:
    using reference = decltype(*std::declval<base_iterator&>());
    using value_type = std::tuple<Index, reference>;
    using difference_type = std::ptrdiff_t;

    MKN_KUL_FN_SIG constexpr iterator(Index const i, base_iterator it) : i_{i}, it_{it} {}

    MKN_KUL_FN_SIG constexpr value_type operator*() const { return value_type{i_, *it_}; }
    MKN_KUL_FN_SIG constexpr iterator& operator++() {
      ++i_;
      ++it_;
      return *this;
    }
    MKN_KUL_FN_SIG constexpr iterator operator++(int) {
      auto const copy = *this;
      ++*this;
      return copy;
    }
    MKN_KUL_FN_SIG constexpr bool operator==(sentinel const& s) const { return it_ == s.end_; }

   private:
    Index i_;
    base_iterator it_;
  };

  MKN_KUL_FN_SIG constexpr explicit Enumerate(Container&& c)
      : container_{std::forward<Container>(c)} {}

  MKN_KUL_FN_SIG constexpr iterator begin() { return iterator{Index{0}, std::begin(container_)}; }
  MKN_KUL_FN_SIG constexpr sentinel end() { return sentinel{std::end(container_)}; }

 private:
  Container container_;  // a reference for an lvalue, the moved container for a temporary
};

// Index is not deduced from the container, enumerate(v) counts in std::size_t, as range
template <std::integral Index = std::size_t, typename Container>
MKN_KUL_FN_SIG_RET constexpr auto enumerate(Container&& container) {
  return Enumerate<Container, Index>{std::forward<Container>(container)};
}

}  // namespace mkn::kul

#endif /* MKN_KUL_RANGE_HPP_ */
