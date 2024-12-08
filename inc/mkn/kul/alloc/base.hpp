/**
Copyright (c) 2022, Philip Deegan.
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
#ifndef _MKN_KUL_ALLOC_BASE_HPP_
#define _MKN_KUL_ALLOC_BASE_HPP_

#include <new>
#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <type_traits>

namespace mkn::kul {

template <typename T>
class Allocator {
  using This = Allocator<T>;

 public:
  using pointer = T*;
  using reference = T&;
  using value_type = T;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

  template <typename U>
  struct rebind {
    using other = Allocator<U>;
  };

  T* allocate(std::size_t const n) const { return static_cast<T*>(::operator new(n * sizeof(T))); }

  void deallocate(T* const p) noexcept {
    if (p) ::operator delete(p);
  }
  void deallocate(T* const p, std::size_t /*n*/) noexcept {  // needed from std::
    if (p) ::operator delete(p);
  }
  bool operator!=(This const& that) const { return !(*this == that); }
  bool operator==(This const& /*that*/) const {
    return true;  // stateless
  }
};

template <typename T>
class NonConstructingAllocator : public Allocator<T> {
  using This = NonConstructingAllocator<T>;

 public:
  template <typename U>
  struct rebind {
    using other = NonConstructingAllocator<U>;
  };

  T* allocate(std::size_t const n) const {
    // if (n == 0) return nullptr;
    return static_cast<T*>(malloc(n * sizeof(T)));
  }

  template <typename U, typename... Args>
  void construct(U* ptr, Args&&... args) {
    ::new ((void*)ptr) U(std::forward<Args>(args)...);
  }

  template <typename U>
  void construct(U* /*ptr*/) noexcept(std::is_nothrow_default_constructible<U>::value) {}

  bool operator!=(This const& that) const { return !(*this == that); }
  bool operator==(This const& /*that*/) const {
    return true;  // stateless
  }
};

}  // namespace mkn::kul

#endif /*_MKN_KUL_ALLOC_BASE_HPP_*/
