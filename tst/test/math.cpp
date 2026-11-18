

#include "test_common.hpp"

#include "mkn/kul/math.hpp"

template <typename T>
void do_math() {
  T beta = 2;
  T a[2] = {1, 2}, b[2] = {3, 4};
  mkn::kul::math::dot_matrix_vector_incr(2, 2, 2, &a[0], &b[0], beta, &b[0]);
  mkn::kul::math::dot(2u, &a[0], &b[0]);
  mkn::kul::math::scale(2u, 2u, &a[0]);
  mkn::kul::math::mult_incr(2u, 2u, &a[0], &b[0]);
}

template <typename T>
void do_math_has_atomics() {
  T a[2] = {1, 2};
  std::atomic<T> b[2];
  b[0] = 3;
  b[1] = 4;
  mkn::kul::math::dot(2u, &a[0], &b[0]);
  mkn::kul::math::scale(2u, 2u, &a[0]);
  mkn::kul::math::mult_incr(2u, 2u, &a[0], &b[0]);
}

TEST(Math, double) { do_math<double>(); }
TEST(Math, atomic_double) { do_math_has_atomics<double>(); }
TEST(Math, float) { do_math<float>(); }
TEST(Math, atomic_float) { do_math_has_atomics<float>(); }

TEST(Math, more) {
  do_math<uint32_t>();
  do_math<int32_t>();
}

#include <array>
#include <vector>

TEST(Math, from) {
  std::vector<int> const v{3, -1, 7, 2};
  EXPECT_EQ(mkn::kul::math::product(std::array{2, 3, 4}), 24);
  EXPECT_EQ(mkn::kul::math::max_from(v), 7);
  EXPECT_EQ(mkn::kul::math::min_from(v), -1);
  EXPECT_EQ(mkn::kul::math::sum_from(v, [](auto e) { return e * 2; }), 22);
  EXPECT_DOUBLE_EQ(mkn::kul::math::norm(std::array{3., 4., 0.}), 5.);
}
