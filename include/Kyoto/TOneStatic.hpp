#ifndef _TONESTATIC
#define _TONESTATIC
#include "rstl/RstlVersions.h"

#include "stdio.h"
#include "types.h"

template < typename T >
class TOneStatic {
public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#ifndef __MWERKS__
  // For clangd
  void* operator new(const size_t sz);
#endif
  void* operator new(size_t sz, const char* const fileAndLine, const char* const type);
#else
  void* operator new(size_t sz, const char*, const char*);
  void* operator new(size_t sz) { return operator new(sz, "??(??)", nullptr); }
#endif

  void operator delete(void* ptr);

private:
  static void* GetAllocSpace();
  static uint& ReferenceCount();
};

template < typename T >
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
uint& TOneStatic< T >::ReferenceCount() {
  static uint sReferenceCount = 0;
  return sReferenceCount;
}
#else
void* TOneStatic< T >::operator new(size_t sz, const char*, const char*) {
  ReferenceCount()++;
  return GetAllocSpace();
}
#endif

template < typename T >
void* TOneStatic< T >::GetAllocSpace() {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  ALIGNAS(T) static char sAllocSpace[sizeof(T)];
#else
  ALIGNAS(T) static uchar sAllocSpace[sizeof(T)];
#endif

  return &sAllocSpace;
}

template < typename T >
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void* TOneStatic< T >::operator new(size_t sz, const char* const fileAndLine,
                                    const char* const type) {
  ReferenceCount()++;
  return GetAllocSpace();
}
#else
uint& TOneStatic< T >::ReferenceCount() {
  static uint sReferenceCount = 0;
  return sReferenceCount;
}
#endif

template < typename T >
void TOneStatic< T >::operator delete(void* ptr) {
  ReferenceCount()--;
}

#endif // _TONESTATIC
