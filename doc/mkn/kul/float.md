# `mkn/kul/float.hpp` — Floating-Point Comparison

**Namespace:** `mkn::kul`

```cpp
auto inline float_equals(float  const a, float  const b, float  const diff = 1e-6);
auto inline float_equals(double const a, double const b, double const diff = 1e-12);
```

Returns `true` when `|a - b| <= diff`.
