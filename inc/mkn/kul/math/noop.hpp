
#ifndef KUL_MATH_NOOP_HPP_
#define KUL_MATH_NOOP_HPP_

// This macro ensures that the corresponding optimized blas function is used if
// available
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <numeric>

#include "mkn/kul/std/ints.hpp"

#if !defined(NDEBUG) && defined(MKN_KUL_USE_CBLAS)
// x and y are two pointers
#define CHECK_BLAS_OPTIMIZATION_PP(x, y, func_name)                               \
  if (typeid(*x) == typeid(*y) &&                                                 \
      (typeid(*x) == typeid(double) || typeid(*x) == typeid(float))) /* NOLINT */ \
    KEXCEPT(mkn::kul::math::Exception, func_name, " should use blas optimized version");

// x is a pointer, y is a scalar
#define CHECK_BLAS_OPTIMIZATION_PS(x, y, func_name)                               \
  if (typeid(*x) == typeid(y) &&                                                  \
      (typeid(*x) == typeid(double) || typeid(*x) == typeid(float))) /* NOLINT */ \
    KEXCEPT(mkn::kul::math::Exception, func_name, " should use blas optimized version");
#else
#define CHECK_BLAS_OPTIMIZATION_PP(x, y, func_name)
#define CHECK_BLAS_OPTIMIZATION_PS(x, y, func_name)
#endif

