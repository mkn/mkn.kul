# `mkn/kul/tuple.hpp` — Tuple Utilities

**Namespace:** `mkn::kul`

## struct `Pointer<N, T>`

Tagged pointer carrying a compile-time index.

```cpp
template <size_t N, typename T>
struct Pointer {
  static constexpr size_t INDEX = N;
  T* p = nullptr;
};
```

## struct `PointersApply<Tuple>`

Extracts `Pointer<i, element_type>` entries from a tuple of references.

```cpp
template <typename Tuple>
struct PointersApply {
  PointersApply(Tuple& t);

  template <size_t i>
  auto operator()();   // returns Pointer<i, std::tuple_element_t<i, Tuple>>
};
```

## struct `PointerContainer<Pointers...>`

Inherits from each `Pointer` type; provides multi-pointer access by index.

```cpp
template <typename... Pointers>
struct PointerContainer : public Pointers... {
  PointerContainer(Pointers&... args);
};
```

## struct `ApplySingleTupleValue<T>`

Callable that returns the same value for every compile-time index.

```cpp
template <typename T>
struct ApplySingleTupleValue {
  constexpr ApplySingleTupleValue(T t_);

  template <size_t i>
  constexpr auto operator()();
};
```

## Free functions

```cpp
// Build a PointerContainer from a pack of lvalue references
template <typename... Refs>
auto make_pointer_container(Refs&&... args);

// Create a std::tuple of Size copies of value t
template <typename T, size_t Size>
constexpr auto tuple_from(T t);
```
