#include "rstl/RstlVersions.h"
#include "rstl/string.hpp"
#include "rstl/math.hpp"

#if RSTL_VERSION == RSTL_G2MEAB
#include "Kyoto/Alloc/CMemory.hpp"
#endif

#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/COutputStream.hpp"

namespace rstl {

#if !RSTL_TRILOGY_ECHOES
template <>
#if RSTL_STRING_MUTABLE_NULL
char basic_string< char >::mNull = char_traits< char >::eos();
#else
const char basic_string< char >::mNull = char_traits< char >::eos();
#endif

template <>
#if RSTL_STRING_MUTABLE_NULL
wchar_t basic_string< wchar_t >::mNull = char_traits< wchar_t >::eos();
#else
const wchar_t basic_string< wchar_t >::mNull = char_traits< wchar_t >::eos();
#endif

template <>
#if RSTL_STRING_MUTABLE_NULL
char basic_string< char, case_insensitive_char_traits< char > >::mNull =
#else
const char basic_string< char, case_insensitive_char_traits< char > >::mNull =
#endif
    case_insensitive_char_traits< char >::eos();

template <>
#if RSTL_STRING_MUTABLE_NULL
// Guessed specialization: the fourth native null cell is a wchar, as in Prime.
wchar_t basic_string< wchar_t, case_insensitive_char_traits< wchar_t > >::mNull =
#else
const wchar_t basic_string< wchar_t, case_insensitive_char_traits< wchar_t > >::mNull =
#endif
    case_insensitive_char_traits< wchar_t >::eos();

#endif

// Prime owns its literal helpers in game translation units; later games define them here.
#if RSTL_STRING_OUT_OF_CLASS && !RSTL_STRINGS_PROTO_LAYOUT
string string_l(const char* data) {
  string::literal_t literal;
  return string(literal, data);
}

wstring wstring_l(const wchar_t* data) {
  wstring::literal_t literal;
  return wstring(literal, data);
}

istring istring_l(const char* data) {
  istring::literal_t literal;
  return istring(literal, data);
}

#endif
#if RSTL_STRINGS_PROTO_LAYOUT
template <>
void basic_string< char, case_insensitive_char_traits< char > >::internal_dereference() {
  if (mCow && --mCow->mRefCount == 0) {
    CMemory::Free(mCow);
  }
}

#pragma dont_inline on
template <>
void basic_string< char, case_insensitive_char_traits< char > >::internal_allocate(int size) {
  mCow = reinterpret_cast< control* >(rmemory_allocator::allocate(sizeof(control) + size));
  mPtr = reinterpret_cast< char* >(mCow + 1);
  mCow->mCapacity = size;
  mCow->mRefCount = 1;
}
#pragma dont_inline reset

template <>
basic_string< char, case_insensitive_char_traits< char > >::basic_string(
    const char* data, int count, const rmemory_allocator& alloc)
: mAllocator(alloc) {
  if (count <= 0 && !*data) {
    mPtr = &mNull;
    mSize = 0;
    mCow = 0;
    return;
  }
  const pair< const char*, int > range = compute_length(data, count);
  const int length = range.second;
  internal_allocate(length + 1);
  mSize = length;
  char_traits< char >::copy(const_cast< char* >(mPtr), data, length);
  const_cast< char& >(mPtr[length]) = 0;
}

#endif
template <>
basic_string< char >::basic_string(CInputStream& in, const rmemory_allocator& alloc)
#if RSTL_ALLOCATOR_BASE_CLASS
: rmemory_allocator(alloc), mPtr(&mNull), mCow(nullptr), mSize(0) {
#else
: mPtr(&mNull), mCow(nullptr), mSize(0), mAllocator(alloc) {
#endif
  char buffer[1025];
  int count = 0;
#if RSTL_STRING_READ_INT8
  RSTL_STRING_STREAM_CH_TYPE ch = in.ReadInt8();
#else
  char ch = in.Get< schar >();
#endif
  while (ch != '\0') {
    buffer[count++] = ch;
    if (count == 1024) {
      buffer[1024] = '\0';
      append(buffer, -1);
      count = 0;
    }
#if RSTL_STRING_READ_INT8
    ch = in.ReadInt8();
#else
    ch = in.Get< schar >();
#endif
  }
  if (count != 0) {
    buffer[count] = '\0';
    append(buffer, -1);
  }
}

template <>
basic_string< char >::basic_string(const char* data, int count, const rmemory_allocator& alloc)
#if RSTL_ALLOCATOR_BASE_CLASS
: rmemory_allocator(alloc) {
#else
: mAllocator(alloc) {
#endif
  if (count <= 0 && !*data) {
    mPtr = &mNull;
    mSize = 0;
    mCow = nullptr;
    return;
  }

  const pair< const char*, int > range = compute_length(data, count);
  const int len = range.second;
  internal_allocate(len + 1);
  mSize = len;
  char_traits< char >::copy(const_cast< char* >(mPtr), data, len);
  char_traits< char >::assign(const_cast< char& >(mPtr[len]), char_traits< char >::eos());
}

template <>
basic_string< char >::basic_string(const basic_string& other)
#if RSTL_ALLOCATOR_BASE_CLASS
: rmemory_allocator(other)
, mPtr(other.mPtr)
#else
: mPtr(other.mPtr)
#endif
, mCow(other.mCow)
, mSize(other.mSize)
#if !RSTL_ALLOCATOR_BASE_CLASS
, mAllocator(other.mAllocator)
#endif
{
  internal_reference();
}

template <>
#if RSTL_STRING_CHAR_CTOR_ALLOCATOR
basic_string< char >::basic_string(char value, const rmemory_allocator& alloc)
: mAllocator(alloc) {
#else
basic_string< char >::basic_string(char value) {
#endif
  internal_allocate(2);
  const_cast< char& >(mPtr[0]) = value;
  const_cast< char& >(mPtr[1]) = 0;
  mSize = 1;
}

template <>
void basic_string< char >::clear() {
  internal_dereference();
  mSize = 0;
  mPtr = &mNull;
  mCow = 0;
}

template <>
basic_string< char >& basic_string< char >::erase(int pos, int count) {
  const int start = pos > length() ? length() : pos;
  const int removed = min_val(count == -1 ? length() : count, length() - start);

  internal_prepare_to_write(length(), true);
  char_traits< char >::copy(const_cast< char* >(mPtr + start), mPtr + start + removed,
                            length() - start + 1 - removed);
  mSize -= removed;
  return *this;
}

template <>
basic_string< char >& basic_string< char >::append(const basic_string& other) {
  internal_prepare_to_write(length() + other.length(), true);
  char_traits< char >::copy(const_cast< char* >(mPtr) + length(), other.data(), other.length());
  mSize += other.length();
  char_traits< char >::assign(const_cast< char& >(mPtr[length()]), char_traits< char >::eos());
  return *this;
}

template <>
basic_string< char >& basic_string< char >::append(const char* data, int count) {
  const pair< const char*, int > range = compute_length(data, count);
  const int len = range.second;
  internal_prepare_to_write(mSize + len, true);
  char_traits< char >::copy(const_cast< char* >(mPtr) + length(), data, len);
  mSize += len;
  char_traits< char >::assign(const_cast< char& >(mPtr[length()]), char_traits< char >::eos());
  return *this;
}

template <>
basic_string< char >& basic_string< char >::append(int count, char value) {
  internal_prepare_to_write(mSize + count, true);
  char_traits< char >::assign(const_cast< char* >(mPtr) + length(), count, value);
  mSize += count;
  char_traits< char >::assign(const_cast< char& >(mPtr[length()]), char_traits< char >::eos());
  return *this;
}

template <>
basic_string< char >& basic_string< char >::assign(const basic_string& other) {
  if (mCow && mCow == other.mCow) {
    return *this;
  }

  internal_dereference();
  mCow = other.mCow;
  mPtr = other.mPtr;
  mSize = other.mSize;
  internal_reference();
  return *this;
}

#if RSTL_STRING_ASSIGN_SPECIALIZATION
template <>
basic_string< char >& basic_string< char >::assign(const char* data, int count) {
  const pair< const char*, int > range = compute_length(data, count);
  const int len = range.second;
  internal_prepare_to_write(len, false);
  char_traits< char >::copy(const_cast< char* >(mPtr), data, len);
  mSize = len;
  char_traits< char >::assign(const_cast< char& >(mPtr[length()]), char_traits< char >::eos());
  return *this;
}
#endif

template <>
void basic_string< char >::PutTo(COutputStream& out) const {
  for (int i = 0; i < length() + 1; ++i) {
    out.WriteChar(mPtr[i]);
  }
}

#if RSTL_STRINGS_PROTO_LAYOUT
template <>
pair< basic_string< char >::const_iterator, basic_string< char >::const_iterator >
basic_string< char >::range_iterator(int pos, int count) const {
  const_iterator first = position_iterator(pos);
  const_iterator last = count != -1 && pos + count < length() ? first + count : end();
  return pair< const_iterator, const_iterator >(first, last);
}

#endif
template <>
basic_string< char >::const_iterator basic_string< char >::position_iterator(int pos) const {
  if (pos == -1 || pos >= length()) {
    return end();
  }

  return begin() + pos;
}

#if !RSTL_STRINGS_PROTO_LAYOUT
template <>
pair< basic_string< char >::const_iterator, basic_string< char >::const_iterator >
basic_string< char >::range_iterator(int pos, int count) const {
  const_iterator first = position_iterator(pos);
  const_iterator last = count != -1 && pos + count < length() ? first + count : end();
  return pair< const_iterator, const_iterator >(first, last);
}
#endif

template <>
void basic_string< char >::internal_allocate(int size) {
  rmemory_allocator::allocate(reinterpret_cast< uchar*& >(mCow),
                              sizeof(control) + sizeof(char) * size);
  mPtr = reinterpret_cast< char* >(mCow + 1);
  mCow->mCapacity = size;
  mCow->mRefCount = 1;
}

template <>
void basic_string< char >::internal_dereference() {
  if (mCow && --mCow->mRefCount == 0) {
#if RSTL_STRING_FREE_WITH_CMEMORY
    CMemory::Free(mCow);
#else
    rmemory_allocator::deallocate(mCow);
#endif
  }
}

template <>
void basic_string< char >::internal_prepare_to_write(int len, bool preserve) {
  const int required = len + 1;
  if (mCow == nullptr || mCow->mRefCount != 1 || mCow->mCapacity < required) {
    int capacity;
    if (mCow) {
      capacity = mCow->mCapacity < 4 ? 4 : mCow->mCapacity;
      while (capacity < required) {
        capacity *= 2;
      }
    } else {
      capacity = required;
    }

    uchar* allocation;
    rmemory_allocator::allocate(allocation, sizeof(control) + sizeof(char) * capacity);
    control* newControl = reinterpret_cast< control* >(allocation);
    newControl->mCapacity = capacity;
    newControl->mRefCount = 1;
    char* const newData = reinterpret_cast< char* >(newControl + 1);
    if (preserve) {
      char_traits< char >::copy(newData, mPtr, length());
      char_traits< char >::assign(newData[length()], char_traits< char >::eos());
    }
    internal_dereference();
    mCow = newControl;
    mPtr = newData;
  }
}

template <>
basic_string< wchar_t >::basic_string(const wchar_t* data, int count,
                                      const rmemory_allocator& alloc)
#if RSTL_ALLOCATOR_BASE_CLASS
: rmemory_allocator(alloc) {
#else
: mAllocator(alloc) {
#endif
  if (count <= 0 && !*data) {
    mPtr = &mNull;
    mSize = 0;
    mCow = nullptr;
    return;
  }

  const pair< const wchar_t*, int > range = compute_length(data, count);
  const int len = range.second;
  internal_allocate(len + 1);
  mSize = len;
  char_traits< wchar_t >::copy(const_cast< wchar_t* >(mPtr), data, len);
  char_traits< wchar_t >::assign(const_cast< wchar_t& >(mPtr[len]),
                                 char_traits< wchar_t >::eos());
}

template <>
basic_string< wchar_t >::basic_string(const basic_string& other)
#if RSTL_ALLOCATOR_BASE_CLASS
: rmemory_allocator(other)
, mPtr(other.mPtr)
#else
: mPtr(other.mPtr)
#endif
, mCow(other.mCow)
, mSize(other.mSize)
#if !RSTL_ALLOCATOR_BASE_CLASS
, mAllocator(other.mAllocator)
#endif
{
  internal_reference();
}

template <>
basic_string< wchar_t >& basic_string< wchar_t >::append(const basic_string& other) {
  internal_prepare_to_write(length() + other.length(), true);
  char_traits< wchar_t >::copy(const_cast< wchar_t* >(mPtr) + length(), other.data(),
                               other.length());
  mSize += other.length();
  char_traits< wchar_t >::assign(const_cast< wchar_t& >(mPtr[length()]),
                                 char_traits< wchar_t >::eos());
  return *this;
}

template <>
basic_string< wchar_t >& basic_string< wchar_t >::append(const wchar_t* data, int count) {
  const pair< const wchar_t*, int > range = compute_length(data, count);
  const int len = range.second;
  internal_prepare_to_write(mSize + len, true);
  char_traits< wchar_t >::copy(const_cast< wchar_t* >(mPtr) + length(), data, len);
  mSize += len;
  char_traits< wchar_t >::assign(const_cast< wchar_t& >(mPtr[length()]),
                                 char_traits< wchar_t >::eos());
  return *this;
}

template <>
basic_string< wchar_t >& basic_string< wchar_t >::append(int count, wchar_t value) {
  internal_prepare_to_write(mSize + count, true);
  char_traits< wchar_t >::assign(const_cast< wchar_t* >(mPtr) + length(), count, value);
  mSize += count;
  char_traits< wchar_t >::assign(const_cast< wchar_t& >(mPtr[length()]),
                                 char_traits< wchar_t >::eos());
  return *this;
}

template <>
basic_string< wchar_t >& basic_string< wchar_t >::assign(const basic_string& other) {
  if (mCow && mCow == other.mCow) {
    return *this;
  }

  internal_dereference();
  mCow = other.mCow;
  mPtr = other.mPtr;
  mSize = other.mSize;
  internal_reference();
  return *this;
}

template <>
basic_string< wchar_t >& basic_string< wchar_t >::assign(const wchar_t* data, int count) {
  const pair< const wchar_t*, int > range = compute_length(data, count);
  const int len = range.second;
  internal_prepare_to_write(len, false);
  char_traits< wchar_t >::copy(const_cast< wchar_t* >(mPtr), data, len);
  mSize = len;
  char_traits< wchar_t >::assign(const_cast< wchar_t& >(mPtr[length()]),
                                 char_traits< wchar_t >::eos());
  return *this;
}

template <>
basic_string< wchar_t >::const_iterator basic_string< wchar_t >::position_iterator(int pos) const {
  if (pos == -1 || pos >= length()) {
    return end();
  }

  return begin() + pos;
}

template <>
pair< basic_string< wchar_t >::const_iterator, basic_string< wchar_t >::const_iterator >
basic_string< wchar_t >::range_iterator(int pos, int count) const {
  const_iterator first = position_iterator(pos);
  const_iterator last = count != -1 && pos + count < length() ? first + count : end();
  return pair< const_iterator, const_iterator >(first, last);
}

template <>
void basic_string< wchar_t >::internal_allocate(int size) {
  rmemory_allocator::allocate(reinterpret_cast< uchar*& >(mCow),
                              sizeof(control) + sizeof(wchar_t) * size);
  mPtr = reinterpret_cast< wchar_t* >(mCow + 1);
  mCow->mCapacity = size;
  mCow->mRefCount = 1;
}

template <>
void basic_string< wchar_t >::internal_dereference() {
  if (mCow && --mCow->mRefCount == 0) {
#if RSTL_STRING_FREE_WITH_CMEMORY
    CMemory::Free(mCow);
#else
    rmemory_allocator::deallocate(mCow);
#endif
  }
}

template <>
void basic_string< wchar_t >::internal_prepare_to_write(int len, bool preserve) {
  const int required = len + 1;
  if (mCow == nullptr || mCow->mRefCount != 1 || mCow->mCapacity < required) {
    int capacity;
    if (mCow) {
      capacity = mCow->mCapacity < 4 ? 4 : mCow->mCapacity;
      while (capacity < required) {
        capacity *= 2;
      }
    } else {
      capacity = required;
    }

    uchar* allocation;
    rmemory_allocator::allocate(allocation, sizeof(control) + sizeof(wchar_t) * capacity);
    control* newControl = reinterpret_cast< control* >(allocation);
    newControl->mCapacity = capacity;
    newControl->mRefCount = 1;
    wchar_t* const newData = reinterpret_cast< wchar_t* >(newControl + 1);
    if (preserve) {
      char_traits< wchar_t >::copy(newData, mPtr, length());
      char_traits< wchar_t >::assign(newData[length()], char_traits< wchar_t >::eos());
    }
    internal_dereference();
    mCow = newControl;
    mPtr = newData;
  }
}

#if RSTL_STRINGS_PROTO_LAYOUT
istring istring_l(const char* data) {
  istring::literal_t literal;
  return istring(literal, data);
}

wstring wstring_l(const wchar_t* data) {
  wstring::literal_t literal;
  return wstring(literal, data);
}

string string_l(const char* data) {
  string::literal_t literal;
  return string(literal, data);
}
#else
template <>
basic_string< char, case_insensitive_char_traits< char > >::basic_string(
    const char* data, int count, const rmemory_allocator& alloc)
#if RSTL_ALLOCATOR_BASE_CLASS
: rmemory_allocator(alloc) {
#else
: mAllocator(alloc) {
#endif
  if (count <= 0 && !*data) {
    mPtr = &mNull;
    mSize = 0;
    mCow = nullptr;
    return;
  }

  const pair< const char*, int > range = compute_length(data, count);
  const int len = range.second;
  internal_allocate(len + 1);
  mSize = len;
  case_insensitive_char_traits< char >::copy(const_cast< char* >(mPtr), data, len);
  case_insensitive_char_traits< char >::assign(const_cast< char& >(mPtr[len]),
                                               case_insensitive_char_traits< char >::eos());
}

#if RSTL_STRING_ASSIGN_SPECIALIZATION
// This specialization also places compute_length ahead of the constructor under
// deferred inlining, preserving Prime's PAL/Japanese layout.
template <>
basic_string< char, case_insensitive_char_traits< char > >&
basic_string< char, case_insensitive_char_traits< char > >::assign(const char* data, int count) {
  const pair< const char*, int > range = compute_length(data, count);
  const int len = range.second;
  internal_prepare_to_write(len, false);
  case_insensitive_char_traits< char >::copy(const_cast< char* >(mPtr), data, len);
  mSize = len;
  case_insensitive_char_traits< char >::assign(const_cast< char& >(mPtr[length()]),
                                               case_insensitive_char_traits< char >::eos());
  return *this;
}
#endif

template <>
void basic_string< char, case_insensitive_char_traits< char > >::internal_allocate(int size) {
  rmemory_allocator::allocate(reinterpret_cast< uchar*& >(mCow),
                              sizeof(control) + sizeof(char) * size);
  mPtr = reinterpret_cast< char* >(mCow + 1);
  mCow->mCapacity = size;
  mCow->mRefCount = 1;
}

template <>
void basic_string< char, case_insensitive_char_traits< char > >::internal_dereference() {
  if (mCow && --mCow->mRefCount == 0) {
#if RSTL_STRING_FREE_WITH_CMEMORY
    CMemory::Free(mCow);
#else
    rmemory_allocator::deallocate(mCow);
#endif
  }
}
#endif

} // namespace rstl
