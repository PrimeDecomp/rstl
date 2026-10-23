#ifndef _RSTL_RESERVED_VECTOR
#define _RSTL_RESERVED_VECTOR

#include "rstl/RstlVersions.h"

#include "types.h"

#include "rstl/construct.hpp"
#include "rstl/pointer_iterator.hpp"

class CInputStream;
class COutputStream;

namespace rstl {
#if RSTL_RESERVED_VECTOR_FILL_TYPE
// Guessed name; scalar byte fills pass their value directly.
template < typename T >
struct reserved_vector_fill_type {
  typedef const T& type;
};
template <>
struct reserved_vector_fill_type< uchar > {
  typedef uchar type;
};
#endif
#if RSTL_RESERVED_VECTOR_TRAITS
// Scalar fill and destruction behavior recovered from Trilogy's input filters.
template < typename T >
struct reserved_vector_traits {
  typedef const T& fill_type;
  typedef const T& push_type;
  enum { trivial_destructor = false };
  static void fill(T* dest, int count, const T& value) { uninitialized_fill_n(dest, count, value); }
};

template <>
struct reserved_vector_traits< bool > {
  typedef bool fill_type;
  typedef bool push_type;
  enum { trivial_destructor = true };
  static void fill(bool* dest, int count, bool value) {
    for (int i = 0; i < count; ++i, ++dest) {
      *dest = value;
    }
  }
};

template <>
struct reserved_vector_traits< uchar > {
  typedef uchar fill_type;
  typedef const uchar& push_type;
  enum { trivial_destructor = true };
  static void fill(uchar* dest, int count, uchar value) {
    for (int i = 0; i < count; ++i, ++dest) {
      *dest = value;
    }
  }
};

template <>
struct reserved_vector_traits< float > {
  typedef float fill_type;
  typedef float push_type;
  enum { trivial_destructor = true };
  static void fill(float* dest, int count, float value) {
    for (int i = 0; i < count; ++i) {
      dest[i] = value;
    }
  }
};
#endif

template < typename T, int N >
class reserved_vector {
  int mCount;
#if RSTL_RESERVED_VECTOR_ALIGNED_DATA
  ALIGNAS(T) uchar mData[N * sizeof(T)];
#else
  uchar mData[N * sizeof(T)];
#endif

public:
  // typedef pointer_iterator< T, reserved_vector< T, N >, void > iterator;
  // typedef const_pointer_iterator< T, reserved_vector< T, N >, void > const_iterator;
  typedef T* iterator;
  typedef const T* const_iterator;
  typedef T value_type;

  inline iterator begin() { return iterator(data()); }
  inline const_iterator begin() const { return const_iterator(data()); }
  inline iterator end() { return iterator(data() + mCount); }
  inline const_iterator end() const { return const_iterator(data() + mCount); }

