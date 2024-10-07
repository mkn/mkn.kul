#ifndef _MKN_TEST_INSTRUMENT_ALLOC_HPP_
#define _MKN_TEST_INSTRUMENT_ALLOC_HPP_

#include "mkn/kul/dbg.hpp"
#include "mkn/kul/vector.hpp"

#include <array>
#include <cstddef>
#include <algorithm>

template <std::size_t size = 8>
struct S {
  S() {
    for (std::size_t i = 0; i < size; i++) vars[i] = i;
  }

  bool operator==(S const& that) const {
    return std::equal(vars.begin(), vars.end(), that.vars.begin());
  }

  bool operator!=(S const& that) const { return !(*this == that); }
  auto& operator[](std::uint8_t const i) const { return vars[i]; }
  std::array<std::size_t, size> vars;
};

template <typename V>
auto copy_construct(V const& v) {
  KUL_DBG_FUNC_ENTER;
  V out{v};
  return out;
}

template <typename V>
auto copy_operator_equal(V const& v) {
  KUL_DBG_FUNC_ENTER;
  V out;
  out.reserve(v.capacity());
  out = v;
  return out;
}

template <typename V>
auto copy_operator_equal_super(V const& v) {
  KUL_DBG_FUNC_ENTER;
  V out;
  out.reserve(v.capacity());
  mkn::kul::as_super(out) = mkn::kul::as_super(v);
  return out;
}

template <typename V>
auto copy_manual(V const& v) {
  KUL_DBG_FUNC_ENTER;
  V out;
  out.reserve(v.capacity());
  out.resize(v.size());
  std::copy(v.begin(), v.end(), out.begin());
  return out;
}

template <typename V>
auto make_vector(std::size_t const& size) {
  KUL_DBG_FUNC_ENTER;
  return V(size);
}
template <typename V0, typename V1>
auto make_vector_from(V1 const& v1) {
  KUL_DBG_FUNC_ENTER;
  V0 v(v1.size());
  std::copy(v1.begin(), v1.end(), v.begin());
  return v;
}

#endif /*_MKN_TEST_INSTRUMENT_ALLOC_HPP_*/
