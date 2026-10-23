#ifndef _RSTL_OWNERSHIP_TRANSFER
#define _RSTL_OWNERSHIP_TRANSFER

#include "rstl/RstlVersions.h"
#include "types.h"

namespace rstl {
template < typename T >
class ownership_transfer {
  mutable bool mOwns;
  mutable T* mPtr;

public:
  ownership_transfer() : mOwns(false), mPtr(nullptr) {}
  ownership_transfer(T* ptr) : mOwns(ptr != nullptr), mPtr(ptr) {}
  template < typename U >
  ownership_transfer(U* ptr) : mOwns(ptr != nullptr), mPtr(static_cast< T* >(ptr)) {}
  ownership_transfer(const ownership_transfer& other)
  : mOwns(other.mOwns), mPtr(other.mPtr) {
    other.mOwns = false;
  }
  ~ownership_transfer() { dec_ref_count(); }

  void operator=(const ownership_transfer& other) {
    if (&other != this) {
      if (mOwns) {
        delete mPtr;
      }
      mOwns = other.mOwns;
      mPtr = other.mPtr;
      other.mOwns = false;
    }
  }

  void dec_ref_count() {
    if (mOwns) {
      delete mPtr;
    }
  }
  T* take_ownership() const {
#if RSTL_OWNERSHIP_TRANSFER_CHECKED_TAKE
    if (mOwns) {
      mOwns = false;
      return mPtr;
    }
    return nullptr;
#else
    mOwns = false;
    return mPtr;
#endif
  }
  T* get() const { return mPtr; }
  T* operator->() const { return get(); }
  T& operator*() const { return *get(); }
  bool owns_object() const { return mOwns; }
};

} // namespace rstl

#endif // _RSTL_OWNERSHIP_TRANSFER
