# `mkn/kul/all.hpp` — Aggregate Comparison

**Namespace:** `mkn::kul`

## Free functions

```cpp
template <typename Container, typename Function>
void for_each(Container& container, Function&& function);

template <typename Container, typename Function>
bool any_of(Container const& container, Function&& function);

template <typename Container, typename Function>
bool any_of(Container&& container, Function&& function);

template <typename Container, typename Function>
bool all_of(Container const& container, Function&& function);

template <typename Container, typename Function>
bool all_of(Container&& container, Function&& function);

// Compare t against every element of container/args/tuple using binary predicate op
template <typename T, typename OP, typename Container>
bool compare_to(T const& t, OP const& op, Container const& container);

template <typename T, typename OP, typename... Args>
bool compare_to(T const& t, OP const& op, Args const&... args);

template <typename T, typename OP, typename... Args>
bool compare_to(T&& t, OP&& op, Args&&... args);

template <typename T, typename OP, typename... TupleElements>
bool compare_to(T const& t, OP const& op, std::tuple<TupleElements...> const& tuple);

template <typename T, typename OP, typename... TupleElements>
bool compare_to(T&& t, OP&& op, std::tuple<TupleElements...>&& tuple);
```

## class `All<Args...>`

Holds a set of values and exposes relational operators that test all values against a single comparand.

```cpp
template <typename... Args>
class All {
public:
  All(Args const&... args);

  template <typename T> bool operator==(T const& t);
  template <typename T> bool operator==(T&&      t);
  template <typename T> bool operator!=(T const& t);
  template <typename T> bool operator!=(T&&      t);
  template <typename T> bool operator> (T const& t);
  template <typename T> bool operator> (T&&      t);
  template <typename T> bool operator< (T const& t);
  template <typename T> bool operator< (T&&      t);
  template <typename T> bool operator>=(T const& t);
  template <typename T> bool operator>=(T&&      t);
  template <typename T> bool operator<=(T const& t);
  template <typename T> bool operator<=(T&&      t);
};
```

**Usage:**
```cpp
if (mkn::kul::All{a, b, c} == 0) { /* all three are zero */ }
```
