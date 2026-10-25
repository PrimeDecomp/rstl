#ifndef _RSTL_SINGLE_PTR
#define _RSTL_SINGLE_PTR

#include "rstl/RstlVersions.h"
#include "rstl/rmemory_allocator.hpp"
#include "rstl/type_traits.hpp"
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
  single_ptr(const single_ptr& other) : mPtr(other.mPtr) {
#if RSTL_SINGLE_PTR_MUTABLE
    other.mPtr = nullptr; 
#else
    const_cast< single_ptr& >(other).mPtr = nullptr;
#endif
  }
#else
  single_ptr(single_ptr& other) : mPtr(other.mPtr) { other.mPtr = nullptr; }
#endif

  ~single_ptr() {
#if RSTL_SINGLE_PTR_DELETE_VIA_GET
    RSTL_DELETE_PTR(T, get());
#else
    RSTL_DELETE_PTR(T, mPtr);
#endif
  }

  single_ptr& operator=(single_ptr& other) {
    if (&other == this) {
      return *this;
    }
    RSTL_DELETE_PTR(T, mPtr);
    mPtr = other.mPtr;
    other.mPtr = nullptr;
    return *this;
  }

  single_ptr& operator=(T* const ptr) {
    RSTL_DELETE_PTR(T, mPtr);
    mPtr = ptr;
    return *this;
  }

  typename pointer_traits< T >::get_type get() const { return mPtr; }
  typename pointer_traits< T >::arrow_type operator->() const { return mPtr; }
  typename pointer_traits< T >::dereference_type operator*() { return *mPtr; }
  typename pointer_traits< T >::dereference_type_const operator*() const { return *mPtr; }

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

typedef single_ptr< void > unk_singleptr;
CHECK_SIZEOF(unk_singleptr, 0x4);
} // namespace rstl

#endif // _RSTL_SINGLE_PTR
