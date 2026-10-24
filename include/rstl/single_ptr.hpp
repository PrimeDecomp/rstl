#ifndef _RSTL_SINGLE_PTR
#define _RSTL_SINGLE_PTR

#include "rstl/RstlVersions.h"
#include "rstl/rmemory_allocator.hpp"
#include "types.h"

namespace rstl {
template < typename T >
class single_ptr {
#if RSTL_SINGLE_PTR_MUTABLE
  mutable T* mPtr;
#else
  T* mPtr;
#endif

public:
  single_ptr() : mPtr(nullptr) {}
  single_ptr(T* ptr) : mPtr(ptr) {}
#if RSTL_SINGLE_PTR_TRANSFER
#if RSTL_SINGLE_PTR_MUTABLE
  single_ptr(const single_ptr& other) : mPtr(other.mPtr) { other.mPtr = nullptr; }
#else
  single_ptr(const single_ptr& other) : mPtr(other.mPtr) {
    const_cast< single_ptr& >(other).mPtr = nullptr;
  }
#endif
#else
  single_ptr(single_ptr& other) : mPtr(other.mPtr) { other.mPtr = nullptr; }
#endif

  ~single_ptr() {
#if RSTL_SINGLE_PTR_DELETE_VIA_GET
    delete get();
#else
    delete mPtr;
#endif
  }

  single_ptr& operator=(single_ptr& other) {
    if (&other == this) {
      return *this;
    }
    delete mPtr;
    mPtr = other.mPtr;
    other.mPtr = nullptr;
    return *this;
  }

  single_ptr& operator=(T* const ptr) {
    delete mPtr;
    mPtr = ptr;
    return *this;
  }

  T* get() const { return mPtr; }
  // const T* get() const { return mPtr; }
#if RSTL_SINGLE_PTR_PLAIN_ACCESS
  T* operator->() const { return mPtr; }
#else
  T* const operator->() const { return mPtr; }
#endif
#ifndef CLANGD
  T& operator*() { return *mPtr; }
#if RSTL_SINGLE_PTR_PLAIN_ACCESS
  const T& operator*() const { return *mPtr; }
#else
  T& operator*() const { return *mPtr; }
#endif
#endif

  bool null() const { return mPtr == nullptr; }
  T* release() {
    T* ptr = mPtr;
    mPtr = nullptr;
    return ptr;
  }

  // This is certainly not real, but handy to force not-inline
  single_ptr& reset(T* ptr);

  operator bool() const { return !null(); }
};

template < typename T >
single_ptr< T >& single_ptr< T >::reset(T* ptr) {
  return *this = ptr;
}

#if RSTL_UNK_SINGLE_PTR_VOID
typedef single_ptr< void > unk_singleptr;
#else
typedef single_ptr< char > unk_singleptr;
#endif
CHECK_SIZEOF(unk_singleptr, 0x4);
} // namespace rstl

#endif // _RSTL_SINGLE_PTR
