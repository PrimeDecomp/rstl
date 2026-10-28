#ifndef _RSTL_STREAM_OBJECTS
#define _RSTL_STREAM_OBJECTS

#include "Kyoto/Streams/COutputStream.hpp"

#include "rstl/reserved_vector.hpp"

namespace rstl {

template < typename Iter >
inline void StreamObjects(COutputStream& out, const Iter& begin, const Iter& end, int) {
  Iter iterEnd = end;
  for (Iter iter = begin; iter != iterEnd; ++iter) {
    out.Put(*iter);
  }
}

template < typename T, int N >
inline void reserved_vector< T, N >::PutTo(COutputStream& out) const {
  const int count = size();
  out.Put(count);
  StreamObjects(out, begin(), end(), count);
}

} // namespace rstl

#endif // _RSTL_STREAM_OBJECTS