namespace mkn {
namespace kul {
namespace math {
namespace detail {
template <typename T, typename K, typename Y>
void mult_incr(uint64_t const n, K const alpha, Y const* x, T* y) {
  for (uint64_t i = 0; i < n; ++i) {
    K y_i = y[i];
    y_i += alpha * x[i];
    y[i] = y_i;
  }
}
template <typename T, typename K>
void scale(Size_t const n, K const alpha, T* x) {
  for (uint64_t i = 0; i < n; ++i) x[i] *= alpha;
}
template <typename T, typename K = T>
void scale(Size_t const n, T const alpha, std::atomic<T>* x) {
  for (uint64_t i = 0; i < n; ++i) {
    K x_i = x[i].load();
    x_i *= alpha;
    x[i].store(x_i);
  }
}
template <typename T, typename K>
T dot(Size_t const n, T const* x, K const* y) {
  T result{0};
  for (uint64_t i = 0; i < n; ++i) result += x[i] * y[i];
  return result;
}
template <typename T, typename K = T>
T dot(Size_t const n, T const* x, std::atomic<T> const* y) {
  K result{0};
  for (uint64_t i = 0; i < n; ++i) result += x[i] * y[i].load();
  return result;
}
}  // namespace detail

template <typename T, typename K>
T dot(Size_t const n, T const* x, K const* y)
  requires((!std::is_same_v<T, double> && !std::is_same_v<T, float>) &&
           std::is_same_v<T, std::atomic<K>>)
{
  return detail::dot(n, y, x);
}

template <typename T, typename K>
K dot(Size_t const n, K const* x, T const* y)
  requires((!std::is_same_v<T, double> && !std::is_same_v<T, float>) &&
           std::is_same_v<T, std::atomic<K>>)
{
  return detail::dot(n, x, y);
}

template <typename T, typename K>
T dot(Size_t const n, T const* x, K const* y)
  requires((!std::is_same_v<T, double> && !std::is_same_v<T, float>) &&
           !std::is_same_v<T, std::atomic<K>>)
{
  CHECK_BLAS_OPTIMIZATION_PP(x, y, "dot prod");
  return detail::dot(n, x, y);
}

template <typename T, typename K, typename Y>
void mult_incr(uint64_t const n, K const alpha, Y const* x, T* y)
  requires(!(std::is_same_v<T, double> || std::is_same_v<T, float>) &&
           std::is_same_v<T, std::atomic<K>> && !std::is_same_v<Y, std::atomic<K>>)
{
  for (uint64_t i = 0; i < n; ++i) {
    K y_i = y[i].load();
    y_i += alpha * x[i];
    y[i].store(y_i);
  }
}

template <typename T, typename K, typename Y>
void mult_incr(uint64_t const n, K const alpha, Y const* x, T* y)
  requires(!(std::is_same_v<T, double> || std::is_same_v<T, float>) &&
           std::is_same_v<Y, std::atomic<K>> && !std::is_same_v<T, std::atomic<K>>)
{
  for (uint64_t i = 0; i < n; ++i) {
    K y_i = y[i];
    y_i += alpha * x[i].load();
    y[i] = y_i;
  }
}

template <typename T, typename K, typename Y>
void mult_incr(uint64_t const n, K const alpha, Y const* x, T* y)
  requires(!(std::is_same_v<T, double> || std::is_same_v<T, float>) &&
           std::is_same_v<T, std::atomic<K>> && std::is_same_v<Y, std::atomic<K>>)
{
  for (uint64_t i = 0; i < n; ++i) {
    K y_i = y[i].load();
    y_i += alpha * x[i].load();
    y[i].store(y_i);
  }
}

template <typename T, typename K, typename Y>
void mult_incr(uint64_t const n, K const alpha, Y const* x, T* y)
  requires(!(std::is_same_v<T, double> || std::is_same_v<T, float>))
{
  CHECK_BLAS_OPTIMIZATION_PP(x, y, "mult_incr");
  return detail::mult_incr(n, alpha, x, y);
}

template <typename T, typename K>
void set(Size_t const n, K const alpha, T* x)
  requires(std::is_same_v<T, std::atomic<K>>)
{
  for (uint64_t i = 0; i < n; ++i) x[i].store(alpha);
}

template <typename T, typename K>
void set(Size_t const n, K const alpha, T* x)
  requires(!std::is_same_v<T, std::atomic<K>>)
{
  for (uint64_t i = 0; i < n; ++i) x[i] = alpha;
}

template <typename T, typename K>
K sum(Size_t const n, T const* x) {
  return std::accumulate(x, x + n, K{0});
}

template <typename T, typename K>
void scale(Size_t const n, K const alpha, T* x)
  requires(!(std::is_same_v<T, double> || std::is_same_v<T, float>) &&
           std::is_same_v<T, std::atomic<K>>)
{
  for (uint64_t i = 0; i < n; ++i) {
    K x_i = x[i].load();
    x_i *= alpha;
    x[i].store(x_i);
  }
}

template <typename T, typename K>
void scale(Size_t const n, K const alpha, T* x)
  requires(!(std::is_same_v<T, double> || std::is_same_v<T, float>) &&
           !std::is_same_v<T, std::atomic<K>>)
{
  CHECK_BLAS_OPTIMIZATION_PS(x, alpha, "scale");
  detail::scale(n, alpha, x);
}

template <typename T, typename K>
void dot_matrix_vector_incr(Size_t const m, Size_t const n, K const alpha, T const* a,
                             T const* x, T const beta, T* y)
  requires(std::is_same_v<T, std::atomic<K>>)
{
  for (size_t i = 0; i < m; ++i) {
    K y_i = beta * y[i];
    for (size_t j = 0; j < n; ++j) y_i += alpha * a[i * n + j] * x[j].load();
    y[i].store(y_i);
  }
}

template <typename T, typename K>
void dot_matrix_vector_incr(Size_t const m, Size_t const n, K const alpha, T const* a,
                             T const* x, T const beta, T* y)
  requires(!std::is_same_v<T, std::atomic<K>>)
{
  for (size_t i = 0; i < m; ++i) {
    y[i] = beta * y[i];
    for (size_t j = 0; j < n; ++j) y[i] += alpha * a[i * n + j] * x[j];
  }
}

template <typename T, typename K>
void dot_matrix_vector(Size_t const m, Size_t const n, K const alpha, T const* a, T const* x,
                        T* y)
  requires(std::is_same_v<T, std::atomic<K>>)
{
  for (size_t i = 0; i < m; ++i) {
    K y_i = 0;
    for (size_t j = 0; j < n; ++j) y_i += alpha * a[i * n + j] * x[j].load();
    y[i].store(y_i);
  }
}

template <typename T, typename K>
void dot_matrix_vector(Size_t const m, Size_t const n, K const alpha, T const* a, T const* x,
                        T* y)
  requires(!std::is_same_v<T, std::atomic<K>>)
{
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
