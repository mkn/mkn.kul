# `mkn/kul/vector.hpp` — Vectors with Custom Allocators

**Namespace:** `mkn::kul`

See also: [`alloc.hpp`](alloc.md) for the underlying allocator types.

## Type aliases

```cpp
template <typename T>
using Vector = std::vector<T, Allocator<T>>;

template <typename T>
using NonConstructingVector = std::vector<T, NonConstructingAllocator<T>>;

template <typename T, std::size_t S = 4096>
using NonConstructingHugePageVector = std::vector<T, NonConstructingHugePageAllocator<T, S>>;

template <typename T, std::size_t S = 4096>
using HugePageVector = std::vector<T, HugePageAllocator<T, S>>;
```

## Free functions

```cpp
// Zero-copy reinterpret of a NonConstructingVector as a plain Vector
template <typename T>
std::vector<T, Allocator<T>>& as_super(
    std::vector<T, NonConstructingAllocator<T>>& v);

template <typename T>
std::vector<T, Allocator<T>> const& as_super(
    std::vector<T, NonConstructingAllocator<T>> const& v);

// Cross-allocator equality (enabled when A1 derives from Allocator<T>)
template <typename T, typename A0, typename A1>
bool operator==(std::vector<T, A0> const& v0, std::vector<T, A1> const& v1);

template <typename T, typename A0, typename A1>
bool operator!=(std::vector<T, A0> const& v0, std::vector<T, A1> const& v1);

template <typename T, typename A0>
bool operator==(mkn::kul::Vector<T> const& v0, std::vector<T, A0> const& v1);

template <typename T, typename A0>
bool operator!=(mkn::kul::Vector<T> const& v0, std::vector<T, A0> const& v1);
```
