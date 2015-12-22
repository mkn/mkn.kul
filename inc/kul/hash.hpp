/**
Copyright (c) 2013, Philip Deegan.
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
/**     BREAKDOWN
*       
*       2 To
*       S STRING
*       T TEMPLATE 
*       V VECTOR 
**/

#ifndef _KUL_HASH_HPP_
#define _KUL_HASH_HPP_

#include "sparsehash/sparse_hash_map"
#include "sparsehash/sparse_hash_set"

#include "sparsehash/dense_hash_map"
#include "sparsehash/dense_hash_set"

#include <string>
#include <vector>

namespace kul{ namespace hash{

struct StdStringComparator{
    public:
        bool operator()(const std::string& s1, const std::string& s2) const{
            return (s1.compare(s2) == 0);
        }
};
using namespace google;

template <class T, class HashFcn, class EqualKey, class Alloc = libc_allocator_with_realloc<T> >
class Set : google::sparse_hash_set<T, HashFcn, EqualKey>{
    public:
        typedef typename google::sparse_hash_set<T, HashFcn, EqualKey, Alloc> Hash;
        typedef typename Hash::size_type size_type;
        typedef typename Hash::key_type key_type;
        typedef typename Hash::iterator iterator;
        typedef typename Hash::const_iterator const_iterator;

        std::pair<iterator, bool>   insert(const T obj)                                     { return Hash::insert(obj); }
        T&                          operator[](const key_type& key)                         { return Hash::operator[](key); }
        size_type                   count(const key_type& key)                      const   { return Hash::count(key); }        
        size_type                   erase(const key_type& key)                              { return Hash::erase(key); }
        iterator                    begin()                                                 { return Hash::begin(); }
        iterator                    end()                                                   { return Hash::end(); }
        const_iterator              begin()                                         const   { return Hash::begin(); }
        const_iterator              end()                                           const   { return Hash::end(); }
        size_type                   size()                                          const   { return Hash::size(); }
        void                        setDeletedKey(const key_type& key)                      { Hash::set_deleted_key(key); }
        void                        clear()                                                 { Hash::clear(); }
};

namespace set{
typedef Set<std::string, std::hash<std::string>, StdStringComparator> String;
}

template <class K, class V, class HashFcn, class EqualKey, class Alloc = libc_allocator_with_realloc<std::pair<K, V> > >
class Map : google::sparse_hash_map<K, V, HashFcn, EqualKey>{
    public:
        typedef typename google::sparse_hash_map<K, V, HashFcn, EqualKey> map;
        typedef typename map::size_type size_type;
        typedef typename map::key_type key_type;
        typedef typename map::iterator iterator;
        typedef typename map::const_iterator const_iterator;
        std::pair<iterator, bool>   insert(const std::pair<K, V>& obj)          { return map::insert(obj); }
        std::pair<iterator, bool>   insert(const K k, V v)                      { return insert(std::pair<K, V>(k, v)); }
        V&                          operator[](const key_type& key)             { return map::operator[](key); }
        size_type                   count(const key_type& key)          const   { return map::count(key); }
        size_type                   erase(const key_type& key)                  { return map::erase(key); }
        iterator                    begin()                                     { return map::begin(); }
        iterator                    end()                                       { return map::end(); }
        iterator                    find(const key_type& key)                   { return map::find(key); }
        const_iterator              begin()                             const   { return map::begin(); }
        const_iterator              end()                               const   { return map::end(); }
        const_iterator              find(const key_type& key)           const   { return map::find(key); }
        void                        setDeletedKey(const key_type& key)          { map::set_deleted_key(key); }
        void                        clear()                                     { map::clear(); }
};

namespace map{
template <class T, class HashFcn = std::hash<std::string>, class EqualKey = StdStringComparator, class Alloc = libc_allocator_with_realloc<std::pair<const std::string, T> > >
class S2T : google::sparse_hash_map<std::string, T, HashFcn, EqualKey>{
    public:
        typedef typename google::sparse_hash_map<std::string, T, HashFcn, EqualKey, Alloc> s2T;
        typedef typename s2T::size_type size_type;
        typedef typename s2T::key_type key_type;
        typedef typename s2T::iterator iterator;
        typedef typename s2T::const_iterator const_iterator;

