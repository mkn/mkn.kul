
#ifndef KUL_MATH_BLAS_HPP_
#define KUL_MATH_BLAS_HPP_

#include "mkn/kul/std/ints.hpp"

namespace mkn {
namespace kul {
namespace math {
template <typename T, typename K, typename Y = T>
static inline void mult_incr(uint64_t const n, K const alpha, Y const* x, T* y)
  requires(std::is_same_v<T, float>)
{
  cblas_saxpy(n, alpha, x, 1, y, 1);
}
template <typename T, typename K, typename Y = T>
static inline void mult_incr(uint64_t const n, K const alpha, Y const* x, T* y)
  requires(std::is_same_v<T, double>)
{
  cblas_daxpy(n, alpha, x, 1, y, 1);
}

template <typename T, typename K>
static inline K dot(Size_t const n, T const* x, K const* y)
  requires(std::is_same_v<T, float> && !std::is_same_v<T, std::atomic<K>> &&
           !std::is_same_v<K, std::atomic<T>>)
{
  return cblas_sdot(n, x, 1, y, 1);
}
template <typename T, typename K>
static inline K dot(Size_t const n, T const* x, K const* y)
  requires(std::is_same_v<double, T> && !std::is_same_v<T, std::atomic<K>> &&
           !std::is_same_v<K, std::atomic<T>>)
{
  return cblas_ddot(n, x, 1, y, 1);
}

template <typename T, typename K, typename Y = T>
static inline void scale(Size_t const n, std::atomic<K> const& alpha, T* x)
  requires(std::is_same_v<T, float>)
{
  cblas_sscal(n, alpha.load(), x, 1);
}
template <typename T, typename K, typename Y = T>
static inline void scale(Size_t const n, K const alpha, T* x)
  requires(std::is_same_v<T, float>)
{
  cblas_sscal(n, alpha, x, 1);
}

template <typename T, typename K, typename Y = T>
static inline void scale(Size_t const n, std::atomic<K> const& alpha, T* x)
  requires(std::is_same_v<T, double>)
{
  cblas_dscal(n, alpha.load(), x, 1);
}
template <typename T, typename K, typename Y = T>
static inline void scale(Size_t const n, K const alpha, T* x)
  requires(std::is_same_v<T, double>)
{
  cblas_dscal(n, alpha, x, 1);
}
}  // namespace math
}  // namespace kul
}  // namespace mkn
#endif  // KUL_MATH_BLAS_HPP_
