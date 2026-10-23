#ifndef _RSTL_LINEAR_ITERATOR
#define _RSTL_LINEAR_ITERATOR

#include "rstl/RstlVersions.h"
#include "rstl/iterator.hpp"

namespace rstl {
template < typename T, typename Container, typename Alloc >
class const_linear_iterator {
public:
  typedef T value_type;
  typedef long difference_type;
  typedef random_access_iterator_tag iterator_category;

  const_linear_iterator(const Container* owner, int index) : mOwner(owner), mIndex(index) {}

#if RSTL_PRIME1
  const T& operator*() const { return mOwner->mPtr[mIndex]; }
#else
  const T& operator*() const { return (*mOwner)[mIndex]; }
#endif
  const_linear_iterator& operator++() {
    ++mIndex;
    return *this;
  }
  const_linear_iterator& operator+=(int count) {
    mIndex += count;
    return *this;
  }
  const_linear_iterator& operator-=(int count) {
    mIndex -= count;
    return *this;
  }
#if RSTL_LINEAR_ITERATOR_DIRECT_ARITHMETIC
  const_linear_iterator operator-(int count) const {
    return const_linear_iterator(mOwner, mIndex - count);
  }
  const_linear_iterator operator+(int count) const {
    return const_linear_iterator(mOwner, mIndex + count);
  }
  long operator-(const const_linear_iterator& other) const { return mIndex - other.mIndex; }
#else
  const_linear_iterator operator+(int count) const {
    const_linear_iterator result = *this;
    result += count;
    return result;
  }
  const_linear_iterator operator-(int count) const {
    const_linear_iterator result = *this;
    result -= count;
    return result;
  }
  difference_type operator-(const const_linear_iterator& other) const {
    return mIndex - other.mIndex;
  }
#endif
  bool operator==(const const_linear_iterator& other) const {
    return mOwner == other.mOwner && mIndex == other.mIndex;
  }
  bool operator!=(const const_linear_iterator& other) const {
    return mOwner != other.mOwner || mIndex != other.mIndex;
  }

private:
  const Container* mOwner;
  int mIndex;
};
} // namespace rstl

#endif // _RSTL_LINEAR_ITERATOR