        std::pair<iterator, bool>   insert(const std::pair<std::string, T>& obj)            { return s2T::insert(obj); }
        std::pair<iterator, bool>   insert(const std::string& s, const T& t)                { return insert(std::pair<std::string, T>(s, t)); }
        T&                          operator[](const key_type& key)                         { return s2T::operator[](key); }
        T&                          get(const key_type& key)                                { return (*find(key)).second; }
        size_type                   count(const key_type& key)                      const   { return s2T::count(key); }
        size_type                   erase(const key_type& key)                              { return s2T::erase(key); }
        iterator                    begin()                                                 { return s2T::begin(); }
        iterator                    end()                                                   { return s2T::end(); }
        iterator                    find(const key_type& key)                               { return s2T::find(key); }
        const_iterator              begin()                                         const   { return s2T::begin(); }
        const_iterator              end()                                           const   { return s2T::end(); }
        const_iterator              find(const key_type& key)                       const   { return s2T::find(key); }
        size_type                   size()                                          const   { return s2T::size(); }
        void                        setDeletedKey(const key_type& key)                      { s2T::set_deleted_key(key); }
        void                        clear()                                                 { s2T::clear(); }
};

typedef S2T<std::string, std::hash<std::string>, StdStringComparator> S2S;

template <class T, class HashFcn = std::hash<std::string>, class EqualKey = StdStringComparator, class Alloc = libc_allocator_with_realloc<std::pair<const std::string, std::vector<T> > > >
class S2VT : google::sparse_hash_map<std::string, std::vector<T>, HashFcn, EqualKey>{
    public:
        typedef typename google::sparse_hash_map<std::string, std::vector<T>, HashFcn, EqualKey, Alloc> s2VT;
        typedef typename s2VT::size_type size_type;
        typedef typename s2VT::key_type key_type;
        typedef typename s2VT::iterator iterator;
        typedef typename s2VT::const_iterator const_iterator;
        std::pair<iterator, bool>   insert(const std::pair<std::string, std::vector<T> >& obj)          { return s2VT::insert(obj); }
        std::pair<iterator, bool>   insert(const std::string& s, const std::vector<T>& t)               { return insert(std::pair<std::string, std::vector<T> >(s, t)); }
        size_type                   count(const key_type& key)                                  const   { return s2VT::count(key); }
        size_type                   erase(const key_type& key)                                          { return s2VT::erase(key); }
        iterator                    begin()                                                             { return s2VT::begin(); }
        iterator                    end()                                                               { return s2VT::end(); }
        iterator                    find(const key_type& key)                                           { return s2VT::find(key); }
        const_iterator              begin()                                                     const   { return s2VT::begin(); }
        const_iterator              end()                                                       const   { return s2VT::end(); }
        const_iterator              find(const key_type& key)                                   const   { return s2VT::find(key); }
        size_type                   size()                                                      const   { return s2VT::size(); }
        void                        clear()                                                             { s2VT::clear(); }
};

template <class T, class HashFcn = std::hash<std::string>, class EqualKey = StdStringComparator, class Alloc = libc_allocator_with_realloc<std::pair<const std::string, S2T<T> > > >
class S2S2T : google::sparse_hash_map<std::string, S2T<T>, HashFcn, EqualKey>{
    public:
        typedef typename google::sparse_hash_map<std::string, S2T<T>, HashFcn, EqualKey> s2T2T;
        typedef typename s2T2T::size_type size_type;
        typedef typename s2T2T::key_type key_type;
        typedef typename s2T2T::iterator iterator;
        typedef typename s2T2T::const_iterator const_iterator;

        std::pair<iterator, bool>   insert(const std::pair<std::string, S2T<T> >& obj)          { return s2T2T::insert(obj); }
        std::pair<iterator, bool>   insert(const std::string s, S2T<T> t)                       { return insert(std::pair<std::string, S2T<T> >(s, t)); }
        S2T<T>&                     operator[](const key_type& key)                             { return s2T2T::operator[](key); }
        S2T<T>&                     get(const key_type& key)                                    { return (*s2T2T::find(key)).second; }
        size_type                   count(const key_type& key)                          const   { return s2T2T::count(key); }
        size_type                   erase(const key_type& key)                                  { return s2T2T::erase(key); }
        iterator                    begin()                                                     { return s2T2T::begin(); }
        iterator                    end()                                                       { return s2T2T::end(); }
        iterator                    find(const key_type& key)                                   { return s2T2T::find(key); }
        const_iterator              begin()                                             const   { return s2T2T::begin(); }
        const_iterator              end()                                               const   { return s2T2T::end(); }
        const_iterator              find(const key_type& key)                           const   { return s2T2T::find(key); }
        void                        clear()                                                     { s2T2T::clear(); }
};
}
}



namespace dense{
namespace hash{

struct StdStringComparator{
    public:
        bool operator()(const std::string& s1, const std::string& s2) const{
            return (s1.compare(s2) == 0);
        }
};
using namespace google;

template <class T, class HashFcn, class EqualKey, class Alloc = libc_allocator_with_realloc<T> >
class Set : google::dense_hash_set<T, HashFcn, EqualKey>{
    public:
        typedef typename google::dense_hash_set<T, HashFcn, EqualKey, Alloc> Hash;
        typedef typename Hash::size_type size_type;
        typedef typename Hash::key_type key_type;
        typedef typename Hash::iterator iterator;
        typedef typename Hash::const_iterator const_iterator;