  reserved_vector() : mCount(0) {}
#if RSTL_RESERVED_VECTOR_TRAITS
  explicit reserved_vector(int count) : mCount(count) {
    int i;
    T* dest = data();
    for (i = 0; i < count; ++i, ++dest) {
      new (dest) T;
    }
  }
#endif
#if RSTL_SCALAR_TRAITS
  explicit reserved_vector(typename reserved_vector_traits< T >::fill_type value) : mCount(N) {
    reserved_vector_traits< T >::fill(data(), N, value);
  }
  explicit reserved_vector(int count, typename reserved_vector_traits< T >::fill_type value)
  : mCount(count) {
    reserved_vector_traits< T >::fill(data(), count, value);
  };
#else
  explicit reserved_vector(const T& value) : mCount(N) { uninitialized_fill_n(data(), N, value); }
  explicit reserved_vector(int count, const T& value) : mCount(count) {
#if !RSTL_PRECONDITION_MACRO
    RS_ASSERT(count <= N, "rstl precondition");
#endif
    uninitialized_fill_n(data(), count, value);
  };
#endif
  reserved_vector(const reserved_vector& other) : mCount(other.mCount) {
#if RSTL_RESERVED_VECTOR_POLICY_COPY
    // Scalars are copied by assignment. Class types, even those rstl constructs by assignment
    // elsewhere (CVector3f, CVector2f), are placement-new'ed with its null check.
    const T* it = other.data();
    T* cur = data();
    for (int remaining = mCount; remaining != 0; --remaining, ++it, ++cur) {
      construction_policy< T, is_scalar_type< T >::value >::construct(cur, *it);
    }
  }
#else
    RSTL_PRECONDITION(mCount >= 0);
#if RSTL_RESERVED_VECTOR_RAW_COPY
    uninitialized_copy_n(other.data(), mCount, data());
#else
    uninitialized_copy_n(const_iterator(other.data()), mCount, begin());
#endif
  }
#endif
#if RSTL_RESERVED_VECTOR_RANGE_CTOR
  reserved_vector(const T* first, const T* last) : mCount(0) {
    mCount = last - first;
    uninitialized_copy(first, last, data());
  }
#endif
  reserved_vector(CInputStream& in);

#if RSTL_RESERVED_VECTOR_OUT_OF_CLASS
  reserved_vector& operator=(const reserved_vector& other);

  void clear() {
    destroy_elements();
    mCount = 0;
  }

  ~reserved_vector() { destroy_elements(); }

  void push_back(const T& in) {
    construct(data() + mCount, in);
    ++mCount;
  }

  void pop_back() {
#if RSTL_RESERVED_VECTOR_POP_BACK_DATA_INDEX
    destroy(&data()[mCount - 1]);
#else
    destroy(end() - 1);
#endif
    --mCount;
  }
#else
  reserved_vector& operator=(const reserved_vector& other) {
    if (this == &other) {
      return *this;
    }
#if RSTL_SCALAR_TRAITS
    if (!reserved_vector_traits< T >::trivial_destructor) {
      T* ptr = data();
      for (int i = 0; i < mCount; ++i) {
        destroy(&ptr[i]);
      }
    }
#else
    clear();
#endif
    uninitialized_copy(other.data(), other.data() + other.size(), data());
    mCount = other.mCount;
    return *this;
  }

  void clear() {
    RS_ASSERT(mCount <= N, "rstl invariant");
#if RSTL_RESERVED_VECTOR_TRAITS
    if (!reserved_vector_traits< T >::trivial_destructor) {
#endif
      T* ptr = data();
      for (int i = 0; i < mCount; ++i) {
        destroy(&ptr[i]);
      }
#if RSTL_RESERVED_VECTOR_TRAITS
    }
#endif
    mCount = 0;
  }

