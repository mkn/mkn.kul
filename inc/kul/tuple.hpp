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

#include "kul/for.hpp"

namespace kul {

template <size_t N, typename T>
struct Pointer {
  static constexpr size_t INDEX = N;
  T* p = nullptr;
};

template <typename T, typename SIZE = size_t>
struct Pointers {
  Pointers(T const* p_, SIZE s_) : p{p_}, s{s_} {}
  T const* p = nullptr;
  SIZE s = 0;
  auto& operator[](SIZE i) const { return p[i]; }
  auto& data() const { return p; }
  auto& begin() const { return p; }
  auto end() const { return p + s; }
  auto& size() const { return s; }
};



template <typename T, typename SIZE = size_t>
struct SplitVector {
  using value_type = T;
  using SplitVector_ = SplitVector<T, SIZE>;

  SplitVector() {}
  SplitVector(SIZE size) : vec(size) {}
  SplitVector(SplitVector&& from) : vec{std::move(from.vec)}, sizes{std::move(from.sizes)} {};

  struct iterator {
    iterator(SplitVector_* sv) : sv(sv) {}
    iterator operator++() {
      curr_pos += sv->sizes[curr_ptr++];
      return *this;
    }
    bool operator!=(const iterator& other) const { return curr_ptr != sv->sizes.size(); }
    Pointers<T, SIZE> operator*() const {
      return Pointers<T, SIZE>{sv->vec.data() + curr_pos, sv->sizes[curr_ptr]};
    }

    SplitVector_* sv = nullptr;
    SIZE curr_pos = 0, curr_ptr = 0;
  };

  auto operator[](SIZE i) const {
    size_t pos = std::accumulate(sizes.begin() + 1, sizes.begin() + i + 1, 0);
    return Pointers<T, SIZE>{this->vec.data() + pos, this->sizes[i]};
  }

  T* data() { return const_cast<T*>(&vec[0]); }

  auto begin() { return iterator(this); }
  auto cbegin() const { return iterator(this); }

  auto end() { return iterator(this); }
  auto cend() const { return iterator(this); }

  std::vector<T> vec;
  std::vector<SIZE> sizes;
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
