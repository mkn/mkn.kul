/**
Copyright (c) 2023, Philip Deegan.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

    * Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
copyright notice, this list of conditions and the following disclaimer
in the documentation and/or other materials provided with the
distribution.
    * Neither the name of Philip Deegan nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/*
REQUIRES
    dep: google.sparsehash

*/
/**     BREAKDOWN
 *
 *       2 To
 *       S STRING
 *       T TEMPLATE
 **/

#ifndef _MKN_KUL_MAP_HPP_
#define _MKN_KUL_MAP_HPP_

#include <string>

#ifndef _MKN_WITH_GOOGLE_SPARSEHASH_

#include <unordered_map>
#include <unordered_set>

namespace mkn {
namespace kul {
namespace hash {

template <class K, class V>
class Map {
 public:
  Map& emplace(const K& k, const V& v) {
    _map.emplace(k, v);
    return *this;
  }
  Map& insert(const K& k, const V& v) {
    _map.insert(std::make_pair(k, v));
    return *this;
  }
  Map& insert(std::pair<K, V> const& pair) {
    _map.insert(pair);
    return *this;
  }
  void setDeletedKey(K const&) { /* ununused, but here for interoperability with sparsehash */
  }

  auto& at(K const& k) { return _map.at(k); }
  auto& at(K const& k) const { return _map.at(k); }

  auto& operator[](K const& k) { return _map[k]; }
  auto& operator[](K const& k) const { return _map.at(k); }

  auto count(K const& k) const { return _map.count(k); }
  auto erase(K const& k) { return _map.erase(k); }
  auto size() const { return _map.size(); }

  auto begin() { return _map.begin(); }
  auto begin() const { return _map.begin(); }
  auto end() { return _map.end(); }
  auto end() const { return _map.end(); }

  auto find(K const& key) { return _map.find(key); }
  auto find(K const& key) const { return _map.find(key); }

  void clear() { _map.clear(); }
  // auto empty() { return size() == 0; }

 private:
  std::unordered_map<K, V> _map;
};

namespace map {
template <class T>
using S2T = Map<std::string, T>;
using S2S = S2T<std::string>;
}  // namespace map

template <class T>
using Set = std::unordered_set<T>;

namespace set {
using String = Set<std::string>;
}  // namespace set
}  // namespace hash
}  // namespace kul
}  // namespace mkn

#else

#include "sparsehash/sparse_hash_map"
#include "sparsehash/sparse_hash_set"

#include "sparsehash/dense_hash_map"
#include "sparsehash/dense_hash_set"

namespace mkn {
namespace kul {

struct StdStringComparator {
 public:
  bool operator()(std::string const& s1, std::string const& s2) const {
    return (s1.compare(s2) == 0);
  }
};

namespace hash {
using namespace google;

template <class T, class HashFcn, class EqualKey, class Alloc = libc_allocator_with_realloc<T> >
class Set : public google::sparse_hash_set<T, HashFcn, EqualKey> {
 public:
  typedef typename google::sparse_hash_set<T, HashFcn, EqualKey, Alloc> Hash;
  typedef typename Hash::size_type size_type;
  typedef typename Hash::key_type key_type;
  typedef typename Hash::iterator iterator;
  typedef typename Hash::const_iterator const_iterator;

  void setEmptyKey(key_type const& key) { Hash::set_empty_key(key); }
  void setDeletedKey(key_type const& key) { Hash::set_deleted_key(key); }
  void clear() { Hash::clear(); }
  std::pair<iterator, bool> insert(const T obj) { return Hash::insert(obj); }
  T& operator[](key_type const& key) { return Hash::operator[](key); }
  size_type erase(key_type const& key) { return Hash::erase(key); }
  iterator begin() { return Hash::begin(); }
  iterator end() { return Hash::end(); }
  const_iterator begin() const { return Hash::begin(); }
  const_iterator end() const { return Hash::end(); }
  size_type size() const { return Hash::size(); }
  const key_type getDeletedKey() const { return Hash::deleted_key(); }
  size_type count(key_type const& key) const { return Hash::count(key); }
};

namespace set {
using String = Set<std::string, std::hash<std::string>, StdStringComparator>;
}  // namespace set

template <class K, class V, class HashFcn, class EqualKey,
          class Alloc = libc_allocator_with_realloc<std::pair<K, V> > >
class Map : public google::sparse_hash_map<K, V, HashFcn, EqualKey> {
 public:
  typedef typename google::sparse_hash_map<K, V, HashFcn, EqualKey> map;
  typedef typename map::size_type size_type;
  typedef typename map::key_type key_type;
  typedef typename map::iterator iterator;
  typedef typename map::const_iterator const_iterator;

