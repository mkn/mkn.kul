
#ifndef KUL_MATH_BLAS_HPP_
#define KUL_MATH_BLAS_HPP_

namespace kul {
namespace math {
namespace detail {

template <typename T>
struct cblas : noop<T> {};

template <typename T>
struct cblas_base : noop<T> {

  template <typename K = T> static inline
  K sum(const ulong n, const T *x) { return noop<T>::template sum<K>(n, x); }

  template <typename K = T> static inline
  void set(const ulong n, const K alpha, T *x) { noop<T>::set(n, alpha, x); }

  template <typename K = T> static inline
  typename std::enable_if<std::is_same<T, std::atomic<K>>::value>::type
  scale(const ulong n, const K alpha, T *x) { noop<T>::scale(n, alpha, x); }

  template <typename K = T> static inline
  typename std::enable_if<std::is_same<T, std::atomic<K>>::value, T>::type
  dot(const ulong n, const T *x, const K *y) { return noop<T>::dot(n, x, y); }

  template <typename K = T> static inline
  typename std::enable_if<std::is_same<T, std::atomic<K>>::value, K>::type
  dot(const ulong n,  const K *x, const T *y) {return noop<T>::dot(n, x, y); }

    // template <typename K = T> static inline
    // KUL_TRACE_OR_VOID mult_incr(const uint64_t n, const float alpha, const T *x, T *y) {
    //   noop<T>::template mult_incr<K>(n, alpha, x, y);
    //   KUL_STACK_TRACE;
    // }

    // template <typename K = T> static inline
    // KUL_TRACE_OR_VOID mult_incr(const uint64_t n, const float alpha, const std::atomic<T> *x, T *y) {
    //   noop<T>::template mult_incr<K>(n, alpha, x, y);
    //   KUL_STACK_TRACE;
    // }

};

template <>
struct cblas<float> final : public cblas_base<float> {

  template <typename K = float>
  static inline K
  absolute_sum(const ulong n, const float *x) { return cblas_sasum(n, x, 1); }

  template <typename K = float>
  static inline K dot(const ulong n, const float *x, const float *y) {
    return cblas_sdot(n, x, 1, y, 1);
  }

  template <typename K = float>
  static inline K dot(const ulong n, const float *x, const std::atomic<float> *y) {
    return noop<float>::dot(n, x, y);
  }
  static inline
  void scale(const ulong n, const std::atomic<float> alpha, float *x) {
    cblas_sscal(n, alpha.load(), x, 1);
  }
  static inline
  void scale(const ulong n, float alpha, float *x) { cblas_sscal(n, alpha, x, 1); }

  static inline
  KUL_TRACE_OR_VOID mult_incr(const uint64_t n, const float alpha, const float *x, float *y) {
    cblas_saxpy(n, alpha, x, 1, y, 1);
    KUL_STACK_TRACE;
  }

#if defined(_KUL_USE_CATLAS)
  void set(const ulong n, const float alpha, float *x) { catlas_sset(n, alpha, x, 1); }
#endif
};

template <>
struct cblas<double> final : public cblas_base<double> {
  template <typename K = double>
  static inline K absolute_sum(const ulong n, const double *x) { return cblas_dasum(n, x, 1); }
  template <typename K = double>
  static inline K  dot(const ulong n, const double *x, const double *y) {
    return cblas_ddot(n, x, 1, y, 1);
  }
  template <typename K = double>
  static inline K dot(const ulong n, const double *x, const std::atomic<double> *y) {
    return noop<double>::dot(n, x, y);
  }
  static inline
  KUL_TRACE_OR_VOID mult_incr(const uint64_t n, const double alpha, const double *x, double *y) {
    cblas_daxpy(n, alpha, x, 1, y, 1);
    KUL_STACK_TRACE;
  }
  static inline
  KUL_TRACE_OR_VOID mult_incr(const uint64_t n, const double alpha, const std::atomic<double> *x,
                 double *y) {
    noop<double>::mult_incr(n, alpha, x, y);
    KUL_STACK_TRACE;
  }
  static inline
  void scale(const ulong n, const std::atomic<double> alpha, double *x) {
    cblas_dscal(n, alpha.load(), x, 1);
  }
  static inline
  void scale(const ulong n, double alpha, double *x) { cblas_dscal(n, alpha, x, 1); }
#if defined(_KUL_USE_CATLAS)
  static inline
  void set(const ulong n, const double alpha, double *x) { catlas_sset(n, alpha, x, 1); }
#endif
};

template <>
struct cblas<uint32_t> final : public cblas_base<uint32_t> {
  using T = uint32_t;

