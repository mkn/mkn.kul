// IWYU pragma: private, include "mkn/kul/alloc.hpp"

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
#ifndef _MKN_KUL_ALLOC_HUGE_HPP_
#define _MKN_KUL_ALLOC_HUGE_HPP_

#if __has_include(<sys/mman.h>)

#include "mkn/kul/alloc/base.hpp"

#include <new>
#include <cstddef>
#include <cstdlib>
#include <type_traits>

#include <limits>
#include <stdlib.h>    // posix_memalign
#include <sys/mman.h>  // madvise

namespace mkn::kul {

template <typename T, std::size_t huge_page_size = 4096>
class HugePageAllocator : public Allocator<T> {
  using This = HugePageAllocator<T, huge_page_size>;

 public:
  template <typename U>
  struct rebind {
    using other = HugePageAllocator<U>;
  };

  HugePageAllocator() = default;
  template <class U>
  constexpr HugePageAllocator(HugePageAllocator<U> const&) noexcept {}

  T* allocate(std::size_t n) {
    if (n > std::numeric_limits<std::size_t>::max() / sizeof(T)) throw std::bad_alloc();
    void* p = nullptr;

    if (posix_memalign(&p, huge_page_size, n * sizeof(T)) != 0) throw std::bad_alloc{};

#ifdef MADV_HUGEPAGE
    madvise(p, n * sizeof(T), MADV_HUGEPAGE);
#endif  // MADV_HUGEPAGE
    if (p == nullptr) throw std::bad_alloc();
    return static_cast<T*>(p);
  }
  void deallocate(T* p, std::size_t n) { std::free(p); }
  bool operator!=(This const& that) const { return !(*this == that); }
  bool operator==(This const& /*that*/) const {
    return true;  // stateless
  }
};

template <typename T, std::size_t huge_page_size = 4096>
class NonConstructingHugePageAllocator : public NonConstructingAllocator<T> {
  using This = NonConstructingHugePageAllocator<T, huge_page_size>;

 public:
  template <typename U>
  struct rebind {
    using other = NonConstructingHugePageAllocator<U>;
  };

  NonConstructingHugePageAllocator() = default;
  template <class U>
  constexpr NonConstructingHugePageAllocator(NonConstructingHugePageAllocator<U> const&) noexcept {}

  T* allocate(std::size_t n) {
    if (n > std::numeric_limits<std::size_t>::max() / sizeof(T)) throw std::bad_alloc();
    void* p = nullptr;
    if (posix_memalign(&p, huge_page_size, n * sizeof(T)) != 0) throw std::bad_alloc{};

#ifdef MADV_HUGEPAGE
    madvise(p, n * sizeof(T), MADV_HUGEPAGE);
#endif  // MADV_HUGEPAGE
    if (p == nullptr) throw std::bad_alloc();
    return static_cast<T*>(p);
  }

  void deallocate(T* p, std::size_t n) { std::free(p); }

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

#endif  // __has_include(<sys/mman.h>)

#endif /*_MKN_KUL_ALLOC_HUGE_HPP_*/
