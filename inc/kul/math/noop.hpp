
#ifndef KUL_MATH_NOOP_H_
#define KUL_MATH_NOOP_H_

namespace kul {
namespace math {
namespace detail {

template <typename T>
struct noop {
  template <typename K = T> static inline
  K sum(const ulong n, const T *x);

  template <typename K = T> static inline
  typename std::enable_if<std::is_same<T, std::atomic<K>>::value, T>::type
  dot(const ulong n, const T *x, const K *y);

  template <typename K = T> static inline
  typename std::enable_if<std::is_same<T, std::atomic<K>>::value, K>::type
  dot(const ulong n, const K *x, const T *y);

  template <typename K = T> static inline
  typename std::enable_if<!std::is_same<T, std::atomic<K>>::value, T>::type
  dot(const ulong n, const T *x, const K *y);

  template <typename K = T> static inline
  typename std::enable_if<std::is_same<T, std::atomic<K>>::value>::type
  scale(const ulong n, const K alpha, T *x);

  template <typename K = T> static inline
  typename std::enable_if<!std::is_same<T, std::atomic<K>>::value>::type
  scale(const ulong n, const K alpha, T *x);

  template <typename K = T> static inline
  typename std::enable_if<std::is_same<T, std::atomic<K>>::value>::type
  set(const ulong n, const K alpha, T *x);

  template <typename K = T> static inline
  typename std::enable_if<!std::is_same<T, std::atomic<K>>::value>::type
  set(const ulong n, const K alpha, T *x);

  template <typename K, typename Y> static inline
  typename std::enable_if<std::is_same<T, std::atomic<K>>::value &&
                          !std::is_same<Y, std::atomic<K>>::value>::type
  mult_incr(const uint64_t n, const K alpha, const Y *x, T *y);

  template <typename K, typename Y> static inline
  typename std::enable_if<std::is_same<Y, std::atomic<K>>::value &&
                          !std::is_same<T, std::atomic<K>>::value>::type
  mult_incr(const uint64_t n, const K alpha, const Y *x, T *y);

  template <typename K, typename Y> static inline
  typename std::enable_if<std::is_same<T, std::atomic<K>>::value &&
                          std::is_same<Y, std::atomic<K>>::value>::type
  mult_incr(const uint64_t n, const K alpha, const Y *x, T *y);

  template <typename K, typename Y> static inline
  typename std::enable_if<std::is_same<T, K>::value && std::is_same<Y, K>::value>::type
  mult_incr(const uint64_t n, const K alpha, const Y *x, T *y);

  template <typename K = T> static inline
  typename std::enable_if<std::is_same<T, std::atomic<K>>::value>::type
  dot_matrix_vector_incr(
      const ulong m, const ulong n, const K alpha, const T *a, const T *x, const T beta,
      T *y);

  template <typename K = T> static inline
  typename std::enable_if<!std::is_same<T, std::atomic<K>>::value>::type
  dot_matrix_vector_incr(
    const ulong m, const ulong n, const K alpha, const T *a, const T *x, const T beta,
      T *y);

  template <typename K = T> static inline
  typename std::enable_if<std::is_same<T, std::atomic<K>>::value>::type
  dot_matrix_vector(
    const ulong m, const ulong n, const K alpha, const T *a, const T *x, T *y);

  template <typename K = T> static inline
  typename std::enable_if<!std::is_same<T, std::atomic<K>>::value>::type
  dot_matrix_vector(
      const ulong m, const ulong n, const K alpha, const T *a, const T *x, T *y);
};

}  // namespace detail
}  // namespace math
}  // namespace kul

#endif  // KUL_MATH_NOOP_H_
