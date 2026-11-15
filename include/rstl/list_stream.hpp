#ifndef _RSTL_LIST_STREAM
#define _RSTL_LIST_STREAM

#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/COutputStream.hpp"

#include "rstl/list.hpp"
#include "rstl/stream_objects.hpp"

namespace rstl {

template < typename T, typename Alloc >
inline list< T, Alloc >::list(CInputStream& in, const Alloc& alloc)
:
#if RSTL_ALLOCATOR_BASE_CLASS
    Alloc(alloc)
#else
    mAllocator(alloc)
#endif
, mStart(reinterpret_cast< node* >(&mEmpty_prev))
, mEnd(reinterpret_cast< node* >(&mEmpty_prev))
, mEmpty_prev(reinterpret_cast< node* >(&mEmpty_prev))
, mEmpty_next(reinterpret_cast< node* >(&mEmpty_prev))
, mCount(0) {
  const int count = in.ReadInt32();
  for (int i = 0; i < count; ++i) {
    push_back(in.Get< T >());
  }
}

template < typename T, typename Alloc >
inline void list< T, Alloc >::PutTo(COutputStream& out) const {
  const int count = size();
  out.Put(count);
  StreamObjects(out, begin(), end(), count);
}

} // namespace rstl

#endif // _RSTL_LIST_STREAM
