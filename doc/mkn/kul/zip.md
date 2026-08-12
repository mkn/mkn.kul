# `mkn/kul/zip.hpp` — Zip Iterator

**Namespace:** `mkn::kul`

## struct `Zipit<Args...>`

Iterator that advances all underlying iterators together and dereferences to a `std::tuple`.

```cpp
template <typename... Args>
struct Zipit {
  auto  operator* ();                          // returns tuple of dereferenced values
  bool  operator==(Zipit const& that) const;
  bool  operator!=(Zipit const& that) const;
  auto& operator++();                          // advances all iterators
};
```

## struct `Zipper<Args...>`

Range adaptor that provides `begin`/`end` over multiple containers simultaneously.

```cpp
template <typename... Args>
struct Zipper {
  using Iter = Zipit<Args...>;

  auto begin();
  auto end  ();
};
```

## Free function

```cpp
template <typename... Args>
auto zip(Args&&... args);   // returns Zipper<Args...>
```

**Usage:**
```cpp
for (auto [a, b, c] : mkn::kul::zip(vecA, vecB, vecC)) {
  // a, b, c are references to the current elements
}
```