        std::pair<iterator, bool>   insert(const T obj)                                     { return Hash::insert(obj); }
        T&                          operator[](const key_type& key)                         { return Hash::operator[](key); }
        size_type                   count(const key_type& key)                      const   { return Hash::count(key); }        
        size_type                   erase(const key_type& key)                              { return Hash::erase(key); }
        iterator                    begin()                                                 { return Hash::begin(); }
        iterator                    end()                                                   { return Hash::end(); }
        const_iterator              begin()                                         const   { return Hash::begin(); }
        const_iterator              end()                                           const   { return Hash::end(); }
        size_type                   size()                                          const   { return Hash::size(); }
        void                        setDeletedKey(const key_type& key)                      { Hash::set_deleted_key(key); }
        void                        setEmptyKey(const key_type& key)                        { Hash::set_empty_key(key); }
        void                        clear()                                                 { Hash::clear(); }
};

namespace set{
typedef Set<std::string, std::hash<std::string>, StdStringComparator> String;
}

template <class K, class V, class HashFcn, class EqualKey, class Alloc = libc_allocator_with_realloc<std::pair<K, V> > >
class Map : google::dense_hash_map<K, V, HashFcn, EqualKey>{
    public:
        typedef typename google::dense_hash_map<K, V, HashFcn, EqualKey> map;
        typedef typename map::size_type size_type;
        typedef typename map::key_type key_type;
        typedef typename map::iterator iterator;
        typedef typename map::const_iterator const_iterator;
        std::pair<iterator, bool>   insert(const std::pair<K, V>& obj)          { return map::insert(obj); }
        std::pair<iterator, bool>   insert(const K k, V v)                      { return insert(std::pair<K, V>(k, v)); }
        V&                          operator[](const key_type& key)             { return map::operator[](key); }
        size_type                   count(const key_type& key)          const   { return map::count(key); }
        size_type                   erase(const key_type& key)                  { return map::erase(key); }
        iterator                    begin()                                     { return map::begin(); }
        iterator                    end()                                       { return map::end(); }
        iterator                    find(const key_type& key)                   { return map::find(key); }
        const_iterator              begin()                             const   { return map::begin(); }
        const_iterator              end()                               const   { return map::end(); }
        const_iterator              find(const key_type& key)           const   { return map::find(key); }
        void                        setDeletedKey(const key_type& key)          { map::set_deleted_key(key); }
        void                        setEmptyKey(const key_type& key)            { map::set_empty_key(key); }
        void                        clear()                                     { map::clear(); }
};

namespace map{
template <class T, class HashFcn = std::hash<std::string>, class EqualKey = StdStringComparator, class Alloc = libc_allocator_with_realloc<std::pair<const std::string, T> > >
class S2T : google::dense_hash_map<std::string, T, HashFcn, EqualKey>{
    public:
        typedef typename google::dense_hash_map<std::string, T, HashFcn, EqualKey, Alloc> s2T;
        typedef typename s2T::size_type size_type;
        typedef typename s2T::key_type key_type;
        typedef typename s2T::iterator iterator;
        typedef typename s2T::const_iterator const_iterator;

