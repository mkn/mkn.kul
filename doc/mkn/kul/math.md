# `mkn/kul/math.hpp` — Math Utilities

**Namespace:** `mkn::kul::math`

## class `Exception`

```cpp
class Exception : public mkn::kul::Exception {
public:
  Exception(char const* f, uint16_t const& l, std::string const& s);
};
```

## Free functions

```cpp
template <class T>
T abs(T const& f);

// f raised to the power e (default: square)
template <class T = float>
T pow(float const& f, int16_t const& e = 2);

// Newton-Raphson root of order r (default: square root, 6 iterations)
template <class T = float>
T root(float const& f, int16_t const& r = 2, uint16_t const& it = 6, T g = 0);

// Product of all elements; mul is the initial accumulator
template <typename Container, typename Multiplies = typename Container::value_type>
Multiplies product(Container const& container, Multiplies mul = 1);

// Sum of all elements; r is the initial accumulator
template <typename Container, typename Return = typename Container::value_type>
Return sum(Container const& container, Return r = 0);
```
