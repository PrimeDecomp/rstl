#ifndef _RSTL_BIT_VECTOR
#define _RSTL_BIT_VECTOR

#include "rstl/RstlVersions.h"
#include "rstl/vector.hpp"

namespace rstl {
template < typename Alloc = rmemory_allocator >
class bit_vector {
  typedef vector< uint, Alloc > storage_type;

public:
  typedef Alloc allocator_type;

#if RSTL_BIT_VECTOR_COMPACT
  // G2MEAB's `at()` reads the word storage at +0x10 (mSize, then the vector) and builds a
  // word/mask reference; `operator[]` forwards to it out of line.
  class reference {
  public:
    reference(typename storage_type::iterator word, int bit) : mWord(word), mMask(1 << bit) {}
    operator bool() const { return (mMask & *mWord.get_pointer()) != 0; }
    reference& operator=(bool value) {
      if (value) {
        *mWord |= mMask;
      } else {
        *mWord &= ~mMask;
      }
      return *this;
    }

  private:
    typename storage_type::iterator mWord;
    uint mMask;
  };

  bit_vector() : mSize(0) {}
  bit_vector(int count, bool value);

  int size() const { return mSize; }
  reference at(int bit);
  reference operator[](int bit);

private:
  int get_real_index(int bit) { return bit / 32; }
#else
  class bit_reference {
  public:
    bit_reference(typename storage_type::iterator word, int bit) : mWord(word), mMask(1 << bit) {}
    operator bool() const { return (mMask & *mWord.get_pointer()) != 0; }
    bit_reference& operator=(bool value) {
      if (value) {
        *mWord |= mMask;
      } else {
        *mWord &= ~mMask;
      }
      return *this;
    }

  private:
    typename storage_type::iterator mWord;
    int mMask;
  };

  class const_iterator {
  public:
    const_iterator(typename storage_type::const_iterator data, int bit, int)
    : mData(data), mCurrentBit(bit) {}
    int get_current_bit() const { return mCurrentBit; }

  protected:
    typename storage_type::const_iterator mData;
    int mCurrentBit;
  };

  class iterator : public const_iterator {
  public:
    iterator(typename storage_type::iterator data, int bit, int offset)
    : const_iterator(data, bit, offset), mData(data) {}

  private:
    typename storage_type::iterator mData;
  };

  bit_vector() : mSize(0) {}
#if RSTL_BIT_VECTOR_OUT_OF_CLASS
  bit_vector(int count, bool value);
#else
  bit_vector(int count, bool value) : mSize(0) {
    reserve(count);
    insert(begin(), count, value);
  }
#endif

  bit_vector& operator=(const bit_vector& other) {
    mSize = other.mSize;
    mData = other.mData;
    return *this;
  }

  int size() const { return mSize; }
#if RSTL_BIT_VECTOR_OUT_OF_CLASS
  iterator begin();
  inline iterator end();
  void reserve(int count) { mData.reserve(get_data_size(count)); }
  bit_reference at(int bit);
  bit_reference operator[](int bit);
  inline void push_back(bool value);
  void insert(iterator at, int count, bool value);
#else
  iterator begin() { return iterator(mData.begin(), 0, 0); }
  iterator end() { return iterator(mData.begin(), mSize, 0); }
  void reserve(int count) { mData.reserve(get_data_size(count)); }
  bit_reference at(int bit) {
    return bit_reference(mData.begin() + get_real_index(bit), bit % 32);
  }
  bit_reference operator[](int bit) { return at(bit); }
  void push_back(bool value) {
    if (mSize % 32 == 0) {
      mData.push_back(0u);
    }
    set_bit(mSize++, value);
  }
  void insert(iterator at, int count, bool value) {
    int bit = at.get_current_bit();
    make_room(bit, count);
#if RSTL_COUNTED_FILL_LOOPS
    for (int i = 0; i < count; ++i, ++bit) {
      set_bit(bit, value);
    }
#else
    while (count-- > 0) {
      set_bit(bit++, value);
    }
#endif
  }
#endif

