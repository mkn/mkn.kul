# `mkn/kul/span.hpp` — Span Containers

**Namespace:** `mkn::kul`

## Type trait `is_span_like`

```cpp
template <typename T, typename data_fn = void, typename size_fn = void>
struct is_span_like : std::false_type {};

template <typename T>
auto constexpr is_span_like_v = is_span_like<T>::value;
```

Specialize `is_span_like<T>` to make a custom type constructible from / assignable to `Span`.

## class `Span<T, SIZE>`

Non-owning view over a contiguous range.

```cpp
template <typename T, typename SIZE = size_t>
class Span {
public:
  using value_type = T;

  Span();
  Span(T* ptr_, SIZE s_);

  // Construct from any span-like container
  template <typename C, std::enable_if_t<is_span_like_v<C>, bool> = 0>
  Span(C& c);

  auto&       operator[](SIZE i);
  auto const& operator[](SIZE i) const;

  auto  data();
  auto  data()  const;
  auto  begin();
  auto  begin() const;
  auto  end();
  auto  end()   const;

  SIZE const& size() const;

  template <typename C, std::enable_if_t<is_span_like_v<C>, bool> = 0>
  void  reset(C& c);

  template <typename C, std::enable_if_t<is_span_like_v<C>, bool> = 0>
  auto& operator=(C& c);
};
```

## class `SpanSet<T, SIZE>`

A set of non-owning sub-spans backed by a single buffer, with displacement tracking (useful for MPI-style layouts).

```cpp
template <typename T, typename SIZE = size_t>
struct SpanSet {
  using value_type = T;

  SpanSet();
  SpanSet(std::vector<SIZE>&& sizes_);
  SpanSet(SpanSet&&);

  Span<T, SIZE> operator[](SIZE i);
  Span<T, SIZE> operator[](SIZE i) const;

  T*            data();
  T*            data() const;

  Span<T, SIZE> raw();          // view over the entire backing buffer
  Span<T, SIZE> raw() const;

  auto begin();
  auto cbegin() const;
  auto end();
  auto cend() const;

  SIZE const&              size()   const;
  std::vector<SIZE> const& sizes()  const;
  std::vector<SIZE> const& displs() const;  // displacements into backing buffer
};
```

## Free function

```cpp
template <typename Container, typename SIZE = size_t>
auto to_span(Container& c);   // returns Span<value_type, SIZE>
```
