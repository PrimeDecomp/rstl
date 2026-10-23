#ifndef _RSTL_SET
#define _RSTL_SET

#include "rstl/RstlVersions.h"

#include "types.h"

#include "rstl/pair.hpp"
#include "rstl/red_black_tree.hpp"
#include "rstl/rmemory_allocator.hpp"

namespace rstl {
#if RSTL_SET_INNER_TREE
template < typename T, typename Cmp = less< T >, typename Alloc = rmemory_allocator >
class set {
public:
  typedef T value_type;

private:
  typedef red_black_tree< T, value_type, 0, identity< T >, Cmp, Alloc > rep_type;

public:
  typedef typename rep_type::iterator iterator;
  typedef typename rep_type::const_iterator const_iterator;

  iterator insert(const value_type& item) { return inner.insert(item).first; }

  const_iterator begin() const { return inner.begin(); }
  const_iterator end() const { return inner.end(); }
  iterator begin() { return inner.begin(); }
  iterator end() { return inner.end(); }

  iterator find(const T& key) { return inner.find(key); }
  const_iterator find(const T& key) const { return inner.find(key); }

  void erase(iterator it) { inner.erase(it); }
  int erase(const T& key) { return inner.erase(key); }
  int size() const { return inner.size(); }
  void clear() { inner.clear(); }

private:
  rep_type inner;
};
#elif RSTL_SET_OUT_OF_CLASS
template < typename T, typename Cmp = less< T >, typename Alloc = rmemory_allocator >
class set : public red_black_tree< T, T, false, identity< T >, Cmp, Alloc > {
public:
  typedef T value_type;

private:
  typedef red_black_tree< T, value_type, false, identity< T >, Cmp, Alloc > rep_type;

public:
  typedef typename rep_type::iterator iterator;
  typedef typename rep_type::const_iterator const_iterator;

  set(const Cmp& cmp = Cmp(), const Alloc& alloc = Alloc())
  : rep_type(identity< T >(), cmp, alloc) {}

  iterator insert(const value_type& item) { return rep_type::insert(item).first; }
};
#else
template < typename T, typename Cmp = less< T >, typename Alloc = rmemory_allocator >
class set : public red_black_tree< T, T, false, identity< T >, Cmp, Alloc > {
public:
  typedef T value_type;

private:
  typedef red_black_tree< T, value_type, false, identity< T >, Cmp, Alloc > rep_type;

public:
  typedef typename rep_type::iterator iterator;
  typedef typename rep_type::const_iterator const_iterator;

  ~set() {}

  explicit set(const Cmp& cmp = Cmp(), const Alloc& alloc = Alloc())
  : rep_type(identity< T >(), cmp, alloc) {}

  set(CInputStream& in, const Cmp& cmp = Cmp(), const Alloc& alloc = Alloc());

  set(const set& other);
};

template < typename T, typename Cmp, typename Alloc >
#if RSTL_SET_INLINE_COPY_CTOR
inline
#endif
    set< T, Cmp, Alloc >::set(const set& other)
: rep_type(other) {
}
#endif

typedef set< char, char > unk_set;
CHECK_SIZEOF(unk_set, 0x14)

} // namespace rstl

#endif // _RSTL_SET
