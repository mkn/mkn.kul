# `mkn/kul/map.hpp` — Hash Maps and Sets

**Namespace:** `mkn::kul::hash`

By default wraps `std::unordered_map` / `std::unordered_set`. When `_MKN_WITH_GOOGLE_SPARSEHASH_` is defined, wraps `google::sparse_hash_map` / `google::sparse_hash_set` and an additional `mkn::kul::dense::hash` variant is provided.

## class `Map<K, V>`

```cpp
template <class K, class V>
class Map {
public:
  Map& emplace(K const& k, V const& v);
  Map& insert (K const& k, V const& v);
  Map& insert (std::pair<K, V> const& pair);

  void setDeletedKey(K const&);   // sparsehash only

  auto& at(K const& k);
  auto& at(K const& k) const;
  auto& operator[](K const& k);
  auto& operator[](K const& k) const;

  auto  count (K const& k) const;
  auto  erase (K const& k);
  auto  size  () const;
  auto  empty () const;
  void  clear ();

  auto begin();
  auto begin() const;
  auto end  ();
  auto end  () const;

  auto find(K const& key);
  auto find(K const& key) const;
};
```

## class `Set<T>`

Standard unordered set interface (`insert`, `erase`, `count`, `find`, `size`, `empty`, `begin`, `end`, `clear`).

## Type aliases

```cpp
namespace mkn::kul::hash::map {
  template <class T>
  using S2T = Map<std::string, T>;   // string → T map

  using S2S = S2T<std::string>;      // string → string map
}

namespace mkn::kul::hash::set {
  using String = Set<std::string>;
}
```

## Dense hash variants (sparsehash only)

`mkn::kul::dense::hash::Map<K,V>` and `mkn::kul::dense::hash::Set<T>` mirror the sparse variants above but use `google::dense_hash_map` / `google::dense_hash_set` internally.
