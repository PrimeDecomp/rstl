#ifndef _RSTL_AUTO_PTR
#define _RSTL_AUTO_PTR

#include "Kyoto/Alloc/CMemory.hpp"
#include "rstl/RstlVersions.h"
#include "rstl/pair.hpp"
#include "rstl/rmemory_allocator.hpp"
#include "rstl/type_traits.hpp"
#include "types.h"


namespace rstl {
template < typename T >
class auto_ptr {
  mutable bool mHas;
  T* mItem;

public:
#if RSTL_AUTO_PTR_CONSTRUCT_NULL
  auto_ptr(T* ptr = nullptr) : mHas(ptr != nullptr), mItem(ptr) {}
#else
  auto_ptr() : mHas(false), mItem(nullptr) {}
  auto_ptr(T* ptr) : mHas(ptr != nullptr), mItem(ptr) {}
#endif
  auto_ptr(const auto_ptr& other) : mHas(other.mHas), mItem(other.mItem) { other.mHas = false; }
  
#if RSTL_AUTO_PTR_CONVERSION
  template < typename U >
  friend class auto_ptr;

  template < typename U >
  auto_ptr(const auto_ptr< U >& other) : mHas(other.mHas), mItem(other.mItem) {
    other.mHas = false;
  }
#endif

  ~auto_ptr() {
    if (mHas) {
      RSTL_DELETE_PTR(T, mItem);
    }
  }

  auto_ptr& operator=(const auto_ptr& other) {
    if (&other != this) {
      if (mHas) {
        RSTL_DELETE_PTR(T, mItem);
      }
      mHas = other.mHas;
      mItem = other.mItem;
      other.mHas = false;
    }
    return *this;
  }
  typename pointer_traits< T >::get_type get() { return mItem; }
  typename pointer_traits< T >::get_type get() const { return mItem; }
  bool owner() const { return mHas; }
  typename pointer_traits< T >::get_type operator->() const { return mItem; }
  typename pointer_traits< T >::dereference_type operator*() const { return *mItem; }
  typename pointer_traits< T >::get_type release() const {
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

#if RSTL_PAIR_CONSTRUCTION_POLICY
// Guessed reconstruction: pairs holding an auto_ptr keep the generic copy policy.
template < typename L, typename R >
struct construction_policy< pair< L, auto_ptr< R > >, false > {
  static void construct(void* dest, const pair< L, auto_ptr< R > >& src) {
    RSTL_PRECONDITION(dest != nullptr);
    new (dest) pair< L, auto_ptr< R > >(src);
  }
};
template < typename L, typename R >
struct construction_policy< pair< auto_ptr< L >, R >, false > {
  static void construct(void* dest, const pair< auto_ptr< L >, R >& src) {
    RSTL_PRECONDITION(dest != nullptr);
    new (dest) pair< auto_ptr< L >, R >(src);
  }
};
#endif
} // namespace rstl

#endif // _RSTL_AUTO_PTR
