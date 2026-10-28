#ifndef _RSTL_VECTOR
#define _RSTL_VECTOR

#include "rstl/RstlVersions.h"

#include "types.h"

#if RSTL_VERSION == RSTL_G2MEAB
#include "Kyoto/Alloc/Assert.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#endif

#include "rstl/allocator_auto_ptr.hpp"
#include "rstl/iterator.hpp"
#include "rstl/pointer_iterator.hpp"
#include "rstl/rmemory_allocator.hpp"
#include "rstl/type_traits.hpp"

class CInputStream;
class COutputStream;

namespace rstl {

template < typename T, typename Alloc = rmemory_allocator >
class vector
#if RSTL_ALLOCATOR_BASE_CLASS
: private Alloc
#endif
{
public:
#if !RSTL_ALLOCATOR_BASE_CLASS
  Alloc mAllocator;
#endif
  int mCount;
  int mCapacity;
  T* mItems;

public:
  typedef Alloc allocator_type;
  typedef pointer_iterator< T, vector< T, Alloc >, Alloc > iterator;
  typedef const_pointer_iterator< T, vector< T, Alloc >, Alloc > const_iterator;
  typedef int size_type;
  typedef T value_type;

#if RSTL_VECTOR_ITEM_BEGIN
  iterator begin() { return iterator(this, mItems); }
  const_iterator begin() const { return const_iterator(this, mItems); }
#else
  iterator begin() { return iterator(this, data()); }
  const_iterator begin() const { return const_iterator(this, data()); }
#endif
  iterator end() {
#if RSTL_VECTOR_CONST_LOCALS
    T* const end = data() + mCount;
#else
    T* const end = mItems + mCount;
#endif
    return iterator(end);
  }
  const_iterator end() const { return const_iterator(this, data() + size()); }
  vector(const Alloc& alloc = Alloc())
  :
#if RSTL_ALLOCATOR_BASE_CLASS
    Alloc(alloc),
#else
    mAllocator(alloc),
#endif
    mCount(0), mCapacity(0), mItems(nullptr) {}
#if RSTL_VECTOR_UNINITIALIZED_COUNT_CTOR
  // The count elements are uninitialized until the caller fills the storage.
  vector(int count) : mCount(0), mCapacity(0), mItems(0) {
#if RSTL_VECTOR_COUNT_CTOR_POSITIVE_CHECK
    if (count > 0)
#else
    if (count > mCount)
#endif
      reserve(count);
    mCount = count;
  }
#elif RSTL_VECTOR_RESERVE_CTOR
  vector(int count) : mCapacity(0), mItems(0) {
    mCount = 0;
    reserve(count);
  }
#else
  vector(int count) : mCount(0), mCapacity(0), mItems(0) { reserve(count); }
#endif
#if RSTL_VECTOR_DEFAULT_ALLOC_FILL_CTOR
  vector(int count, typename type_traits< T >::vector_fill_type v, const Alloc& alloc = Alloc())
  :
#if RSTL_ALLOCATOR_BASE_CLASS
      Alloc(alloc)
  ,
#else
      mAllocator(alloc)
  ,
#endif
  mCount(count)
  , mCapacity(count) {
#if RSTL_ALLOCATOR_INSTANCE
    mAllocator.allocate(mItems, mCount);
#else
    Alloc::allocate(mItems, mCount);
#endif
    uninitialized_fill_n(mItems, count, v);
  }
#else
  vector(int count, const T& v) : mCount(count), mCapacity(count) {
    Alloc::allocate(mItems, mCount);
    uninitialized_fill_n(mItems, count, v);
  }
  vector(int count, const T& v, const Alloc& alloc)
  :
#if RSTL_ALLOCATOR_BASE_CLASS
    Alloc(alloc),
#else
    mAllocator(alloc),
#endif
    mCount(count), mCapacity(count) {
    Alloc::allocate(mItems, mCount);
    uninitialized_fill_n(mItems, count, v);
  }
#endif

#if RSTL_VECTOR_OUT_OF_CLASS
  vector(const vector& other);
#else
  vector(const vector& other)
  :
#if RSTL_ALLOCATOR_BASE_CLASS
    Alloc(other),
#else
    mAllocator(other.mAllocator),
#endif
    mCount(other.mCount)
  , mCapacity(other.mCapacity) {
    RS_ASSERT(other.mCount <= other.mCapacity, "rstl precondition");
    if (other.mCount == 0 && other.mCapacity == 0) {
      mItems = nullptr;
    } else {
      Alloc::allocate(mItems, mCapacity);
      RS_ASSERT(mItems != nullptr, "rstl precondition");
      uninitialized_copy_n(other.mItems, mCount, mItems);
    }
  }
#endif
  vector(CInputStream& in, const Alloc& alloc = Alloc());
  template < typename It >
  vector(It first, It last, const Alloc& alloc = Alloc())
  :
#if RSTL_ALLOCATOR_BASE_CLASS
    Alloc(alloc),
#else
    mAllocator(alloc),
#endif
    mCount(0), mCapacity(0) {
    mCount = mCapacity = rstl::distance(first, last);
#if RSTL_ALLOCATOR_INSTANCE
    mAllocator.allocate(mItems, mCount);
#else
    Alloc::allocate(mItems, mCount);
#endif
    rstl::uninitialized_copy(first, last, mItems);
  }
#if RSTL_VECTOR_OUT_OF_CLASS
  ~vector();
#else
  ~vector() {
    RS_ASSERT(mCount <= mCapacity, "rstl invariant");
    RS_ASSERT(mCapacity == 0 || mItems != nullptr, "rstl invariant");
    destroy(begin(), end());
    Alloc::deallocate(mItems);
  }
#endif

  inline void resize(int size, typename type_traits< T >::vector_resize_type in = T());
  // Guessed name. Sets the count without constructing the new elements; the caller fills them.
  void resize_uninitialized(int size) {
#if RSTL_VECTOR_RESIZE_UNINITIALIZED_COUNT_CHECK
    if (size > mCount) {
      reserve(size);
    }
#else
    if (size > mCapacity)
      reserve(size);
#endif
    mCount = size;
  }
#if RSTL_VECTOR_INLINE_DECLS
  inline
#endif
  void assign(int size, typename type_traits< T >::vector_fill_type in = T());
  void reserve(int size);
#if RSTL_VECTOR_INLINE_DECLS
  inline
#endif
  iterator insert(iterator it, typename type_traits< T >::vector_insert_type value);

  template < typename from_iterator >
#if RSTL_VECTOR_OUT_OF_CLASS
  void insert(iterator it, from_iterator begin, from_iterator end) {
    insert_into(it, rstl::distance(begin, end), begin);
  }
#else
  inline void insert(iterator it, from_iterator begin, from_iterator end);
#endif

  // iterator erase(iterator it);
  // iterator erase(iterator first, iterator last);

  iterator erase(iterator it);
  iterator erase(iterator first, iterator last);

#if RSTL_VECTOR_VERIFIED_PUSH_BACK
  void push_back(const T& in) {
    if (mCount >= mCapacity) {
      verify_capacity_failed();
    }
    rstl::construct(mItems + mCount++, in);
  }

  void push_back_unsafe(const T& in) {
    if (!(mCount < mCapacity)) {
      verify_capacity_failed();
    }
    T* items = mItems;
    rstl::construct(items + mCount++, in);
  }

  void push_back_checked(const T& in) {
    if (mCount >= mCapacity) {
      verify_capacity_failed();
    }
    push_back_unsafe(in);
  }
#elif RSTL_VECTOR_GROWING_PUSH_BACK
  void push_back(const T& in) {
    if (mCount >= mCapacity) {
      reserve(mCapacity != 0 ? mCapacity * 2 : 4);
    }
    rstl::construct(mItems + mCount, in);
    ++mCount;
  }

  void push_back_unsafe(const T& in) { rstl::construct(mItems + mCount++, in); }
#else
#if RSTL_VECTOR_UNCHECKED_PUSH_BACK
  void push_back(const T& in) { rstl::construct(mItems + mCount++, in); }
#else
  void push_back(const T& in) {
    if (mCount >= mCapacity) {
      reserve(mCapacity ? mCapacity * 2 : 4);
    }
    RS_ASSERT(mCount < mCapacity, "rstl invariant");
    rstl::construct(mItems + mCount, in);
    ++mCount;
  }
#endif
#endif

  void pop_back() {
    destroy(mItems + mCount - 1);
    --mCount;
  }

#if !RSTL_VECTOR_VERIFIED_PUSH_BACK && !RSTL_VECTOR_GROWING_PUSH_BACK
  void push_back_unsafe(const T& in) { rstl::construct(mItems + mCount++, in); }
#endif

  inline vector& operator=(const vector& other);

#if RSTL_VECTOR_OUT_OF_CLASS
  inline void clear();
#else
  void clear() {
    destroy(begin(), end());
    mCount = 0;
  }
#endif

  T* data() { return mItems; }
  const T* data() const { return mItems; }
  int size() const { return mCount; }
  bool empty() const { return mCount == 0; }
  int capacity() const { return mCapacity; }
  T& at(int idx) { return mItems[idx]; }
  const T& at(int idx) const { return mItems[idx]; }
  T& front() { return at(0); }
  const T& front() const { return at(0); }
  T& back() { return at(mCount - 1); }
  const T& back() const { return at(mCount - 1); }
#if RSTL_INDEX_PRECONDITION
  T& operator[](const int idx) {
    RSTL_PRECONDITION(idx >= 0 && idx < mCount);
    return mItems[idx];
  }
  const T& operator[](int idx) const {
    RSTL_PRECONDITION(idx >= 0 && idx < mCount);
    return mItems[idx];
  }
#else
  T& operator[](int idx) { return mItems[idx]; }
  const T& operator[](int idx) const { return mItems[idx]; }
#endif

  void PutTo(COutputStream& out) const;

protected:
  template < typename In >
  inline void insert_into(iterator at, int n, In in);

#if RSTL_VECTOR_VERIFIED_PUSH_BACK
  // Dolphin vector.h (line 482): no auto resize, asserts when the capacity is exhausted.
  void verify_capacity_failed() const {
    CCallStack stack(0, "vector.h(482) : ", kUnknownType);
    rs_log_assert_failure(&stack, "vector.h", 482, "Verify", "mSize < mCapacity",
                          CBasics::Stringize("Auto vector resize is not supported on dolphin, "
                                             "call reserve() yourself (capacity: %d)",
                                             mCapacity));
    rs_debugger_printf("Would have thrown exception: %s\n", "false");
    rs_halt_cpu();
  }
#endif
};

#if RSTL_VECTOR_OUT_OF_CLASS
template < typename T, typename Alloc >
inline vector< T, Alloc >::vector(const vector& other)
:
#if RSTL_ALLOCATOR_BASE_CLASS
    Alloc(other)
,
#else
    mAllocator(other.mAllocator)
,
#endif
mCount(other.mCount)
, mCapacity(other.mCapacity) {
  if (other.mCount == 0 && other.mCapacity == 0) {
    mItems = nullptr;
  } else {
#if RSTL_ALLOCATOR_INSTANCE
    mAllocator.allocate(mItems, mCapacity);
#else
    Alloc::allocate(mItems, mCapacity);
#endif
    uninitialized_copy_n(other.mItems, mCount, mItems);
  }
}

template < typename T, typename Alloc >
inline vector< T, Alloc >::~vector() {
  destroy(begin(), end());
#if RSTL_ALLOCATOR_INSTANCE
  mAllocator.deallocate(mItems);
#else
  Alloc::deallocate(mItems);
#endif
}

#endif
template < typename T, typename Alloc >
void vector< T, Alloc >::assign(int size, typename type_traits< T >::vector_fill_type in) {
  clear();
  reserve(size);
  for (int i = 0; i < size; ++i) {
#if RSTL_VECTOR_PUSH_BACK_UNSAFE
    push_back_unsafe(in);
#else
    push_back(in);
#endif
  }
}

template < typename T, typename Alloc >
inline void vector< T, Alloc >::resize(int size, typename type_traits< T >::vector_resize_type in) {
  if (mCount != size) {
    if (size > mCount) {
      reserve(size);
      uninitialized_fill_n(mItems + mCount, size - mCount, in);
    } else {
      destroy(begin() + size, end());
    }
    mCount = size;
  }
}

template < typename T, typename Alloc >
void vector< T, Alloc >::reserve(int newSize) {
  if (newSize <= mCapacity) {
    return;
  }

  T* newData;
#if RSTL_ALLOCATOR_INSTANCE
  mAllocator.allocate(newData, newSize);
#else
  Alloc::allocate(newData, newSize);
#endif
  uninitialized_copy(begin(), end(), newData);
  destroy(mItems, mItems + mCount);
#if RSTL_ALLOCATOR_INSTANCE
  mAllocator.deallocate(mItems);
#else
  Alloc::deallocate(mItems);
#endif
  mItems = newData;
  mCapacity = newSize;
}

template < typename T, typename Alloc >
typename vector< T, Alloc >::iterator vector< T, Alloc >::insert(
    iterator it, typename type_traits< T >::vector_insert_type value) {
  typename iterator::difference_type diff = it.operator->() - mItems;
  const_counting_iterator< T > in(&value, 0);
  insert_into(it, 1, in);
  return iterator(mItems) + diff;
}

#if !RSTL_VECTOR_OUT_OF_CLASS
template < typename T, typename Alloc >
template < typename from_iterator >
void vector< T, Alloc >::insert(iterator it, from_iterator begin, from_iterator end) {
  insert_into(it, rstl::distance(begin, end), begin);
}

#endif
template < typename T, typename Alloc >
template < typename In >
void vector< T, Alloc >::insert_into(iterator at, int n, In in) {
  T* oldData = mItems;
  In input = in;

  if (mCount + n <= mCapacity) {
    long atIdx = at - begin();
    int moveCount = mCount - atIdx;
    int i = moveCount - 1;
    for (; i >= 0; --i) {
      construct(oldData + atIdx + n + i, data()[atIdx + i]);
#if RSTL_VECTOR_INSERT_CONST_ITEM
      T* const item = oldData + atIdx + i;
      destroy(item);
#else
      destroy(oldData + atIdx + i);
#endif
    }
    for (i = 0; i < n; ++input, ++i) {
      construct(oldData + atIdx + i, *input);
    }
    mCount += n;
  } else {
    int newCapacity = mCapacity ? mCapacity * 2 : 4;
    while (newCapacity < mCount + n) {
      newCapacity *= 2;
    }

    T* newData;
#if RSTL_ALLOCATOR_INSTANCE
    mAllocator.allocate(newData, newCapacity);
#else
    Alloc::allocate(newData, newCapacity);
#endif
    long atIdx = at - begin();
    // The const alias makes MWCC retain a separate allocation-base register on PAL.
#if RSTL_VECTOR_CONST_LOCALS
    T* const newItems = newData;
#else
    T* newItems = newData;
#endif
    int newIdx = 0;
#if RSTL_VECTOR_LONG_INSERT_INDEX
    for (long i = 0; i < atIdx; ++newIdx, ++i) {
#else
    for (int i = 0; i < atIdx; ++newIdx, ++i) {
#endif
      construct(newItems + newIdx, data()[i]);
    }
    for (int i = 0; i < n; ++input, ++newIdx, ++i) {
      construct(newItems + newIdx, *input);
    }
    for (int i = atIdx; i < size(); ++newIdx, ++i) {
      construct(newItems + newIdx, data()[i]);
    }

    destroy(oldData, oldData + size());
#if RSTL_ALLOCATOR_INSTANCE
    mAllocator.deallocate(mItems);
#else
    Alloc::deallocate(mItems);
#endif
    mItems = newData;
    mCapacity = newCapacity;
    mCount += n;
  }
}

template < typename T, typename Alloc >
inline vector< T, Alloc >& vector< T, Alloc >::operator=(const vector< T, Alloc >& other) {
  if (this == &other)
    return *this;
  clear();
  if (other.size() == 0) {
#if RSTL_ALLOCATOR_INSTANCE
    mAllocator.deallocate(mItems);
#else
    Alloc::deallocate(mItems);
#endif
    mCount = 0;
    mCapacity = 0;
    mItems = nullptr;
  } else {
    reserve(other.size());
    uninitialized_copy(other.mItems, other.mItems + other.mCount, data());
    mCount = other.mCount;
  }
  return *this;
}

template < typename T, typename Alloc >
#if RSTL_VECTOR_INLINE_DECLS
inline
#endif
typename vector< T, Alloc >::iterator vector< T, Alloc >::erase(iterator it) {
  return erase(it, it + 1);
}

template < typename T, typename Alloc >
#if RSTL_VECTOR_INLINE_DECLS
inline
#endif
typename vector< T, Alloc >::iterator vector< T, Alloc >::erase(iterator first, iterator last) {
  destroy(first, last);

  const typename iterator::difference_type tmp = first - begin();

  int newCount = tmp;

#if RSTL_VECTOR_ERASE_DEST_FIRST
  for (iterator moved = iterator(mItems + tmp), it = last; it != end(); ++moved, ++newCount, ++it) {
#else
  for (iterator it = last, moved = iterator(mItems + tmp); it != end(); ++moved, ++newCount, ++it) {
#endif
    construct(&*moved, *it);
    destroy(&*it);
  }
  mCount = newCount;

  return first;
}
#if RSTL_VECTOR_OUT_OF_CLASS
template < typename T, typename Alloc >
inline void vector< T, Alloc >::clear() {
  destroy(begin(), end());
  mCount = 0;
}
#endif

} // namespace rstl
#ifndef __MWERKS__
#include "Kyoto/Streams/CInputStream.hpp"
#endif
#include "rstl/stream_objects.hpp"
namespace rstl {
template < typename T, typename Alloc >
inline vector< T, Alloc >::vector(CInputStream& in, const Alloc& allocator)
:
#if RSTL_ALLOCATOR_BASE_CLASS
  Alloc(allocator),
#else
  mAllocator(allocator),
#endif
  mCount(0), mCapacity(0), mItems(nullptr) {
#if RSTL_STREAM_READ_INT32
  int count = in.ReadInt32();
#else
  int count = in.Get(TGetType(0));
#endif
#if RSTL_PRECONDITION_MACRO
  RSTL_PRECONDITION(count >= 0);
#else
  RS_ASSERT(count >= 0, "rstl precondition");
#endif
#if RSTL_VECTOR_STREAM_RESERVE_APPEND
  reserve(count + mCount);
#else
  reserve(count);
#endif
  for (int i = 0; i < count; i++) {
#if RSTL_VECTOR_PUSH_BACK_UNSAFE
    push_back_unsafe(in.Get< T >());
#else
    push_back(in.Get< T >());
#endif
  }
}

template < typename T, typename Alloc >
inline void vector< T, Alloc >::PutTo(COutputStream& out) const {
  const int count = size();
  out.Put(count);
  StreamObjects(out, begin(), end(), count);
}

} // namespace rstl

#endif // _RSTL_VECTOR
