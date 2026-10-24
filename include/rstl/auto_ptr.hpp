#ifndef _RSTL_AUTO_PTR
#define _RSTL_AUTO_PTR

#include "rstl/RstlVersions.h"
#if RSTL_AUTO_PTR_PROTO
#include "Kyoto/Alloc/CMemory.hpp"
#else
#include "types.h"
#endif

namespace rstl {
#if RSTL_AUTO_PTR_PROTO
template < typename T >
class auto_ptr {
  mutable bool mOwns;
  T* mPointer;

public:
  auto_ptr(T* pointer = 0) : mOwns(pointer != 0), mPointer(pointer) {}
  auto_ptr(const auto_ptr& other) : mOwns(other.mOwns), mPointer(other.mPointer) {
    other.mOwns = false;
  }
  ~auto_ptr() {
    if (mOwns) {
      delete mPointer;
    }
  }

  auto_ptr& operator=(const auto_ptr& other) {
    if (&other != this) {
      if (mOwns) {
        delete mPointer;
      }
      mOwns = other.mOwns;
      mPointer = other.mPointer;
      other.mOwns = false;
    }
    return *this;
  }

  T* get() const { return mPointer; }
  T& operator*() const { return *mPointer; }
  T* release() const {
    if (mOwns) {
      mOwns = false;
      return mPointer;
    }
    return 0;
  }
  bool null() const { return mPointer == 0; }
  void reset() {
    mOwns = false;
    mPointer = 0;
  }
  T* operator->() const { return mPointer; }
  bool owner() const { return mOwns; }
};
#else
template < typename T >
class auto_ptr {
  mutable bool mHas;
  T* mItem;

public:
  auto_ptr() : mHas(false), mItem(nullptr) {}
  auto_ptr(T* ptr) : mHas(ptr != nullptr), mItem(ptr) {}
  ~auto_ptr() {
    if (mHas) {
      delete mItem;
    }
  }
  // TODO check
  auto_ptr(const auto_ptr& other) : mHas(other.mHas), mItem(other.mItem) {
    other.mHas = false;
  }
#if RSTL_AUTO_PTR_CONVERSION
  template < typename U >
  friend class auto_ptr;

  template < typename U >
  auto_ptr(const auto_ptr< U >& other) : mHas(other.mHas), mItem(other.mItem) {
    other.mHas = false;
  }
#endif

  auto_ptr& operator=(const auto_ptr& other) {
    if (&other != this) {
      if (mHas) {
        delete mItem;
      }
      mHas = other.mHas;
      mItem = other.mItem;
      other.mHas = false;
    }
    return *this;
  }
  T* get() { return mItem; }
  T* get() const { return mItem; }
  bool owner() const { return mHas; }
  T* operator->() const { return mItem; }
  T& operator*() const { return *mItem; }
  T* release() const {
#if RSTL_AUTO_PTR_CHECKED_RELEASE
    if (mHas) {
      mHas = false;
      return mItem;
    }
    return nullptr;
#else
    mHas = false;
    return mItem;
#endif
  }
  bool null() const { return mItem == nullptr; }
  void reset() {
    mHas = false;
    mItem = nullptr;
  }
};
#endif
} // namespace rstl

#endif // _RSTL_AUTO_PTR
