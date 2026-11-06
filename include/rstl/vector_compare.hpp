#ifndef _RSTL_VECTOR_COMPARE
#define _RSTL_VECTOR_COMPARE

#include "rstl/algorithm.hpp"
#include "rstl/vector.hpp"

namespace rstl {

// Guessed names. Element-wise comparison built on mismatch. Kept out of vector.hpp so that
// including it does not pull algorithm.hpp (and its RSTL_INLINE_SWAP check) into every user of
// vector.
template < typename T, typename Alloc >
bool operator==(const vector< T, Alloc >& left, const vector< T, Alloc >& right) {
  bool equal = false;
  if (left.size() == right.size()) {
    typename vector< T, Alloc >::const_iterator first = left.begin();
    typename vector< T, Alloc >::const_iterator last = left.end();
    typename vector< T, Alloc >::const_iterator second = right.begin();
    if (mismatch(first, last, second).first == last) {
      equal = true;
    }
  }
  return equal;
}

template < typename T, typename Alloc >
bool operator!=(const vector< T, Alloc >& left, const vector< T, Alloc >& right) {
  return !(left == right);
}

} // namespace rstl

#endif // _RSTL_VECTOR_COMPARE