  void setDeletedKey(key_type const& key) { map::set_deleted_key(key); }
  void setEmptyKey(key_type const& key) { map::set_empty_key(key); }
  void clear() { map::clear(); }
  std::pair<iterator, bool> insert(std::pair<K, V> const& obj) { return map::insert(obj); }
  std::pair<iterator, bool> insert(const K k, V v) { return insert(std::pair<K, V>(k, v)); }
  V& operator[](key_type const& key) { return map::operator[](key); }
  size_type erase(key_type const& key) { return map::erase(key); }
  iterator begin() { return map::begin(); }
  iterator end() { return map::end(); }
  iterator find(key_type const& key) { return map::find(key); }
  size_type size() const { return map::size(); }
  const_iterator begin() const { return map::begin(); }
  const_iterator end() const { return map::end(); }
  const_iterator find(key_type const& key) const { return map::find(key); }
  size_type count(key_type const& key) const { return map::count(key); }
  const key_type getDeletedKey() const { return map::deleted_key(); }
};

namespace map {
template <class T>
using S2T = Map<std::string, T, std::hash<std::string>, StdStringComparator,
                libc_allocator_with_realloc<std::pair<const std::string, T> > >;
using S2S = S2T<std::string>;
}  // namespace map
}  // namespace hash

namespace dense {
namespace hash {
using namespace google;

template <class T, class HashFcn, class EqualKey, class Alloc = libc_allocator_with_realloc<T> >
class Set : public google::dense_hash_set<T, HashFcn, EqualKey> {
 public:
  typedef typename google::dense_hash_set<T, HashFcn, EqualKey, Alloc> Hash;
  typedef typename Hash::size_type size_type;
  typedef typename Hash::key_type key_type;
  typedef typename Hash::iterator iterator;
  typedef typename Hash::const_iterator const_iterator;

  void setEmptyKey(key_type const& key) { Hash::set_empty_key(key); }
  void clear() { Hash::clear(); }
  void setDeletedKey(key_type const& key) { Hash::set_deleted_key(key); }
  std::pair<iterator, bool> insert(const T obj) { return Hash::insert(obj); }
  T& operator[](key_type const& key) { return Hash::operator[](key); }
  size_type erase(key_type const& key) { return Hash::erase(key); }
  iterator begin() { return Hash::begin(); }
  iterator end() { return Hash::end(); }
  const_iterator begin() const { return Hash::begin(); }
  const_iterator end() const { return Hash::end(); }
  size_type size() const { return Hash::size(); }
  const key_type getDeletedKey() const { return Hash::deleted_key(); }
  size_type count(key_type const& key) const { return Hash::count(key); }
};

namespace set {
using String = Set<std::string, std::hash<std::string>, StdStringComparator>;
}

template <class K, class V, class HashFcn, class EqualKey,
          class Alloc = libc_allocator_with_realloc<std::pair<K, V> > >
class Map : public google::dense_hash_map<K, V, HashFcn, EqualKey> {
 public:
  typedef typename google::dense_hash_map<K, V, HashFcn, EqualKey> map;
  typedef typename map::size_type size_type;
  typedef typename map::key_type key_type;
  typedef typename map::iterator iterator;
  typedef typename map::const_iterator const_iterator;

  void setDeletedKey(key_type const& key) { map::set_deleted_key(key); }
  void setEmptyKey(key_type const& key) { map::set_empty_key(key); }
  void clear() { map::clear(); }
  std::pair<iterator, bool> insert(std::pair<K, V> const& obj) { return map::insert(obj); }
  std::pair<iterator, bool> insert(const K k, V v) { return insert(std::pair<K, V>(k, v)); }
  V& operator[](key_type const& key) { return map::operator[](key); }
  size_type erase(key_type const& key) { return map::erase(key); }
  iterator begin() { return map::begin(); }
  iterator end() { return map::end(); }
  iterator find(key_type const& key) { return map::find(key); }
  size_type size() const { return map::size(); }
  const_iterator begin() const { return map::begin(); }
  const_iterator end() const { return map::end(); }
  const_iterator find(key_type const& key) const { return map::find(key); }
  const key_type getDeletedKey() const { return map::deleted_key(); }
  size_type count(key_type const& key) const { return map::count(key); }
};

namespace map {
template <class T>
using S2T = Map<std::string, T, std::hash<std::string>, StdStringComparator,
                libc_allocator_with_realloc<std::pair<const std::string, T> > >;
using S2S = S2T<std::string>;

}  // namespace map
}  // namespace hash
}  // namespace dense
}  // namespace kul
}  // namespace mkn

#endif  //_MKN_WITH_GOOGLE_SPARSEHASH_

#endif /* _MKN_KUL_MAP_HPP_ */
