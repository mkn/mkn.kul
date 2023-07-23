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
#ifndef _MKN_KUL_SPAN_HPP_
#define _MKN_KUL_SPAN_HPP_

#include <vector>
#include <numeric>
#include "mkn/kul/decltype.hpp"

namespace mkn {
namespace kul {


template <typename T, typename data_fn = void, typename size_fn = void>
struct is_span_like : std::false_type {};

template <typename T>
struct is_span_like<T, func::check<decltype(std::declval<T>().data())>,
                    func::check<decltype(std::declval<T>().size())>> : std::true_type {};

template <typename T>
auto constexpr is_span_like_v = is_span_like<T>::value;

template <typename T, typename SIZE = size_t>
class Span {
 public:
  using value_type = T;

  Span() = default;
  Span(T* ptr_, SIZE s_) : ptr{ptr_}, s{s_} {}
  template <typename C, std::enable_if_t<is_span_like_v<C>, bool> = 0>
  Span(C& c) : Span{c.data(), static_cast<SIZE>(c.size())} {}

  auto& operator[](SIZE i) { return ptr[i]; }
  auto const& operator[](SIZE i) const { return ptr[i]; }
  auto data() { return ptr; }
  auto data() const { return ptr; }
  auto begin() { return ptr; }
  auto begin() const { return ptr; }
  auto end() { return ptr + s; }
  auto end() const { return ptr + s; }
  SIZE const& size() const { return s; }

 private:
  T* ptr = nullptr;
  SIZE s = 0;
};

template <typename Container, typename SIZE = size_t>
auto to_span(Container& c) {
  static_assert(is_span_like_v<Container>, "Container is not Span-able");
  return Span<typename Container::value_type>{c.data(), c.size()};
}

template <typename T, typename SIZE = size_t>
struct SpanSet {
 public:
  using value_type = T;
  using SpanSet_ = SpanSet<T, SIZE>;

  SpanSet() = default;

  SpanSet(std::vector<SIZE>&& sizes_)
      : m_size{std::accumulate(sizes_.begin(), sizes_.end(), SIZE{0})},
        m_sizes(sizes_),
        m_displs(sizes_.size()),
        m_vec(m_size) {
    for (SIZE off = 0, i = 0; i < static_cast<SIZE>(sizes_.size()); i++, off += sizes_[i])
      m_displs[i] = off;
  }

  SpanSet(SpanSet_&& from)
      : m_size{from.m_size},
        m_sizes{std::move(from.m_sizes)},
        m_displs{std::move(from.m_displs)},
        m_vec{std::move(from.m_vec)} {}

  Span<T, SIZE> operator[](SIZE i) { return {m_vec.data() + m_displs[i], m_sizes[i]}; }
  Span<T, SIZE> operator[](SIZE i) const { return {m_vec.data() + m_displs[i], m_sizes[i]}; }

  T* data() { return m_vec.data(); }
  T* data() const { return m_vec.data(); }

  Span<T, SIZE> raw() { return {m_vec.data(), m_size}; }
  Span<T, SIZE> raw() const { return {m_vec.data(), m_size}; }

  struct iterator {
    iterator(SpanSet_* _sv) : sv(_sv) {}
    iterator operator++() {
      curr_pos += sv->m_sizes[curr_ptr++];
      return *this;
    }
    bool operator!=(const iterator& /*other*/) const { return curr_ptr != sv->m_sizes.size(); }
    Span<T, SIZE> operator*() const { return {sv->m_vec.data() + curr_pos, sv->m_sizes[curr_ptr]}; }

    SpanSet_* sv = nullptr;
    SIZE curr_pos = 0, curr_ptr = 0;
  };

  auto begin() { return iterator(this); }
  auto cbegin() const { return iterator(this); }

  auto end() { return iterator(this); }
  auto cend() const { return iterator(this); }

  SIZE const& size() const { return m_size; }
  std::vector<SIZE> const& sizes() const { return m_sizes; }
  std::vector<SIZE> const& displs() const { return m_displs; }

 private:
  SIZE m_size = 0;
  std::vector<SIZE> m_sizes, m_displs;
  std::vector<T> m_vec;
};


}  // namespace kul
}  // namespace mkn

#endif /* _MKN_KUL_SPAN_HPP_ */
