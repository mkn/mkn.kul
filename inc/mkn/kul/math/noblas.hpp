
#ifndef KUL_MATH_NOBLAS_HPP_
#define KUL_MATH_NOBLAS_HPP_

#include <type_traits>

#include "mkn/kul/std/ints.hpp"

namespace mkn {
namespace kul {
namespace math {
template <typename T, typename K, typename Y = T>
static inline void mult_incr(uint64_t const n, K const alpha, Y const* x, T* y)
  requires(std::is_same_v<T, float> || std::is_same_v<T, double>)
{
  detail::mult_incr(n, alpha, x, y);
}

template <typename T, typename K>
static inline K dot(Size_t const n, T const* x, K const* y)
  requires((std::is_same_v<T, float> || std::is_same_v<T, double>) &&
           !std::is_same_v<T, std::atomic<K>> && !std::is_same_v<K, std::atomic<T>>)
{
  return detail::dot(n, x, y);
}

template <typename T, typename K, typename Y = T>
static inline void scale(Size_t const n, std::atomic<K> const& alpha, T* x)
  requires(std::is_same_v<T, float> || std::is_same_v<T, double>)
{
  detail::scale(n, alpha.load(), x);
}

template <typename T, typename K, typename Y = T>
static inline void scale(Size_t const n, K const alpha, T* x)
  requires(std::is_same_v<T, float> || std::is_same_v<T, double>)
{
  detail::scale(n, alpha, x);
}

}  // namespace math
}  // namespace kul
}  // namespace mkn
#endif  // KUL_MATH_NOBLAS_HPP_
