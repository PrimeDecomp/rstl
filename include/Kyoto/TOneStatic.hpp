#ifndef _TONESTATIC
#define _TONESTATIC
#include "rstl/RstlVersions.h"
#include "rstl/macros.hpp"
#include "rstl/RstlVersions.h"

#include "stdio.h"
#include "rstl/types.h"

#if RSTL_CORRUPTION
#include "Kyoto/Alloc/Assert.hpp"



// A class that is only ever allocated once, into static storage. The prototype keeps the
// TOneStatic.h checks (lines 51/52 and 81/82); see the main.cpp instantiations at 0x8000F2CC
// (operator new), 0x8000F3E8 (operator delete), 0x8000F50C and 0x8000F518.
#endif
template < typename T >
class TOneStatic {
public:
#if RSTL_CORRUPTION
  void* operator new(size_t size, const char*, const char*);
#else
  void* operator new(size_t sz, const char* fileAndLine, const char* type);
#endif
  void* operator new(size_t sz) { return operator new(sz, "??(??)", nullptr); }

  void operator delete(void* ptr);

private:
  static void* GetAllocSpace();
  static uint& ReferenceCount();
};

template < typename T >
#if RSTL_CORRUPTION
void* TOneStatic< T >::GetAllocSpace() {
  static uchar sAllocSpace[sizeof(T)];
  return &sAllocSpace;
}

template < typename T >
#endif
uint& TOneStatic< T >::ReferenceCount() {
  static uint sReferenceCount = 0;
  return sReferenceCount;
}


template < typename T >
#if RSTL_CORRUPTION
void* TOneStatic< T >::operator new(size_t size, const char*, const char*) {
  uint& refCount = ReferenceCount();
  if (refCount != 0) {
    RS_VERIFY_FAILURE_IN("TOneStatic.h", 51, "refCount == 0", "false",
                         "Bad reference count for static new'd class");
#else
void* TOneStatic< T >::GetAllocSpace() {
#if RSTL_PRIME1
  ALIGNAS(T) static char sAllocSpace[sizeof(T)];
#else
  ALIGNAS(T) static uchar sAllocSpace[sizeof(T)];
#endif

  return &sAllocSpace;
#endif
}
#if RSTL_CORRUPTION
  if (size > sizeof(T)) {
    RS_VERIFY_FAILURE_IN("TOneStatic.h", 52, "size <= sizeof ( T )", "false",
                         "Bad size passed for operator new");
  }
  ++refCount;
#else

template < typename T >
void* TOneStatic< T >::operator new(size_t sz, const char* fileAndLine, const char* type) {
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
