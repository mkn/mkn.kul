# `mkn/kul/for.hpp` — Compile-Time Iteration

**Namespace:** `mkn::kul`

## enum `for_N_R_mode`

Controls what `for_N` returns.

```cpp
enum class for_N_R_mode {
  make_tuple    = 0,   // std::tuple of return values
  make_array,          // std::array of return values
  forward_tuple,       // forward the results as a tuple
};
```

## struct `Apply<T>`

Default callable for `for_N`: returns `std::integral_constant<T, i>` for each index `i`.

```cpp
template <typename T = std::size_t>
struct Apply {
  template <std::size_t i>
  constexpr auto operator()();
};
```

## Boolean helpers

```cpp
// true if every argument is exactly type T
template <typename T, typename... Ts>
auto constexpr all_are(Ts&&... ts);

// variadic fold over bool values
auto constexpr any(auto... bools) requires(all_are<bool>(bools...));
auto constexpr all(auto... bools) requires(all_are<bool>(bools...));

// container overloads with optional predicate (default: convert to bool)
template <typename Container, typename Fn = decltype(to_bool)>
auto constexpr all (Container const& c, Fn fn = to_bool);

template <typename Container, typename Fn = decltype(to_bool)>
auto constexpr any (Container const& c, Fn fn = to_bool);

template <typename Container, typename Fn = decltype(to_bool)>
auto constexpr none(Container const& c, Fn fn = to_bool);
```

## Compile-time loop

```cpp
// Invoke fn with integral_constant<size_t, 0..N-1>; collect results per M
template <std::size_t N, auto M = for_N_R_mode::make_tuple, typename Fn>
constexpr auto for_N(Fn& fn);

template <std::size_t N, auto M = for_N_R_mode::make_tuple, typename Fn>
constexpr auto for_N(Fn&& fn);

// Convenience wrappers
template <std::size_t N, typename Fn>
constexpr auto for_N_make_array(Fn&& fn);

template <std::size_t N, typename Fn>
constexpr auto for_N_all(Fn&& fn);   // true if all N invocations return true

template <std::size_t N, typename Fn>
constexpr auto for_N_any(Fn&& fn);   // true if any invocation returns true
```

## `generate_from` — transform to new container

```cpp
template <typename F>
auto generate_from(F&& f, std::size_t from, std::size_t to);

template <typename F>
auto generate_from(F&& f, std::size_t count);

template <typename F, typename Container>
auto generate_from(F&& f, Container& container);

template <typename F, typename Container>
auto generate_from(F&& f, Container const& container);

template <typename F, typename T>
auto generate_from(F&& f, std::vector<T>&& v);

template <typename F, typename Type, std::size_t Size>
auto constexpr generate_from(F&& f, std::array<Type, Size> const& arr);
```
