#ifndef _RSTL_RC_PTR
#define _RSTL_RC_PTR

#include "rstl/RstlVersions.h"

#if RSTL_VERSION == RSTL_G2MEAB
#include "Corruption/Kyoto/Alloc/CMemory.hpp"
#else
#include "Kyoto/Alloc/CMemory.hpp"
#endif
#include "rstl/construct.hpp"
#include "rstl/rmemory_allocator.hpp"
#include "types.h"

namespace rstl {
#if RSTL_RC_PTR_PRIVATE_COUNT
// Reference count shared by every null rc_ptr; it starts high so it is never released.
// The name comes from the Wii SEL export sNull__Q24rstl14rc_ptr_private.
class rc_ptr_private {
public:
  static int sNull;
};

#else
class CRefData {
public:
  CRefData(const void* ptr) : mPtr(ptr), mRefCount(1) {}
  CRefData(const void* ptr, int refCount) : mPtr(ptr), mRefCount(refCount) {}
  ~CRefData() {}

  void* GetPtr() const { return const_cast< void* >(mPtr); }
  int GetRefCount() const { return mRefCount; }
  int AddRef() { return ++mRefCount; }
  int DelRef() { return --mRefCount; }

  const void* mPtr;
  int mRefCount;

#if RSTL_RC_PTR_INLINE_COUNT
  static int sNull;
#else
  static CRefData sNull;
#endif
};
#endif

#if RSTL_RC_PTR_PRIVATE_COUNT
template < typename T >
class rc_ptr {
public:
  rc_ptr() : mPtr(nullptr), mRefCount(&rc_ptr_private::sNull) { ++*mRefCount; }
  rc_ptr(const T* ptr) : mPtr(ptr), mRefCount(RSTL_RC_PTR_NEW_COUNT_CTOR int(1)) {}
  // The empty statements stand in for release-build precondition checks; they count toward
  // MWCC's inline size limit (rbtree node ctors holding rc_ptr pairs are outlined).
  rc_ptr(const rc_ptr& other) : mPtr(other.mPtr), mRefCount(other.mRefCount) {
    RSTL_PRECONDITION(mRefCount != nullptr);
    RSTL_PRECONDITION(*mRefCount > 0);
    *mRefCount += 1;
  }
  template < typename U >
  explicit rc_ptr(const rc_ptr< U >& other)
  : mPtr(static_cast< const T* >(other.GetPtr())), mRefCount(other.GetRefCountPtr()) {
    ++*mRefCount;
  }
#if RSTL_RC_PTR_DTOR_PRECONDITION
  ~rc_ptr() {
    RSTL_PRECONDITION(mRefCount != nullptr);
    ReleaseData();
  }
#else
  ~rc_ptr() { ReleaseData(); }
#endif
  rc_ptr& operator=(const rc_ptr& other) {
    if (mPtr != other.mPtr) {
      ReleaseData();
      mPtr = other.mPtr;
      mRefCount = other.mRefCount;
      ++*mRefCount;
    }
    return *this;
  }
  T* GetPtr() const { return const_cast< T* >(mPtr); }
  int* GetRefCountPtr() const { return mRefCount; }
  bool IsNull() const { return GetPtr() == nullptr; }
  template < typename U >
  void Assign(const U* ptr) {
    const T* base = ptr;
    ReleaseData();
    mPtr = base;
    mRefCount = RSTL_RC_PTR_NEW_COUNT_ASSIGN int(1);
  }
  void ReleaseData();
  // Guessed name; shares the reference count with a pointer to a derived class.
  template < typename U >
  rc_ptr< U > Cast() const;
  void reset() {
    ReleaseData();
    mPtr = nullptr;
    mRefCount = &rc_ptr_private::sNull;
    ++*mRefCount;
  }
#if RSTL_RC_PTR_GET_PTR_ACCESS
  T* operator->() const { return GetPtr(); }
#else
  T* operator->() const { return const_cast< T* >(mPtr); }
#endif
  T& operator*() const { return *const_cast< T* >(mPtr); }
#if RSTL_RC_PTR_GET_PTR_ACCESS
  operator bool() const { return GetPtr() != nullptr; }
#else
  operator bool() const { return mPtr != nullptr; }
#endif

private:
  const T* mPtr;
  int* mRefCount;
};

template < typename T >
void rc_ptr< T >::ReleaseData() {
  if (--*mRefCount <= 0) {
    delete GetPtr();
#if RSTL_RC_PTR_GET_PTR_ACCESS
    delete mRefCount;
#else
    delete GetRefCountPtr();
#endif
  }
}

template < typename T >
template < typename U >
rc_ptr< U > rc_ptr< T >::Cast() const {
  return rc_ptr< U >(*this);
}

#else
template < typename T >
class rc_ptr {
public:
#if RSTL_RC_PTR_INLINE_COUNT
  rc_ptr() : mPtr(nullptr), mCount(&CRefData::sNull) { ++*mCount; }
  rc_ptr(const T* ptr) : mPtr(ptr), mCount(rs_new int(1)) {}
  rc_ptr(const rc_ptr& other) : mPtr(other.mPtr), mCount(other.mCount) { ++*mCount; }
#else
  rc_ptr() : mRefData(&CRefData::sNull) { mRefData->AddRef(); }
  rc_ptr(const T* ptr) : mRefData(rs_new CRefData(ptr)) {}
  rc_ptr(const rc_ptr& other) : mRefData(other.mRefData) {
    RS_ASSERT(mRefData != nullptr, "rstl precondition");
    RS_ASSERT(mRefData->GetRefCount() > 0, "rstl precondition");
    mRefData->AddRef();
  }
#endif
  ~rc_ptr() {
#if RSTL_RC_PTR_INLINE_COUNT
    RS_ASSERT(mCount != nullptr, "rstl precondition");
    RS_ASSERT(*mCount > 0, "rstl precondition");
#else
    RS_ASSERT(mRefData != nullptr, "rstl precondition");
    RS_ASSERT(mRefData->GetRefCount() > 0, "rstl precondition");
    RS_ASSERT(mRefData != &CRefData::sNull || mRefData->GetPtr() == nullptr, "rstl precondition");
#endif
    ReleaseData();
  }
  rc_ptr& operator=(const rc_ptr& other) {
#if RSTL_RC_PTR_INLINE_COUNT
    if (mCount != other.mCount) {
      ReleaseData();
      mPtr = other.mPtr;
      mCount = other.mCount;
      ++*mCount;
    }
#else
    RS_ASSERT(mRefData != nullptr, "rstl precondition");
    RS_ASSERT(other.mRefData != nullptr, "rstl precondition");
    RS_ASSERT(other.mRefData->GetRefCount() > 0, "rstl precondition");
    if (mRefData != other.mRefData) {
      ReleaseData();
      mRefData = other.mRefData;
      mRefData->AddRef();
    }
#endif
    return *this;
  }
#if RSTL_RC_PTR_INLINE_COUNT
  T* GetPtr() const { return const_cast< T* >(mPtr); }
#else
  T* GetPtr() const;
#endif
  bool IsNull() const { return GetPtr() == nullptr; }
  template < typename U >
  void Assign(const U* ptr) {
    const T* base = ptr;
    ReleaseData();
#if RSTL_RC_PTR_INLINE_COUNT
    mPtr = base;
    mCount = rs_new int(1);
#else
    mRefData = rs_new CRefData(base);
#endif
  }
  void ReleaseData();
  void reset() {
    ReleaseData();
#if RSTL_RC_PTR_INLINE_COUNT
    mPtr = nullptr;
    mCount = &CRefData::sNull;
    ++*mCount;
#else
    mRefData = &CRefData::sNull;
    mRefData->AddRef();
#endif
  }
  T* operator->() const { return GetPtr(); }
  T& operator*() const { return *GetPtr(); }
  operator bool() const { return GetPtr() != nullptr; }

private:
#if RSTL_RC_PTR_INLINE_COUNT
  const T* mPtr;
  int* mCount;
#else
  CRefData* mRefData;
#endif
};

#if !RSTL_RC_PTR_INLINE_COUNT
template < typename T >
#ifndef RSTL_OUTLINE_RC_PTR_GET
inline
#endif
T* rc_ptr< T >::GetPtr() const {
  return static_cast< T* >(mRefData->GetPtr());
}
#endif

template < typename T >
void rc_ptr< T >::ReleaseData() {
#if RSTL_RC_PTR_INLINE_COUNT
  if (--*mCount <= 0) {
    delete mPtr;
    delete mCount;
  }
#else
  if (mRefData->DelRef() <= 0) {
    T* const ptr = static_cast< T* >(mRefData->GetPtr());
    delete ptr;
    delete mRefData;
  }
#endif
}

#endif

template < typename T >
class ncrc_ptr : public rc_ptr< T > {
public:
  ncrc_ptr() {}
  ncrc_ptr(T* ptr) : rc_ptr< T >(ptr) {}
#if RSTL_NCRC_PTR_COPY_CTOR
  // Explicit copy ctor; its size keeps list<ncrc_ptr<T>>::create_node out of line.
  ncrc_ptr(const ncrc_ptr& other) : rc_ptr< T >(other) {
    RSTL_PRECONDITION(this->mRefCount != nullptr);
    RSTL_PRECONDITION(*this->mRefCount > 0);
  }
#endif
  ncrc_ptr(const rc_ptr< T >& other) : rc_ptr< T >(other) {}
  template < typename U >
  ncrc_ptr(const rc_ptr< U >& other) : rc_ptr< T >(other) {}
  ncrc_ptr& operator=(const rc_ptr< T >& other) {
    rc_ptr< T >::operator=(other);
    return *this;
  }
  template < typename U >
  ncrc_ptr& operator=(const U* ptr) {
    rc_ptr< T >::Assign(ptr);
    return *this;
  }

#if !RSTL_RC_PTR_PRIVATE_COUNT
  T* operator->() const { return rc_ptr< T >::operator->(); }
  T& operator*() const { return rc_ptr< T >::operator*(); }
  operator bool() const { return rc_ptr< T >::operator bool(); }
#endif
};

template < typename T >
#if RSTL_RC_PTR_PRIVATE_COUNT
inline bool operator==(const rc_ptr< T >& left, const rc_ptr< T >& right) {
#else
bool operator==(const rc_ptr< T >& left, const rc_ptr< T >& right) {
#endif
  return left.GetPtr() == right.GetPtr();
}

} // namespace rstl

#endif // _RSTL_RC_PTR