  bool get_bit(int bit) { return (mData[get_real_index(bit)] & get_real_bit_mask(bit)) != 0; }
  void set_bit(int bit, bool value) {
    if (value) {
      mData[get_real_index(bit)] |= get_real_bit_mask(bit);
    } else {
      mData[get_real_index(bit)] &= ~get_real_bit_mask(bit);
    }
  }
  int get_real_index(int bit) { return bit / 32; }
  uint get_real_bit_mask(int bit) { return 1 << (bit % 32); }
  int get_data_size(int count) { return count / 32 + (count % 32 ? 1 : 0); }
#if RSTL_BIT_VECTOR_OUT_OF_CLASS
  void make_room(int bit, int count);
#else
  void make_room(int bit, int count) {
    int available = mData.size() * 32 - mSize;
#if RSTL_BIT_VECTOR_UNSAFE_GROW
    // Insertion requires reserve(size() + count) before making room.
    uint zero = 0;
    while (available < count) {
      mData.push_back_unsafe(zero);
      available += 32;
    }
#else
    while (available < count) {
      mData.push_back(0u);
      available += 32;
    }
#endif
    for (int i = mSize - 1; i >= bit; --i) {
      set_bit(i + count, get_bit(i));
    }
    mSize += count;
  }
#endif

  void PutTo(COutputStream& out) const { mData.PutTo(out); }

private:
#endif
  int mSize;
  storage_type mData;
};

#if RSTL_BIT_VECTOR_COMPACT
template < typename Alloc >
typename bit_vector< Alloc >::reference bit_vector< Alloc >::at(int bit) {
  return reference(mData.begin() + get_real_index(bit), bit % 32);
}

template < typename Alloc >
typename bit_vector< Alloc >::reference bit_vector< Alloc >::operator[](int bit) {
  return at(bit);
}
#elif RSTL_BIT_VECTOR_OUT_OF_CLASS
template < typename Alloc >
bit_vector< Alloc >::bit_vector(int count, bool value) : mSize(0) {
  reserve(count);
  insert(begin(), count, value);
}

template < typename Alloc >
typename bit_vector< Alloc >::iterator bit_vector< Alloc >::begin() {
  return iterator(mData.begin(), 0, 0);
}

template < typename Alloc >
typename bit_vector< Alloc >::iterator bit_vector< Alloc >::end() {
  RSTL_PRECONDITION(mSize >= 0);
  return iterator(mData.begin(), mSize, 0);
}

template < typename Alloc >
typename bit_vector< Alloc >::bit_reference bit_vector< Alloc >::at(int bit) {
  return bit_reference(mData.begin() + get_real_index(bit), bit % 32);
}

template < typename Alloc >
typename bit_vector< Alloc >::bit_reference bit_vector< Alloc >::operator[](int bit) {
  return at(bit);
}

template < typename Alloc >
void bit_vector< Alloc >::push_back(bool value) {
  if (mSize % 32 == 0) {
    mData.push_back_unsafe(0u);
  }
  set_bit(mSize++, value);
}

template < typename Alloc >
void bit_vector< Alloc >::insert(iterator at, int count, bool value) {
  int bit = at.get_current_bit();
  make_room(bit, count);
  while (count-- > 0) {
    set_bit(bit++, value);
  }
}

template < typename Alloc >
void bit_vector< Alloc >::make_room(int bit, int count) {
  int available = mData.size() * 32 - mSize;
  if (available < count) {
    uint words = static_cast< uint >(count + 31 - available) >> 5;
    while (words--) {
      mData.push_back_unsafe(0u);
    }
  }
  for (int i = mSize - 1; i >= bit; --i) {
    set_bit(i + count, get_bit(i));
  }
  mSize += count;
}
#endif
} // namespace rstl
#endif // _RSTL_BIT_VECTOR
