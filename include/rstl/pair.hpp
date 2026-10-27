#ifndef _RSTL_PAIR
#define _RSTL_PAIR

#include "rstl/RstlVersions.h"
#include "rstl/construct.hpp"
#include "rstl/functional.hpp"
#include "types.h"

class CInputStream;
class COutputStream;

namespace rstl {
template < typename L, typename R >
class pair {
public:
  pair() {}
  pair(CInputStream& in);
  pair(const L& first, const R& second) : first(first), second(second) {}
  void PutTo(COutputStream& out) const;

  bool operator==(const pair& other) const {
    return first == other.first && second == other.second;
  }

  bool operator!=(const pair& other) const {
    return first != other.first || second != other.second;
  }

  bool operator<(const pair& other) const {
    return first < other.first || (first == other.first && second < other.second);
  }

  L first;
  R second;
};

#if RSTL_RESERVED_VECTOR_TRAITS
template < typename L, typename R >
inline
    typename construction_result< is_scalar_type< L >::value && is_scalar_type< R >::value >::type
    construct(pair< L, R >* dest, const pair< L, R >& src) {
  *dest = src;
}
template < typename L, typename R >
inline
    typename construction_result< is_scalar_type< L >::value && is_scalar_type< R >::value >::type
    construct(pair< L, R >* dest, pair< L, R >& src) {
  *dest = src;
}

#endif

template < typename L, typename R >
inline pair< L, R > make_pair(L first, R second) {
  return pair< L, R >(first, second);
}

#if RSTL_CONSTRUCTION_POLICY || RSTL_TRIVIAL_INT_BOOL_DESTRUCTION || RSTL_RESERVED_VECTOR_TRAITS
template < typename L, typename R >
struct is_trivially_destructible< pair< L, R > > {
  enum {
    value =
#if RSTL_RESERVED_VECTOR_TRAITS
        (is_scalar_type< L >::value && is_scalar_type< R >::value) ||
#endif
    (is_trivially_destructible< L >::value && is_trivially_destructible< R >::value)
  };
};
#endif

#if RSTL_CONSTRUCTION_POLICY
template < typename L, typename R >
struct use_assignment_for_construction< pair< L, R > > {
  enum {
    value = use_assignment_for_construction< L >::value &&
    use_assignment_for_construction< R >::value
  };
};

#endif

#if RSTL_PAIR_INT_FLOAT_POLICY
// These legacy element policies retain the native construction null checks and
// destruction loops even though the individual members have trivial lifetimes.
template <>
struct is_trivially_destructible< pair< int, float > > {
  enum { value = false };
};

template <>
struct use_assignment_for_construction< pair< int, float > > {
  enum { value = false };
};
#endif

template < typename P >
struct select1st : unary_function< P, P > {
  const P& operator()(const P& it) const { return it; }
};

template < typename K, typename V >
struct select1st< pair< K, V > > : unary_function< pair< K, V >, K > {
  typedef K value_type;

  const K& operator()(const pair< K, V >& it) const { return it.first; }
};

#ifndef __MWERKS__
} // namespace rstl
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/COutputStream.hpp"
namespace rstl {
#endif
template < typename L, typename R >
inline pair< L, R >::pair(CInputStream& in)
: first(in.Get(TGetType(first))), second(in.Get(TGetType(second))) {}

template < typename L, typename R >
inline void pair< L, R >::PutTo(COutputStream& out) const {
  out.Put(first);
  out.Put(second);
}

} // namespace rstl

#endif // _RSTL_PAIR
