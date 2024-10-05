

#include "test_common.hpp"

#include "mkn/kul/dbg.hpp"
#include "mkn/kul/alloc.hpp"
#include "mkn/kul/vector.hpp"

#include <array>
#include <cstddef>
#include <stdexcept>

template <std::size_t size = 8>
struct S {
  S() {
    for (std::size_t i = 0; i < size; i++) vars[i] = i;
  }

  bool operator==(S const& that) const {
    for (std::size_t i = 0; i < size; i++)
      if (vars[i] != that.vars[i]) return false;
    return true;
  }

  bool operator!=(S const& that) const { return !(*this == that); }

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
  return V{size};
}
template <typename V0, typename V1>
auto make_vector_from(V1 const& v1) {
  KUL_DBG_FUNC_ENTER;
  V0 v(v1.size());
  std::copy(v1.begin(), v1.end(), v.begin());
  return v;
}

template <typename T>
void do_compare() {
  constexpr static std::size_t N = 2e6;
  auto const std_vec = make_vector<std::vector<T>>(N);
  auto const std_vec2 = make_vector_from<std::vector<T>>(std_vec);
  auto const no_construct_vec = make_vector_from<mkn::kul::NonConstructingVector<T>>(std_vec);
  if (std_vec != no_construct_vec) throw std::runtime_error("FAIL");
  if (std_vec != std_vec2) throw std::runtime_error("FAIL");
  auto const v0 = copy_construct(std_vec);
  auto const v1 = copy_construct(no_construct_vec);
  auto const v2 = copy_manual(std_vec);
  auto const v3 = copy_manual(no_construct_vec);

  if (v0 != std_vec) throw std::runtime_error("FAIL 0");
  if (v0 == v1) throw std::runtime_error("FAIL 1");  // :(
  if (v0 != v2) throw std::runtime_error("FAIL 2");
  if (v0 != v3) throw std::runtime_error("FAIL 3");
}

TEST(NoConstructAllocator, copies) { do_compare<S<8>>(); }
