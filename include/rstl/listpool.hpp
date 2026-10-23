#ifndef _RSTL_LISTPOOL
#define _RSTL_LISTPOOL

#include "rstl/RstlVersions.h"

#if RSTL_LISTPOOL_FULL
#include "rstl/construct.hpp"
#include "rstl/iterator.hpp"
#include "rstl/rmemory_allocator.hpp"

namespace rstl {

// The declaration survives in Echoes; operations are reconstructed from G2MEAB.
template < typename T, int N, typename Alloc = rmemory_allocator >
class listpool {
  struct node {
    node* mPrev;
    node* mNext;
    int mCount;
    uchar mItems[N * sizeof(T)];

    T* data() { return reinterpret_cast< T* >(mItems); }
    const T* data() const { return reinterpret_cast< const T* >(mItems); }

    T& get_value(int index) { return data()[index]; }

    void clear() {
      T* it = data();
      for (int i = 0; i < mCount; ++i) {
        destroy(it);
        ++it;
      }
      mCount = 0;
    }

    void erase(int index) {
      T* it = data() + index;
      destroy(it);
      for (int i = index; i < mCount - 1; ++i) {
        T* next = it + 1;
        construct(it, *next);
        destroy(next);
        ++it;
      }
      --mCount;
    }

    void insert(int index, const T& value) {
      int remaining = mCount - index - 1;
      T* pos = data() + index;
      T* it = pos + remaining;
      for (; remaining >= 0; --remaining) {
        construct(it + 1, *it);
        destroy(it);
        --it;
      }
      construct(pos, value);
      ++mCount;
    }
  };

public:
  class const_iterator {
  public:
    typedef bidirectional_iterator_tag iterator_category;
    typedef int difference_type;
    typedef T value_type;

    const_iterator(node* current, int index) : mCurrent(current), mIndex(index) {}
    const_iterator& operator++() {
      if (mIndex == mCurrent->mCount - 1) {
        mCurrent = mCurrent->mNext;
        mIndex = 0;
      } else {
        ++mIndex;
      }
      return *this;
    }
    const T& operator*() const { return mCurrent->get_value(mIndex); }
    const T* operator->() const { return &mCurrent->get_value(mIndex); }
    bool operator==(const const_iterator& other) const {
      return mCurrent == other.mCurrent && mIndex == other.mIndex;
    }
    bool operator!=(const const_iterator& other) const {
      return mCurrent != other.mCurrent || mIndex != other.mIndex;
    }
    node* get_node() const { return mCurrent; }
    int get_index() const { return mIndex; }

  protected:
    node* mCurrent;
    int mIndex;
  };

  class iterator : public const_iterator {
  public:
    iterator(node* current, int index) : const_iterator(current, index) {}
    iterator& operator++() {
      const_iterator::operator++();
      return *this;
    }
    T& operator*() const { return this->mCurrent->get_value(this->mIndex); }
    T* operator->() const { return &this->mCurrent->get_value(this->mIndex); }
  };

  listpool(const Alloc& alloc = Alloc())
  : mAllocator(alloc)
  , mStart(reinterpret_cast< node* >(&mEmpty_prev))
  , mEnd(reinterpret_cast< node* >(&mEmpty_prev))
  , mEmpty_prev(reinterpret_cast< node* >(&mEmpty_prev))
  , mEmpty_next(reinterpret_cast< node* >(&mEmpty_prev))
  , mEmptyCount(0)
  , mCount(0) {}

  ~listpool() {
    node* cur = mStart;
    while (cur != mEnd) {
      node* it = cur;
      cur = cur->mNext;
      it->clear();
      Alloc::deallocate(it);
    }
  }

  iterator begin() { return iterator(mStart, 0); }
  iterator end() { return iterator(mEnd, 0); }
  const_iterator begin() const { return const_iterator(mStart, 0); }
  const_iterator end() const { return const_iterator(mEnd, 0); }

  iterator erase(const iterator& it) { return do_erase(it.get_node(), it.get_index()); }

  void insert(const T& value) {
    node* cur = mStart;
    while (cur->mCount == N && cur != mEnd) {
      cur = cur->mNext;
    }
    if (cur == mEnd) {
      push_back(value);
    } else {
      // G2MEAB does not update the aggregate count in this branch.
      cur->insert(cur->mCount, value);
    }
  }

  void push_back(const T& value) { do_insert(mEnd->mPrev, mEnd->mPrev->mCount, value); }

private:
  iterator do_erase(node* cur, int index) {
    --mCount;
    cur->erase(index);
    if (cur->mCount == 0) {
      node* next = cur->mNext;
      if (cur == mStart) {
        mStart = next;
      }
      cur->mPrev->mNext = cur->mNext;
      cur->mNext->mPrev = cur->mPrev;
      Alloc::deallocate(cur);
      return iterator(next, 0);
    }
    if (index == cur->mCount) {
      return iterator(cur->mNext, 0);
    }
    return iterator(cur, index);
  }

  iterator do_insert(node* cur, int index, const T& value) {
    ++mCount;
    if (cur != mEnd && cur->mCount < N) {
      cur->insert(index, value);
      return iterator(cur, index);
    }
    if (index == 0) {
      node* nn = create_node(cur->mPrev, cur, value);
      if (cur == mStart) {
        mStart = nn;
      }
      nn->mPrev->mNext = nn;
      nn->mNext->mPrev = nn;
      return iterator(nn, 0);
    }
    if (index == N) {
      node* nn = create_node(cur, cur->mNext, value);
      nn->mNext->mPrev = nn;
      nn->mPrev->mNext = nn;
      return iterator(nn, 0);
    }
    // The native middle-insertion path moves the last item into a preceding node.
    node* nn = create_node(cur->mPrev, cur, cur->data()[cur->mCount - 1]);
    cur->erase(cur->mCount - 1);
    cur->insert(index, value);
    nn->mPrev->mNext = nn;
    nn->mNext->mPrev = nn;
    return iterator(cur, index);
  }

  node* create_node(node* prev, node* next, const T& value) {
    node* nn;
    Alloc::allocate(nn, 1);
    nn->mPrev = prev;
    nn->mNext = next;
    nn->mCount = 1;
    construct(nn->data(), value);
    return nn;
  }

  Alloc mAllocator;
  node* mStart;
  node* mEnd;
  node* mEmpty_prev;
  node* mEmpty_next;
  int mEmptyCount;
  int mCount;
};
} // namespace rstl
#else
#include "rstl/rmemory_allocator.hpp"

#include <stddef.h>

namespace rstl {
template < typename T, int N, typename Alloc = rmemory_allocator >
class listpool : private Alloc {
public:
  listpool(const Alloc& alloc = Alloc());
  ~listpool();

private:
  struct node;
  node* mStart;
  node* mEnd;
  node* mEmpty_prev;
  node* mEmpty_next;
  size_t mEmptyCount;
  size_t mCount;
};
} // namespace rstl
#endif

#endif // _RSTL_LISTPOOL
