#ifndef MKN_KUL_STD_TYPES_HPP_
#define MKN_KUL_STD_TYPES_HPP_

namespace mkn::kul {

template <typename T>
struct StrongType {
  explicit StrongType(T const& t) : _t{t} {}

  template <typename U>
  StrongType(U const& u) : _t{static_cast<T>(u)} {}

  StrongType(StrongType&&) = default;
  StrongType(StrongType const&) = default;

  operator T&() { return _t; }
  operator T const&() const { return _t; }

  StrongType& operator=(StrongType&&) = default;
  StrongType& operator=(StrongType const&) = default;

  StrongType& operator=(T&& t) {
    _t = t;
    return *this;
  }
  StrongType& operator=(T const& t) {
    _t = t;
    return *this;
  }

  template <typename U>
  StrongType& operator=(U const& u) {
    _t = static_cast<T>(u);
    return *this;
  };

  T _t;
};

}  // namespace mkn::kul

#endif /* MKN_KUL_STD_TYPES_HPP_ */
