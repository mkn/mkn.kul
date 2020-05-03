/**
Copyright (c) 2017, Philip Deegan.
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

template<size_t N, typename T>
struct Pointer {
  static constexpr size_t INDEX = N;
  T* p                          = nullptr;
};

template<typename T>
struct Pointers {
  Pointers(T const * p_, size_t s_) : p{p_}, s{s_}{}
  T const * p = nullptr;
  size_t s    = 0;
  auto& begin() const { return p; }
  auto  end()   const { return p + s; }
  auto& size()  const { return s; }
};

template<typename Tuple>
struct PointersApply {
  PointersApply(Tuple& t) : tuple{t}{}

  template<size_t i>
  decltype(auto) operator()()
  {
      auto t  = std::get<i>(tuple);
      using T = decltype(t);
      if constexpr (std::is_pointer<T>::value)
          return Pointer<i, std::remove_pointer_t<std::decay_t<T>>>{t};
      else
          return Pointer<i, std::decay_t<T>>{&t};
  }

  Tuple& tuple;
};

template <typename ...Pointer>
struct PointerContainer : public Pointer...{
    PointerContainer(Pointer&... args) : Pointer(std::forward<Pointer>(args))...{}
};

template <typename ...Args>
decltype(auto) _make_pointer_container(std::tuple<Args...>&& t){
   return std::make_from_tuple<PointerContainer<Args...>>(t);
}

template <typename ...Refs>
decltype(auto) make_pointer_container(Refs&&... args){
  auto tuple = std::forward_as_tuple(args...);
  constexpr size_t size = std::tuple_size<decltype(tuple)>::value;
  return _make_pointer_container(for_N<size>(PointersApply{tuple}));
}


}  // namespace kul

#endif /* _KUL_TUPLE_HPP_ */
