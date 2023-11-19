
#ifndef KUL_MATH_NOOP_HPP_
#define KUL_MATH_NOOP_HPP_

// This macro ensures that the corresponding optimized blas function is used if
// available
#if !defined(NDEBUG) && defined(_MKN_KUL_USE_CBLAS)
// x and y are two pointers
#define CHECK_BLAS_OPTIMIZATION_PP(x, y, func_name)                               \
  if (typeid(*x) == typeid(*y) &&                                                 \
      (typeid(*x) == typeid(double) || typeid(*x) == typeid(float))) /* NOLINT */ \
    KEXCEPT(mkn::kul::math::Exception, func_name) << " should use blas optimized version";

// x is a pointer, y is a scalar
#define CHECK_BLAS_OPTIMIZATION_PS(x, y, func_name)                               \
  if (typeid(*x) == typeid(y) &&                                                  \
      (typeid(*x) == typeid(double) || typeid(*x) == typeid(float))) /* NOLINT */ \
    KEXCEPT(mkn::kul::math::Exception, func_name) << " should use blas optimized version";
#else
#define CHECK_BLAS_OPTIMIZATION_PP(x, y, func_name)
#define CHECK_BLAS_OPTIMIZATION_PS(x, y, func_name)
#endif

namespace mkn {
namespace kul {
namespace math {
namespace detail {
template <typename T, typename K, typename Y>
void mult_incr(const uint64_t n, const K alpha, Y const* x, T* y) {
  for (uint64_t i = 0; i < n; ++i) {
    K y_i = y[i];
    y_i += alpha * x[i];
    y[i] = y_i;
  }
}
template <typename T, typename K>
void scale(const size_t n, const K alpha, T* x) {
  for (uint64_t i = 0; i < n; ++i) x[i] *= alpha;
}
template <typename T, typename K = T>
void scale(const size_t n, const T alpha, std::atomic<T>* x) {
  for (uint64_t i = 0; i < n; ++i) {
    K x_i = x[i].load();
    x_i *= alpha;
    x[i].store(x_i);
  }
}
template <typename T, typename K>
T dot(const size_t n, T const* x, K const* y) {
  T result{0};
  for (uint64_t i = 0; i < n; ++i) result += x[i] * y[i];
  return result;
}
template <typename T, typename K = T>
T dot(const size_t n, T const* x, std::atomic<T> const* y) {
  K result{0};
  for (uint64_t i = 0; i < n; ++i) result += x[i] * y[i].load();
  return result;
}
}  // namespace detail
template <typename T, typename K>
typename std::enable_if<(!std::is_same<T, double>::value && !std::is_same<T, float>::value) &&
                            std::is_same<T, std::atomic<K>>::value,
                        T>::type
dot(const size_t n, T const* x, K const* y) {
  return detail::dot(n, y, x);
}
template <typename T, typename K>
typename std::enable_if<(!std::is_same<T, double>::value && !std::is_same<T, float>::value) &&
                            std::is_same<T, std::atomic<K>>::value,
                        K>::type
dot(const size_t n, K const* x, T const* y) {
  return detail::dot(n, x, y);
}

template <typename T, typename K>
typename std::enable_if<(!std::is_same<T, double>::value && !std::is_same<T, float>::value) &&
                            !std::is_same<T, std::atomic<K>>::value,
                        T>::type
dot(const size_t n, T const* x, K const* y) {
  CHECK_BLAS_OPTIMIZATION_PP(x, y, "dot prod");
  return detail::dot(n, x, y);
}

template <typename T, typename K, typename Y>
typename std::enable_if<!(std::is_same<T, double>::value || std::is_same<T, float>::value) &&
                        std::is_same<T, std::atomic<K>>::value &&
                        !std::is_same<Y, std::atomic<K>>::value>::type
mult_incr(const uint64_t n, const K alpha, Y const* x, T* y) {
  for (uint64_t i = 0; i < n; ++i) {
    K y_i = y[i].load();
    y_i += alpha * x[i];
    y[i].store(y_i);
  }
}

template <typename T, typename K, typename Y>
typename std::enable_if<!(std::is_same<T, double>::value || std::is_same<T, float>::value) &&
                        std::is_same<Y, std::atomic<K>>::value &&
                        !std::is_same<T, std::atomic<K>>::value>::type
mult_incr(const uint64_t n, const K alpha, Y const* x, T* y) {
  for (uint64_t i = 0; i < n; ++i) {
    K y_i = y[i];
    y_i += alpha * x[i].load();
    y[i] = y_i;
  }
}
template <typename T, typename K, typename Y>
typename std::enable_if<!(std::is_same<T, double>::value || std::is_same<T, float>::value) &&
                        std::is_same<T, std::atomic<K>>::value &&
                        std::is_same<Y, std::atomic<K>>::value>::type
mult_incr(const uint64_t n, const K alpha, Y const* x, T* y) {
  for (uint64_t i = 0; i < n; ++i) {
    K y_i = y[i].load();
    y_i += alpha * x[i].load();
    y[i].store(y_i);
  }
}
template <typename T, typename K, typename Y>
typename std::enable_if<!(std::is_same<T, double>::value || std::is_same<T, float>::value)>::type
mult_incr(const uint64_t n, const K alpha, Y const* x, T* y) {
  CHECK_BLAS_OPTIMIZATION_PP(x, y, "mult_incr");
  return detail::mult_incr(n, alpha, x, y);
}

template <typename T, typename K>
typename std::enable_if<std::is_same<T, std::atomic<K>>::value>::type set(const size_t n,
                                                                          const K alpha, T* x) {
  for (uint64_t i = 0; i < n; ++i) x[i].store(alpha);
}
template <typename T, typename K>
typename std::enable_if<!std::is_same<T, std::atomic<K>>::value>::type set(const size_t n,
                                                                           const K alpha, T* x) {
  for (uint64_t i = 0; i < n; ++i) x[i] = alpha;
}

template <typename T, typename K>
K sum(const size_t n, T const* x) {
  return std::accumulate(x, x + n, K{0});
}

template <typename T, typename K>
typename std::enable_if<!(std::is_same<T, double>::value || std::is_same<T, float>::value) &&
                        std::is_same<T, std::atomic<K>>::value>::type
scale(const size_t n, const K alpha, T* x) {
  for (uint64_t i = 0; i < n; ++i) {
    K x_i = x[i].load();
    x_i *= alpha;
    x[i].store(x_i);
  }
}
template <typename T, typename K>
typename std::enable_if<!(std::is_same<T, double>::value || std::is_same<T, float>::value) &&
                        !std::is_same<T, std::atomic<K>>::value>::type
scale(const size_t n, const K alpha, T* x) {
  CHECK_BLAS_OPTIMIZATION_PS(x, alpha, "scale");
  detail::scale(n, alpha, x);
}

template <typename T, typename K>
typename std::enable_if<std::is_same<T, std::atomic<K>>::value>::type dot_matrix_vector_incr(
    const size_t m, const size_t n, const K alpha, T const* a, T const* x, const T beta, T* y) {
  for (size_t i = 0; i < m; ++i) {
    K y_i = beta * y[i];
    for (size_t j = 0; j < n; ++j) y_i += alpha * a[i * n + j] * x[j].load();
    y[i].store(y_i);
  }
}

template <typename T, typename K>
typename std::enable_if<!std::is_same<T, std::atomic<K>>::value>::type dot_matrix_vector_incr(
    const size_t m, const size_t n, const K alpha, T const* a, T const* x, const T beta, T* y) {
  for (size_t i = 0; i < m; ++i) {
    y[i] = beta * y[i];
    for (size_t j = 0; j < n; ++j) y[i] += alpha * a[i * n + j] * x[j];
  }
}

template <typename T, typename K>
typename std::enable_if<std::is_same<T, std::atomic<K>>::value>::type dot_matrix_vector(
    const size_t m, const size_t n, const K alpha, T const* a, T const* x, T* y) {
  for (size_t i = 0; i < m; ++i) {
    K y_i = 0;
    for (size_t j = 0; j < n; ++j) y_i += alpha * a[i * n + j] * x[j].load();
    y[i].store(y_i);
  }
}

template <typename T, typename K>
typename std::enable_if<!std::is_same<T, std::atomic<K>>::value>::type dot_matrix_vector(
    const size_t m, const size_t n, const K alpha, T const* a, T const* x, T* y) {
  for (size_t i = 0; i < m; ++i) {
    y[i] = 0;
    for (size_t j = 0; j < n; ++j) y[i] += alpha * a[i * n + j] * x[j];
  }
}

#undef CHECK_BLAS_OPTIMIZATION_PP
#undef CHECK_BLAS_OPTIMIZATION_PS

}  //  end namespace math
}  // namespace kul
}  // namespace mkn

#endif  // KUL_MATH_NOOP_HPP_
