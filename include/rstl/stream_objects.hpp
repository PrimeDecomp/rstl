#ifndef _RSTL_STREAM_OBJECTS
#define _RSTL_STREAM_OBJECTS

#ifdef __MWERKS__
class COutputStream;
#else
#include "Kyoto/Streams/COutputStream.hpp"
#endif

namespace rstl {

template < typename Iter >
inline void StreamObjects(COutputStream& out, const Iter& begin, const Iter& end, int) {
  Iter iterEnd = end;
  for (Iter iter = begin; iter != iterEnd; ++iter) {
    out.Put(*iter);
  }
}

} // namespace rstl

#endif // _RSTL_STREAM_OBJECTS