        std::pair<iterator, bool>   insert(const std::pair<std::string, T>& obj)            { return s2T::insert(obj); }
        std::pair<iterator, bool>   insert(const std::string& s, const T& t)                { return insert(std::pair<std::string, T>(s, t)); }
        T&                          operator[](const key_type& key)                         { return s2T::operator[](key); }
        T&                          get(const key_type& key)                                { return (*find(key)).second; }
        size_type                   count(const key_type& key)                      const   { return s2T::count(key); }
        size_type                   erase(const key_type& key)                              { return s2T::erase(key); }
        iterator                    begin()                                                 { return s2T::begin(); }
        iterator                    end()                                                   { return s2T::end(); }
        iterator                    find(const key_type& key)                               { return s2T::find(key); }
        const_iterator              begin()                                         const   { return s2T::begin(); }
        const_iterator              end()                                           const   { return s2T::end(); }
        const_iterator              find(const key_type& key)                       const   { return s2T::find(key); }
        size_type                   size()                                          const   { return s2T::size(); }
        void                        setDeletedKey(const key_type& key)                      { s2T::set_deleted_key(key); }
        void                        setEmptyKey(const key_type& key)                        { s2T::set_empty_key(key); }
        void                        clear()                                                 { s2T::clear(); }
};

typedef S2T<std::string, std::hash<std::string>, StdStringComparator> S2S;

template <class T, class HashFcn = std::hash<std::string>, class EqualKey = StdStringComparator, class Alloc = libc_allocator_with_realloc<std::pair<const std::string, std::vector<T> > > >
class S2VT : google::dense_hash_map<std::string, std::vector<T>, HashFcn, EqualKey>{
    public:
        typedef typename google::dense_hash_map<std::string, std::vector<T>, HashFcn, EqualKey, Alloc> s2VT;
        typedef typename s2VT::size_type size_type;
        typedef typename s2VT::key_type key_type;
        typedef typename s2VT::iterator iterator;
        typedef typename s2VT::const_iterator const_iterator;
        std::pair<iterator, bool>   insert(const std::pair<std::string, std::vector<T> >& obj)          { return s2VT::insert(obj); }
        std::pair<iterator, bool>   insert(const std::string& s, const std::vector<T>& t)               { return insert(std::pair<std::string, std::vector<T> >(s, t)); }
        size_type                   count(const key_type& key)                                  const   { return s2VT::count(key); }
        size_type                   erase(const key_type& key)                                          { return s2VT::erase(key); }
        iterator                    begin()                                                             { return s2VT::begin(); }
        iterator                    end()                                                               { return s2VT::end(); }
        iterator                    find(const key_type& key)                                           { return s2VT::find(key); }
        const_iterator              begin()                                                     const   { return s2VT::begin(); }
        const_iterator              end()                                                       const   { return s2VT::end(); }
        const_iterator              find(const key_type& key)                                   const   { return s2VT::find(key); }
        size_type                   size()                                                      const   { return s2VT::size(); }
        void                        clear()                                                             { s2VT::clear(); }
};

template <class T, class HashFcn = std::hash<std::string>, class EqualKey = StdStringComparator, class Alloc = libc_allocator_with_realloc<std::pair<const std::string, S2T<T> > > >
class S2S2T : google::dense_hash_map<std::string, S2T<T>, HashFcn, EqualKey>{
    public:
        typedef typename google::dense_hash_map<std::string, S2T<T>, HashFcn, EqualKey> s2T2T;
        typedef typename s2T2T::size_type size_type;
        typedef typename s2T2T::key_type key_type;
        typedef typename s2T2T::iterator iterator;
        typedef typename s2T2T::const_iterator const_iterator;

        std::pair<iterator, bool>   insert(const std::pair<std::string, S2T<T> >& obj)          { return s2T2T::insert(obj); }
        std::pair<iterator, bool>   insert(const std::string s, S2T<T> t)                       { return insert(std::pair<std::string, S2T<T> >(s, t)); }
        S2T<T>&                     operator[](const key_type& key)                             { return s2T2T::operator[](key); }
        S2T<T>&                     get(const key_type& key)                                    { return (*s2T2T::find(key)).second; }
        size_type                   count(const key_type& key)                          const   { return s2T2T::count(key); }
        size_type                   erase(const key_type& key)                                  { return s2T2T::erase(key); }
        iterator                    begin()                                                     { return s2T2T::begin(); }
        iterator                    end()                                                       { return s2T2T::end(); }
        iterator                    find(const key_type& key)                                   { return s2T2T::find(key); }
        const_iterator              begin()                                             const   { return s2T2T::begin(); }
        const_iterator              end()                                               const   { return s2T2T::end(); }
        const_iterator              find(const key_type& key)                           const   { return s2T2T::find(key); }
        void                        clear()                                                     { s2T2T::clear(); }
};
}

}
}

}
#endif /* _KUL_HASH_HPP_ */