  template <typename K = T> static inline
  K sum(const ulong n, const T *x) { return noop<T>::template sum<K>(n, x); }

  template <typename K = T> static inline
  void set(const ulong n, const K alpha, T *x) { noop<T>::set(n, alpha, x); }

  template <typename K = T> static inline
  typename std::enable_if<std::is_same<T, std::atomic<K>>::value>::type
  scale(const ulong n, const K alpha, T *x) { noop<T>::scale(n, alpha, x); }

  template <typename K = T> static inline
  typename std::enable_if<!std::is_same<T, std::atomic<K>>::value>::type
  scale(const ulong n, const K alpha, T *x) { noop<T>::scale(n, alpha, x); }

  template <typename K = T> static inline
  typename std::enable_if<std::is_same<T, std::atomic<K>>::value, T>::type
  dot(const ulong n, const T *x, const K *y) { return noop<T>::dot(n, x, y); }

  template <typename K = T> static inline
  typename std::enable_if<std::is_same<T, std::atomic<K>>::value, K>::type
  dot(const ulong n,  const K *x, const T *y) {return noop<T>::dot(n, x, y); }

  template <typename K = T> static inline
  typename std::enable_if<!std::is_same<T, std::atomic<K>>::value, T>::type
  dot(const ulong n, const T *x, const K *y) {return noop<T>::dot(n, x, y); }

  template <typename K = T> static inline
  KUL_TRACE_OR_VOID mult_incr(const uint64_t n, const T alpha, const T *x, T *y) {
    noop<T>::template mult_incr<K>(n, alpha, x, y);
    KUL_STACK_TRACE;
  }
};

template <>
struct cblas<int32_t> final : public cblas_base<int32_t> {
  using T = int32_t;

  template <typename K = T> static inline
  K sum(const ulong n, const T *x) { return noop<T>::template sum<K>(n, x); }

  template <typename K = T> static inline
  void set(const ulong n, const K alpha, T *x) { noop<T>::set(n, alpha, x); }

  template <typename K = T> static inline
  typename std::enable_if<std::is_same<T, std::atomic<K>>::value>::type
  scale(const ulong n, const K alpha, T *x) { noop<T>::scale(n, alpha, x); }

  template <typename K = T> static inline
  typename std::enable_if<!std::is_same<T, std::atomic<K>>::value>::type
  scale(const ulong n, const K alpha, T *x) { noop<T>::scale(n, alpha, x); }

  template <typename K = T> static inline
  typename std::enable_if<std::is_same<T, std::atomic<K>>::value, T>::type
  dot(const ulong n, const T *x, const K *y) { return noop<T>::dot(n, x, y); }

  template <typename K = T> static inline
  typename std::enable_if<std::is_same<T, std::atomic<K>>::value, K>::type
  dot(const ulong n,  const K *x, const T *y) {return noop<T>::dot(n, x, y); }

  template <typename K = T> static inline
  typename std::enable_if<!std::is_same<T, std::atomic<K>>::value, T>::type
  dot(const ulong n, const T *x, const K *y) {return noop<T>::dot(n, x, y); }

  template <typename K = T> static inline
  KUL_TRACE_OR_VOID mult_incr(const uint64_t n, const T alpha, const T *x, T *y) {
    noop<T>::template mult_incr<K>(n, alpha, x, y);
    KUL_STACK_TRACE;
  }
};

// template <> struct cblas<int32_t>;
// template <> struct cblas<uint32_t>;

}  // namespace detail
}  // namespace math
}  // namespace kul

#endif  // KUL_MATH_BLAS_HPP_
