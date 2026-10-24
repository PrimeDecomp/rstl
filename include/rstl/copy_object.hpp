#ifndef _RSTL_COPY_OBJECT
#define _RSTL_COPY_OBJECT

#include "Kyoto/Alloc/CMemory.hpp"
#include "rstl/rmemory_allocator.hpp"

namespace rstl {
// Guessed name. Empty allocation policy of copy_object.
struct copy_object_allocator {};

// Guessed name. Empty cloning policy of copy_object: the copy is made with the copy constructor.
template < typename T >
class copy_constructor_clone {};

// Guessed name. Owns one heap object and duplicates it when the owner is copied (the target's
// duplicate is allocated at "copy_object.h" line 31). The two policies are empty and are copied
// byte-wise like any empty class.
template < typename T, typename Alloc = copy_object_allocator,
           typename Clone = copy_constructor_clone< T > >
class copy_object {
public:
  explicit copy_object(T* ptr, const Alloc& alloc = Alloc(), const Clone& clone = Clone())
  : mAlloc(alloc)
  , mClone(clone)
  , mPtr(ptr) {}

  copy_object(const copy_object& other)
  : mAlloc(other.mAlloc)
  , mClone(other.mClone)
  , mPtr(other.mPtr != nullptr ? rs_new_line_in("copy_object.h", 31) T(*other.mPtr) : nullptr) {}

  ~copy_object() { delete mPtr; }

  T& operator*() { return *mPtr; }
  const T& operator*() const { return *mPtr; }
  T* operator->() { return mPtr; }
  const T* operator->() const { return mPtr; }

private:
  copy_object& operator=(const copy_object&);

  Alloc mAlloc;
  Clone mClone;
  T* mPtr;
};
} // namespace rstl

#endif // _RSTL_COPY_OBJECT
