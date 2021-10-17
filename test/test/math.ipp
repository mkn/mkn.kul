

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