  ~reserved_vector() {
#if RSTL_RESERVED_VECTOR_TRAITS
    if (!reserved_vector_traits< T >::trivial_destructor) {
      T* ptr = data();
      for (int i = 0; i < mCount; ++i) {
        destroy(&ptr[i]);
      }
    }
#else
    clear();
#endif
  }

#if RSTL_SCALAR_TRAITS
  void push_back(typename reserved_vector_traits< T >::push_type in) {
#else
  void push_back(const T& in) {
#endif
    RS_ASSERT(mCount < N, "rstl precondition");
    construct(data() + mCount, in);
    ++mCount;
  }

  void pop_back() {
    destroy(&data()[mCount - 1]);
    --mCount;
  }
#endif

  inline T* data() { return reinterpret_cast< T* >(mData); }
  inline const T* data() const { return reinterpret_cast< const T* >(mData); }
  inline bool empty() const { return size() == 0; }
  inline int size() const { return mCount; }
  inline int capacity() const { return N; }
#if RSTL_RESERVED_VECTOR_SET_SIZE
  // Guessed name. Sets the count without constructing elements; the caller writes them
  // through data().
  inline void set_size(int count) { mCount = count; }
#endif
  inline T& front() { return data()[0]; }
  inline const T& front() const { return data()[0]; }
  inline T& back() { return at(mCount - 1); }
  inline const T& back() const { return at(mCount - 1); }
#if RSTL_INDEX_PRECONDITION
  inline T& operator[](int idx) {
    RSTL_PRECONDITION(idx >= 0 && idx < mCount);
    return data()[idx];
  }
#else
  inline T& operator[](int idx) { return data()[idx]; }
#endif
#if RSTL_RESERVED_VECTOR_CONST_INDEX_PRECONDITION
  inline const T& operator[](int idx) const {
    RSTL_PRECONDITION(idx >= 0 && idx < mCount);
    return data()[idx];
  }
#else
  inline const T& operator[](int idx) const { return data()[idx]; }
#endif
  inline T& at(int idx) { return data()[idx]; }
  inline const T& at(int idx) const { return data()[idx]; }
  iterator erase(iterator it) {
    if (it >= begin() && it < end()) {
      for (iterator j = it; j < end() - 1; ++j) {
#if RSTL_SCALAR_TRAITS
        destroy(j);
        construct(j, *(j + 1));
#else
        *j = *(j + 1);
#endif
      }
      destroy(end() - 1);
      --mCount;
      return it;
    }
    return end();
  }

#if RSTL_RESERVED_VECTOR_FILL_TYPE
  // Guessed name. For scalar storage that the caller fills before reading.
  void resize_uninitialized(int count) { mCount = count; }

  void resize(int count, typename reserved_vector_fill_type< T >::type item = T()) {
    if (mCount == count) {
      return;
    }
    if (mCount > count) {
      destroy(data() + count, data() + mCount);
    } else {
      uninitialized_fill_n(data() + mCount, count - mCount, item);
    }
    mCount = count;
  }
#elif RSTL_RESERVED_VECTOR_SHRINKING_RESIZE
  void resize(int count, const T& item = T()) {
    if (mCount == count) {
      return;
    }
    if (mCount > count) {
      destroy(begin() + count, end());
    } else {
      uninitialized_fill_n(data() + mCount, count - mCount, item);
    }
    mCount = count;
  }
#else
  void resize(int count, const T& item = T()) {
    if (mCount < count) {
      uninitialized_fill_n(data() + mCount, count - mCount, item);
      mCount = count;
    }
  }
#endif

  void PutTo(COutputStream& out) const;
#if RSTL_RESERVED_VECTOR_OUT_OF_CLASS

private:
  void destroy_elements() {
    if (is_trivially_destructible< T >::value) {
      return;
    }
#if RSTL_PRIME1
    RSTL_PRECONDITION(mCount >= 0);
#endif
#if RSTL_RESERVED_VECTOR_DESTROY_INDEXED
    T* ptr = data();
#else
    T* const ptr = data();
#endif
    for (int i = 0; i < mCount; ++i) {
#if RSTL_RESERVED_VECTOR_DESTROY_INDEXED
      destroy(&ptr[i]);
#else
      destroy(ptr + i);
#endif
    }
  }
};

template < typename T, int N >
#if RSTL_RESERVED_VECTOR_DONT_INLINE_OPTION
#ifndef RSTL_DONT_INLINE_RESERVED_VECTOR
inline
#endif
    reserved_vector< T, N >& reserved_vector< T, N >::operator=(const reserved_vector& other) {
#else
inline reserved_vector< T, N >& reserved_vector< T, N >::operator=(const reserved_vector& other) {
#endif
  if (this != &other) {
    destroy_elements();
    uninitialized_copy(other.data(), other.data() + other.size(), data());
    mCount = other.mCount;
  }
  return *this;
}
#else
};

#endif

// template < typename T, int N >
// reserved_vector< T, N >::reserved_vector(int count, const T& value) : x0_count(count) {
//   uninitialized_fill_n(data(), count, value);
// }

} // namespace rstl

#endif // _RSTL_RESERVED_VECTOR
