
#ifndef KUL_MATH_BLAS_HPP_
#define KUL_MATH_BLAS_HPP_

namespace kul {
namespace math {
template <typename T, typename K, typename Y = T>
static inline typename std::enable_if<std::is_same<T, float>::value>::type mult_incr(
    const uint64_t n, const K alpha, const Y *x, T *y) {
  cblas_saxpy(n, alpha, x, 1, y, 1);
}
template <typename T, typename K, typename Y = T>
static inline typename std::enable_if<std::is_same<T, double>::value>::type mult_incr(
    const uint64_t n, const K alpha, const Y *x, T *y) {
  cblas_daxpy(n, alpha, x, 1, y, 1);
}

template <typename T, typename K>
static inline typename std::enable_if<std::is_same<T, float>::value &&
                                          !std::is_same<T, std::atomic<K>>::value &&
                                          !std::is_same<K, std::atomic<T>>::value,
                                      K>::type
dot(const size_t n, const T *x, const K *y) {
  return cblas_sdot(n, x, 1, y, 1);
}
template <typename T, typename K>
static inline typename std::enable_if<std::is_same<double, T>::value &&
                                          !std::is_same<T, std::atomic<K>>::value &&
                                          !std::is_same<K, std::atomic<T>>::value,
                                      K>::type
dot(const size_t n, const T *x, const K *y) {
  return cblas_ddot(n, x, 1, y, 1);
}

template <typename T, typename K, typename Y = T>
static inline typename std::enable_if<std::is_same<T, float>::value>::type scale(
    const size_t n, const std::atomic<K> alpha, T *x) {
  cblas_sscal(n, alpha.load(), x, 1);
}
template <typename T, typename K, typename Y = T>
static inline typename std::enable_if<std::is_same<T, float>::value>::type scale(const size_t n,
                                                                                 const K alpha,
                                                                                 T *x) {
  cblas_sscal(n, alpha, x, 1);
}

template <typename T, typename K, typename Y = T>
static inline typename std::enable_if<std::is_same<T, double>::value>::type scale(
    const size_t n, const std::atomic<K> alpha, T *x) {
  cblas_dscal(n, alpha.load(), x, 1);
}
template <typename T, typename K, typename Y = T>
static inline typename std::enable_if<std::is_same<T, double>::value>::type scale(const size_t n,
                                                                                  const K alpha,
                                                                                  T *x) {
  cblas_dscal(n, alpha, x, 1);
}
}  // namespace math
}  // namespace kul
#endif  // KUL_MATH_BLAS_HPP_
