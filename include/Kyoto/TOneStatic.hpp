#ifndef _TONESTATIC
#define _TONESTATIC
#include "rstl/macros.hpp"
#include "rstl/RstlVersions.h"

#include "stdio.h"
#include "rstl/types.h"

#include "Kyoto/Alloc/Assert.hpp"

template < typename T >
class TOneStatic {
public:
  void* operator new(size_t size, const char* fileAndLine, const char* type);
  void* operator new(size_t size) { return operator new(size, "??(??)", nullptr); }

  void operator delete(void* p);

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
void* TOneStatic< T >::operator new(size_t size, const char* fileAndLine, const char* type) {
  uint& refCount = ReferenceCount();
  RS_VERIFY_THROW_DIRECT_IN("TOneStatic.h", 51, refCount == 0, false,
                            "Bad reference count for static new'd class");
  RS_VERIFY_THROW_DIRECT_IN("TOneStatic.h", 52, size <= sizeof ( T ), false,
                            "Bad size passed for operator new");
  ++refCount;
  return GetAllocSpace();
}

template < typename T >
void TOneStatic< T >::operator delete(void* p) {
  uint& refCount = ReferenceCount();
  RS_VERIFY_THROW_DIRECT_IN("TOneStatic.h", 81, refCount == 1, false,
                            "Bad reference count for static delete'd class");
  RS_VERIFY_THROW_IN("TOneStatic.h", 82, p == GetAllocSpace (), false,
                     "ERROR - Trying to delete the wrong pointer!");
  --refCount;
}

#endif // _TONESTATIC
