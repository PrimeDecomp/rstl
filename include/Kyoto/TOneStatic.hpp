#ifndef _TONESTATIC
#define _TONESTATIC
#include "rstl/macros.hpp"
#include "rstl/RstlVersions.h"

#include "stdio.h"
#include "rstl/types.h"

#if RSTL_CORRUPTION
#include "Kyoto/Alloc/Assert.hpp"
#endif

template < typename T >
class TOneStatic {
public:
  void* operator new(size_t sz, const char* fileAndLine, const char* type);
  void* operator new(size_t sz) { return operator new(sz, "??(??)", nullptr); }

  void operator delete(void* ptr);

private:
  static void* GetAllocSpace();
  static uint& ReferenceCount();
};

template < typename T >
uint& TOneStatic< T >::ReferenceCount() {
  static uint sReferenceCount = 0;
  return sReferenceCount;
}


template < typename T >
void* TOneStatic< T >::GetAllocSpace() {
#if RSTL_PRIME1
  ALIGNAS(T) static char sAllocSpace[sizeof(T)];
#else
  ALIGNAS(T) static uchar sAllocSpace[sizeof(T)];
#endif

  return &sAllocSpace;
}

template < typename T >
void* TOneStatic< T >::operator new(size_t sz, const char* fileAndLine, const char* type) {
#if RSTL_CORRUPTION
  uint& refCount = ReferenceCount();
  if (refCount != 0) {
    RS_VERIFY_FAILURE_IN("TOneStatic.h", 51, "refCount == 0", "false",
                         "Bad reference count for static new'd class");
  }
  if (sz > sizeof(T)) {
    RS_VERIFY_FAILURE_IN("TOneStatic.h", 52, "size <= sizeof ( T )", "false",
                         "Bad size passed for operator new");
  }
  ++refCount;
#else
  ReferenceCount()++;
#endif
  return GetAllocSpace();
}

template < typename T >
void TOneStatic< T >::operator delete(void* ptr) {
#if RSTL_CORRUPTION
  uint& refCount = ReferenceCount();
  if (refCount != 1) {
    RS_VERIFY_FAILURE_IN("TOneStatic.h", 81, "refCount == 1", "false",
                         "Bad reference count for static delete'd class");
  }
  if (!(ptr == GetAllocSpace())) {
    RS_VERIFY_FAILURE_IN("TOneStatic.h", 82, "p == GetAllocSpace ()", "false",
                         "ERROR - Trying to delete the wrong pointer!");
  }
  --refCount;
#else
  ReferenceCount()--;
#endif
}

#endif // _TONESTATIC
