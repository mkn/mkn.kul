
#ifndef KUL_MATH_BLAS_HPP_
#define KUL_MATH_BLAS_HPP_

namespace mkn {
namespace kul {
namespace math {
template <typename T, typename K, typename Y = T>
static inline typename std::enable_if<std::is_same<T, float>::value>::type mult_incr(
    uint64_t const n, K const alpha, Y const* x, T* y) {
  cblas_saxpy(n, alpha, x, 1, y, 1);
}
template <typename T, typename K, typename Y = T>
static inline typename std::enable_if<std::is_same<T, double>::value>::type mult_incr(
    uint64_t const n, K const alpha, Y const* x, T* y) {
  cblas_daxpy(n, alpha, x, 1, y, 1);
}

template <typename T, typename K>
static inline typename std::enable_if<std::is_same<T, float>::value &&
                                          !std::is_same<T, std::atomic<K>>::value &&
                                          !std::is_same<K, std::atomic<T>>::value,
                                      K>::type
dot(size_t const n, T const* x, K const* y) {
  return cblas_sdot(n, x, 1, y, 1);
}
template <typename T, typename K>
static inline typename std::enable_if<std::is_same<double, T>::value &&
                                          !std::is_same<T, std::atomic<K>>::value &&
                                          !std::is_same<K, std::atomic<T>>::value,
                                      K>::type
dot(size_t const n, T const* x, K const* y) {
  return cblas_ddot(n, x, 1, y, 1);
}

template <typename T, typename K, typename Y = T>
static inline typename std::enable_if<std::is_same<T, float>::value>::type scale(
    size_t const n, std::atomic<K> const alpha, T* x) {
  cblas_sscal(n, alpha.load(), x, 1);
}
template <typename T, typename K, typename Y = T>
static inline typename std::enable_if<std::is_same<T, float>::value>::type scale(size_t const n,
                                                                                 K const alpha,
                                                                                 T* x) {
  cblas_sscal(n, alpha, x, 1);
}

template <typename T, typename K, typename Y = T>
static inline typename std::enable_if<std::is_same<T, double>::value>::type scale(
    size_t const n, std::atomic<K> const alpha, T* x) {
  cblas_dscal(n, alpha.load(), x, 1);
}
template <typename T, typename K, typename Y = T>
static inline typename std::enable_if<std::is_same<T, double>::value>::type scale(size_t const n,
                                                                                  K const alpha,
                                                                                  T* x) {
  cblas_dscal(n, alpha, x, 1);
}
}  // namespace math
}  // namespace kul
}  // namespace mkn
#endif  // KUL_MATH_BLAS_HPP_
