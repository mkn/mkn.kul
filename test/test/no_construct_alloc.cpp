

#include "test_common.hpp"

#include "mkn/kul/dbg.hpp"
#include "mkn/kul/alloc.hpp"
#include "mkn/kul/vector.hpp"

#include <array>
#include <cstddef>
#include <cstring>
#include <algorithm>
#include <stdexcept>

template <std::size_t size = 8>
struct S {
  S() {
    for (std::size_t i = 0; i < size; i++) vars[i] = i;
  }

  bool operator==(S const& that) const {
    return std::equal(vars.begin(), vars.end(), that.vars.begin());
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
auto copy_mem(V const& v) {
  KUL_DBG_FUNC_ENTER;
  V out;
  out.reserve(v.capacity());
  out.resize(v.size());
  memcpy(out.data(), v.data(), v.size() * sizeof(typename V::value_type));
  return out;
}

template <typename V>
auto copy_loop(V const& v) {
  KUL_DBG_FUNC_ENTER;
  V out;
  out.reserve(v.capacity());
  out.resize(v.size());
  for (std::size_t i = 0; i < v.size(); ++i) out[i] = v[i];
  // memcpy(out.data(), v.data(), v.size() * sizeof(typename V::value_type));
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

template <typename V0>
void resize(V0& v) {
  KUL_DBG_FUNC_ENTER;
  v.resize(v.capacity() + 1);  // force reallocation
}

template <typename T>
void do_compare() {
  constexpr static std::size_t N = 2000000;
  auto const std_vec = make_vector<std::vector<T>>(N);
  auto const v0 = copy_construct(std_vec);

  auto wash = [&]<typename V>(auto const& name) {
    KOUT(NON) << name;
    auto const copy = make_vector_from<V>(v0);
    auto const v1 = copy_mem(copy);
    auto const v2 = copy_loop(copy);
    auto const v3 = copy_manual(copy);

    if (v0 != v1) throw std::runtime_error("FAIL 1");  // :(
    if (v0 != v2) throw std::runtime_error("FAIL 2");
    if (v0 != v3) throw std::runtime_error("FAIL 3");
  };

  using namespace mkn::kul;
  wash.template operator()<std::vector<T>>("std::vector<T>");
  wash.template operator()<NonConstructingVector<T>>("NonConstructingVector<T>");
  wash.template operator()<HugePageVector<T>>("HugePageVector<T>");
  wash.template operator()<NonConstructingHugePageVector<T>>("NonConstructingHugePageVector<T>");
}

TEST(NoConstructAllocator, copies) { do_compare<double>(); }

// int main(int argc, char* argv[]) {
//   ::testing::InitGoogleTest(&argc, argv);
//   return RUN_ALL_TESTS();
// }
