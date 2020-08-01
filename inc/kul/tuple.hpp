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
#ifndef _KUL_TUPLE_HPP_
#define _KUL_TUPLE_HPP_

#include <vector>
#include <numeric>
#include "kul/for.hpp"

namespace kul {

template <size_t N, typename T>
struct Pointer {
  static constexpr size_t INDEX = N;
  T* p = nullptr;
};

template <typename T, typename SIZE = size_t>
class Span {
 public:
  using value_type = T;

  Span() = default;
  Span(T* ptr_, SIZE s_) : ptr{ptr_}, s{s_} {}

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
    bool operator!=(const iterator& other) const { return curr_ptr != sv->m_sizes.size(); }
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

template <typename Tuple>
struct PointersApply {
  PointersApply(Tuple& t) : tuple{t} {}

  template <size_t i>
  decltype(auto) operator()() {
    auto t = std::get<i>(tuple);
    using T = decltype(t);
    if constexpr (std::is_pointer<T>::value)
      return Pointer<i, std::remove_pointer_t<std::decay_t<T>>>{t};
    else
      return Pointer<i, std::decay_t<T>>{&t};
  }

  Tuple& tuple;
};

template <typename... Pointer>
struct PointerContainer : public Pointer... {
  PointerContainer(Pointer&... args) : Pointer(std::forward<Pointer>(args))... {}
};

template <typename... Args>
decltype(auto) _make_pointer_container(std::tuple<Args...>&& t) {
  return std::make_from_tuple<PointerContainer<Args...>>(t);
}

template <typename... Refs>
decltype(auto) make_pointer_container(Refs&&... args) {
  auto tuple = std::forward_as_tuple(args...);
  constexpr size_t size = std::tuple_size<decltype(tuple)>::value;
  return _make_pointer_container(for_N<size>(PointersApply{tuple}));
}

template <typename T>
struct ApplySingleTupleValue {
  constexpr ApplySingleTupleValue(T t_) : t{t_} {}
  template <size_t i>
  constexpr decltype(auto) operator()() {
    return t;
  }
  T t;
};

template <typename T, size_t Size>
constexpr decltype(auto) tuple_from(T t) {
  return for_N<Size>(ApplySingleTupleValue{t});
}

}  // namespace kul

#endif /* _KUL_TUPLE_HPP_ */
