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

#include <cstddef>
#include <concepts>
#include <iterator>
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

    constexpr iterator() = default;
    constexpr explicit iterator(T const v) : v_{v} {}

    constexpr T operator*() const { return v_; }
    constexpr iterator& operator++() {
      ++v_;
      return *this;
    }
    constexpr iterator operator++(int) {
      auto const copy = *this;
      ++v_;
      return copy;
    }
    constexpr bool operator==(iterator const&) const = default;

   private:
    T v_{};
  };

  constexpr Range(T const start, T const stop)
      : start_{start}, stop_{stop < start ? start : stop} {}

  constexpr iterator begin() const { return iterator{start_}; }
  constexpr iterator end() const { return iterator{stop_}; }
  constexpr std::size_t size() const { return static_cast<std::size_t>(stop_ - start_); }
  constexpr bool empty() const { return start_ == stop_; }

 private:
  T start_, stop_;
};

// T is not deduced from the arguments, so range(10) is std::size_t, range<int>(10) is int
template <std::integral T = std::size_t>
constexpr auto range(std::type_identity_t<T> const stop) {
  return Range<T>{T{0}, stop};
}

template <std::integral T = std::size_t>
constexpr auto range(std::type_identity_t<T> const start, std::type_identity_t<T> const stop) {
  return Range<T>{start, stop};
}

}  // namespace mkn::kul

#endif /* MKN_KUL_RANGE_HPP_ */
