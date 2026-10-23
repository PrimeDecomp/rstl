#ifndef _RSTL_LIST
#define _RSTL_LIST

#include "types.h"

#include "rstl/RstlVersions.h"
#include "rstl/construct.hpp"
#include "rstl/functional.hpp"
#include "rstl/iterator.hpp"
#include "rstl/rmemory_allocator.hpp"
namespace rstl {
template < typename T, typename Alloc = rmemory_allocator >
class list
#if RSTL_ALLOCATOR_BASE_CLASS
: private Alloc
#endif
{
public:
  class iterator;
  class const_iterator;

  // private:
#pragma pack(push, 1)
  struct node;
#pragma pack(pop)

public:
  list(const Alloc& alloc = Alloc())
  :
#if RSTL_ALLOCATOR_BASE_CLASS
      Alloc(alloc)
#else
      mAllocator(alloc)
#endif
  , mStart(reinterpret_cast< node* >(&mEmpty_prev))
  , mEnd(reinterpret_cast< node* >(&mEmpty_prev))
  , mEmpty_prev(reinterpret_cast< node* >(&mEmpty_prev))
  , mEmpty_next(reinterpret_cast< node* >(&mEmpty_prev))
  , mCount(0) {}

  struct destroy_helper {
    destroy_helper(list* l) : mList(l), mActive(true) {}
    ~destroy_helper() {
      if (mActive) {
        mList->destroy();
      }
    }
    void release() { mActive = false; }

  private:
    list* mList;
    bool mActive;
  };

  list(const list& other)
  :
#if RSTL_ALLOCATOR_BASE_CLASS
      Alloc(other)
#else
      mAllocator(other.mAllocator)
#endif
  , mStart(reinterpret_cast< node* >(&mEmpty_prev))
  , mEnd(reinterpret_cast< node* >(&mEmpty_prev))
  , mEmpty_prev(reinterpret_cast< node* >(&mEmpty_prev))
  , mEmpty_next(reinterpret_cast< node* >(&mEmpty_prev))
  , mCount(0) {
#if RSTL_LIST_COPY_DESTROY_GUARD
    destroy_helper dh(this);
#endif
    insert(end(), other.begin(), other.end());
#if RSTL_LIST_COPY_DESTROY_GUARD
    dh.release();
#endif
  }

  list& operator=(const list& other) {
    if (this == &other) {
      return *this;
    }
    clear();
    insert(end(), other.begin(), other.end());
    return *this;
  }

  ~list();
  node* do_erase(node* item);

  void push_front(const T& val) { do_insert_before(mStart, val); }
  void push_back(const T& val) { do_insert_before(mEnd, val); }
  void clear() { erase(begin(), end()); }

  int size() const { return mCount; }
  bool empty() const { return mCount == 0; }

  T& front() { return *mStart->get_value(); }
  T& back() { return *mEnd->get_prev()->get_value(); }
  const T& front() const { return *mStart->get_value(); }

  void pop_front() { erase(mStart); }

  iterator begin() { return iterator(mStart); }
  const_iterator begin() const { return const_iterator(mStart); }
  iterator end() { return iterator(mEnd); }
  const_iterator end() const { return const_iterator(mEnd); }

  iterator erase(const iterator& item) { return do_erase(item.get_node()); }
#if RSTL_LIST_OUT_OF_CLASS
  iterator erase(const iterator& start, const iterator& end);
#else
  iterator erase(const iterator& start, const iterator& end) {
    RS_ASSERT(start.get_node() != nullptr, "rstl precondition");
    iterator it = start;
    while (it != end) {
      it = erase(it);
    }
    return it;
  }
#endif

  struct node {
    node* mPrev;
    node* mNext;
#if RSTL_LIST_NODE_PLAIN
    uchar mItem[sizeof(T)];
#else
    ALIGNAS(T) uchar mItem[sizeof(T)];

    node(node* prev, node* next) : mPrev(prev), mNext(next) {}
#endif

    node* get_prev() const { return mPrev; }
    node* get_next() const { return mNext; }
    void set_prev(node* prev) { mPrev = prev; }
    void set_next(node* next) { mNext = next; }
    T* get_value() { return reinterpret_cast< T* >(&mItem); }
    const T* get_value() const { return reinterpret_cast< const T* >(&mItem); }
  };

  node* create_node(node* prev, node* next, const T& val) {
    node* n;
#if RSTL_ALLOCATOR_INSTANCE
    mAllocator.allocate(n, 1);
#if RSTL_LIST_PRECONDITIONS
    RSTL_PRECONDITION(n != nullptr);
#endif
    n->mPrev = prev;
    n->mNext = next;
    construct(n->get_value(), val);
#else
    Alloc::allocate(n, 1);
    new (n) node(prev, next);
    new (n->get_value()) T(val);
#endif
    return n;
  }

  node* do_insert_before(node* n, const T& val) {
#if RSTL_LIST_NODE_PLAIN
    node* const nn = create_node(n->mPrev, n, val);
#else
    node* const nn = create_node(n->get_prev(), n, val);
#endif
    if (n == mStart) {
      mStart = nn;
    }
    nn->get_prev()->set_next(nn);
    nn->get_next()->set_prev(nn);
    ++mCount;

    return nn;
  }

#if RSTL_LIST_OUT_OF_CLASS
  iterator insert(const iterator& pos, const T& val) {
    node* const result = do_insert_before(pos.get_node(), val);
    return iterator(result);
  }

  template < typename InputIterator >
  void insert(const iterator& pos, InputIterator first, InputIterator last);
#else
  iterator insert(const iterator& pos, const T& val);

  template < typename InputIterator >
  inline void insert(const iterator& pos, InputIterator first, InputIterator last);
#endif

  void destroy() {
#if RSTL_LIST_DESTROY_CALLS_CLEAR
    clear();
#else
    iterator last = end();
    iterator first = begin();
    erase(first, last);
#endif
  }

  void remove(const T& val);

#if RSTL_LIST_REMOVE_IF_IN_CLASS
  template < typename Pred >
  void remove_if(Pred pred) {
    node* it = mStart;
    while (it != mEnd) {
      if (pred(*it->get_value())) {
        it = do_erase(it);
      } else {
        it = it->get_next();
      }
    }
  }
#elif RSTL_LIST_OUT_OF_CLASS
  template < typename Pred >
  void remove_if(Pred pred);
#else
  template < typename Pred >
  void remove_if(Pred pred) {
    iterator it = begin();
    while (it != end()) {
      if (pred(*it)) {
        it = erase(it);
      } else {
        it++;
      }
    }
  }
#endif

  template < typename Cmp >
  void sort(Cmp cmp) {
    iterator it = begin();
    while (it != end()) {
      iterator min = it;
      iterator next = it;
      ++next;
      for (; next != end(); ++next) {
        if (cmp(*min, *next)) {
          min = next;
        }
      }

      next = it;
      ++next;
      exchange(it.get_node(), min.get_node());
      it = next;
    }
  }

#if !RSTL_LIST_EXCHANGE_DIRECT
  inline
#endif
  void exchange(node* nodeA, node* nodeB);

public:
  class const_iterator {
  public:
    typedef bidirectional_iterator_tag iterator_category;
    typedef int difference_type;
    typedef T* value_type;

    const_iterator() : current(nullptr) {}
    const_iterator(node* const begin) : current(begin) {}
    const_iterator& operator++() {
      this->current = this->current->mNext;
      return *this;
    }
#if RSTL_LIST_POSTFIX_ITERATORS
    const_iterator operator++(int) {
      const_iterator old = *this;
      ++*this;
      return old;
    }
#else
    const_iterator operator++(int) const { return const_iterator(this->current->mNext); }
#endif
    const_iterator& operator--() {
      this->current = this->current->mPrev;
      return *this;
    }
#if RSTL_LIST_POSTFIX_ITERATORS
    const_iterator operator--(int) {
      const_iterator old = *this;
      --*this;
      return old;
    }
#else
    const_iterator operator--(int) const { return const_iterator(this->current->mPrev); }
#endif
    const T* get_pointer() const { return current->get_value(); }
    const T& operator*() const { return *current->get_value(); }
    const T* operator->() const { return current->get_value(); }
    bool operator==(const const_iterator& other) const { return current == other.current; }
    bool operator!=(const const_iterator& other) const { return current != other.current; }

    node* get_node() const { return current; }

  protected:
    node* current;
  };

  class iterator : public const_iterator {
  public:
    typedef T* value_type;

    iterator() : const_iterator(nullptr) {}
    iterator(node* const begin) : const_iterator(begin) {}
    iterator& operator++() {
      this->current = this->current->mNext;
      return *this;
    }
    iterator operator++(int) {
      node* cur = this->current;
      this->current = this->current->mNext;
      return cur;
    }
    iterator& operator--() {
      this->current = this->current->mPrev;
      return *this;
    }
#if RSTL_LIST_POSTFIX_ITERATORS
    iterator operator--(int) {
      iterator old = *this;
      --*this;
      return old;
    }
#else
    iterator operator--(int) { return iterator(this->curent->mPrev); }
#endif
    T* get_pointer() const { return this->current->get_value(); }
    T& operator*() const { return *this->current->get_value(); }
    T* operator->() const { return this->current->get_value(); }
    bool operator==(const iterator& other) const { return this->current == other.current; }
    bool operator!=(const iterator& other) const { return this->current != other.current; }
  };

private:
#if !RSTL_ALLOCATOR_BASE_CLASS
  Alloc mAllocator;
#endif
  node* mStart;
  node* mEnd;
#if RSTL_LIST_PLAIN_SENTINEL
  node* mEmpty_prev;
#else
  ALIGNAS(node) node* mEmpty_prev;
#endif
  node* mEmpty_next;
  int mCount;
};

#if RSTL_LIST_OUT_OF_CLASS
template < typename T, typename Alloc >
template < typename InputIterator >
inline void list< T, Alloc >::insert(const iterator& pos, InputIterator first, InputIterator last) {
  for (InputIterator it = first; it != last; ++it) {
    insert(pos, *it);
  }
}

template < typename T, typename Alloc >
void list< T, Alloc >::remove(const T& val) {
  rstl::equal_to< T > equal;
  remove_if(rstl::bind1st(equal, val));
}

#if !RSTL_LIST_REMOVE_IF_IN_CLASS
template < typename T, typename Alloc >
template < typename Pred >
void list< T, Alloc >::remove_if(Pred pred) {
  node* it = mStart;
  while (it != mEnd) {
    if (pred(*it->get_value())) {
      it = do_erase(it);
    } else {
      it = it->get_next();
    }
  }
}
#endif
#else
template < typename T, typename Alloc >
inline typename list< T, Alloc >::iterator list< T, Alloc >::insert(const iterator& pos,
                                                                    const T& val) {
  node* const result = do_insert_before(pos.get_node(), val);
  return iterator(result);
}

template < typename T, typename Alloc >
template < typename InputIterator >
void list< T, Alloc >::insert(const iterator& pos, InputIterator first, InputIterator last) {
  for (InputIterator it = first; it != last; ++it) {
    insert(pos, *it);
  }
}

template < typename T, typename Alloc >
inline void list< T, Alloc >::remove(const T& val) {
  rstl::equal_to< T > equal;
  remove_if(rstl::bind1st(equal, val));
}
#endif

template < typename T, typename Alloc >
list< T, Alloc >::~list() {
  node* cur = mStart;
  while (cur != mEnd) {
    node* it = cur;
    node* next = cur->get_next();
    cur = next;
#if RSTL_LIST_DIRECT_DTOR_CALL
    it->get_value()->~T();
    mAllocator.deallocate(it);
#elif RSTL_ALLOCATOR_INSTANCE
    rstl::destroy(it->get_value());
    mAllocator.deallocate(it);
#else
    it->get_value()->~T();
    Alloc::deallocate(it);
#endif
  }
}

template < typename T, typename Alloc >
typename list< T, Alloc >::node* list< T, Alloc >::do_erase(node* node) {
#if RSTL_PRIME1
  typename list< T, Alloc >::node* const result = node->get_next();
#else
  typename list< T, Alloc >::node* result = node->get_next();
#endif
  if (node == mStart) {
#if RSTL_PRIME1
    mStart = result;
#else
    mStart = node->get_next();
#endif
  }
  node->get_prev()->set_next(node->get_next());
  node->get_next()->set_prev(node->get_prev());
#if RSTL_ALLOCATOR_INSTANCE
  rstl::destroy(node->get_value());
  mAllocator.deallocate(node);
#else
  node->get_value()->~T();
  Alloc::deallocate(node);
#endif
  mCount--;
  return result;
}

template < typename T, typename Alloc >
void list< T, Alloc >::exchange(node* nodeA, node* nodeB) {
  if (nodeA == nodeB) {
    return;
  }

  if (nodeA == this->mStart) {
    this->mStart = nodeB;
  } else if (nodeB == this->mStart) {
    this->mStart = nodeA;
  }

  node* nodeANext = nodeA->mNext;
  node* nodeAPrev = nodeA->mPrev;
  node* nodeBNext = nodeB->mNext;
  node* nodeBPrev = nodeB->mPrev;

#if RSTL_LIST_EXCHANGE_DIRECT
  if (nodeBNext != nodeA) {
    nodeBNext->mPrev = nodeA;
    nodeA->mNext = nodeBNext;
    nodeAPrev->mNext = nodeB;
    nodeB->mPrev = nodeAPrev;
  } else {
    nodeA->mNext = nodeB;
    nodeB->mPrev = nodeA;
  }

  if (nodeBPrev != nodeA) {
    nodeBPrev->mNext = nodeA;
    nodeA->mPrev = nodeBPrev;
    nodeANext->mPrev = nodeB;
    nodeB->mNext = nodeANext;
  } else {
    nodeB->mNext = nodeA;
    nodeA->mPrev = nodeB;
  }
#else
  if (nodeBNext != nodeA) {
    nodeBNext->set_prev(nodeA);
    nodeA->set_next(nodeBNext);
    nodeAPrev->set_next(nodeB);
    nodeB->set_prev(nodeAPrev);
  } else {
    nodeA->set_next(nodeB);
    nodeB->set_prev(nodeA);
  }

  if (nodeBPrev != nodeA) {
    nodeBPrev->set_next(nodeA);
    nodeA->set_prev(nodeBPrev);
    nodeANext->set_prev(nodeB);
    nodeB->set_next(nodeANext);
  } else {
    nodeB->set_next(nodeA);
    nodeA->set_prev(nodeB);
  }
#endif
}

#if RSTL_LIST_OUT_OF_CLASS
template < typename T, typename Alloc >
inline typename list< T, Alloc >::iterator list< T, Alloc >::erase(const iterator& start,
                                                                   const iterator& end) {
#if RSTL_LIST_PRECONDITIONS
  RSTL_PRECONDITION(start.get_node() != nullptr);
#endif
  iterator it = start;
  while (it != end) {
    it = erase(it);
  }
  return it;
}
#endif

} // namespace rstl

#endif // _RSTL_LIST
