
#ifndef KUL_MATH_NOBLAS_HPP_
#define KUL_MATH_NOBLAS_HPP_

namespace mkn {
namespace kul {
namespace math {
template <typename T, typename K, typename Y = T>
static inline
    typename std::enable_if<std::is_same<T, float>::value || std::is_same<T, double>::value>::type
    mult_incr(uint64_t const n, K const alpha, Y const* x, T* y) {
  detail::mult_incr(n, alpha, x, y);
}

template <typename T, typename K>
static inline
    typename std::enable_if<(std::is_same<T, float>::value || std::is_same<T, double>::value) &&
                                !std::is_same<T, std::atomic<K>>::value &&
                                !std::is_same<K, std::atomic<T>>::value,
                            K>::type
    dot(size_t const n, T const* x, K const* y) {
  return detail::dot(n, x, y);
}

template <typename T, typename K, typename Y = T>
static inline
    typename std::enable_if<std::is_same<T, float>::value || std::is_same<T, double>::value>::type
    scale(size_t const n, std::atomic<K> const& alpha, T* x) {
  detail::scale(n, alpha.load(), x);
}

template <typename T, typename K, typename Y = T>
static inline
    typename std::enable_if<std::is_same<T, float>::value || std::is_same<T, double>::value>::type
    scale(size_t const n, K const alpha, T* x) {
  detail::scale(n, alpha, x);
}

}  // namespace math
}  // namespace kul
}  // namespace mkn
#endif  // KUL_MATH_NOBLAS_HPP_
