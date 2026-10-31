#ifndef _RSTL_STRING
#define _RSTL_STRING

#include "rstl/RstlVersions.h"
#include "types.h"

#include "rstl/linear_iterator.hpp"
#include "rstl/pair.hpp"
#include "rstl/rmemory_allocator.hpp"

class CInputStream;
class COutputStream;

namespace rstl {
template < typename _CharTp >
struct char_traits {
  static void copy(_CharTp* out, const _CharTp* in, int count) {
    for (int i = 0; i < count; ++i) {
      out[i] = in[i];
    }
  }

  static void assign(_CharTp& out, const _CharTp& value) { out = value; }

#if RSTL_STRING_TRAITS_VALUE_PARAM
  static void assign(_CharTp* out, int count, _CharTp value) {
#else
  static void assign(_CharTp* out, int count, const _CharTp& value) {
#endif
    for (int i = 0; i < count; ++i) {
      out[i] = value;
    }
  }

  static bool eq(const _CharTp& lhs, const _CharTp& rhs) { return lhs == rhs; }
  static _CharTp eos() { return 0; }
  static int compare(const _CharTp& lhs, const _CharTp& rhs) {
    return static_cast< int >(lhs) - static_cast< int >(rhs);
  }
};

template <>
struct char_traits< char > {
  static void copy(char* out, const char* in, int count) {
    for (int i = 0; i < count; ++i) {
      out[i] = in[i];
    }
  }

  static void assign(char& out, const char& value) { out = value; }

#if RSTL_STRING_TRAITS_VALUE_PARAM
  static void assign(char* out, int count, char value) {
#else
  static void assign(char* out, int count, const char& value) {
#endif
    for (int i = 0; i < count; ++i) {
      out[i] = value;
    }
  }

  static bool eq(const char& lhs, const char& rhs) { return lhs == rhs; }
  static char eos() { return 0; }
  static int compare(const char& lhs, const char& rhs) {
    return static_cast< int >(static_cast< signed char >(lhs)) -
           static_cast< int >(static_cast< signed char >(rhs));
  }
};

template < typename _CharTp >
struct case_insensitive_char_traits {
  static void copy(_CharTp* out, const _CharTp* in, int count) {
    for (int i = 0; i < count; ++i) {
      out[i] = in[i];
    }
  }

  static void assign(_CharTp& out, const _CharTp& value) { out = value; }

#if RSTL_STRING_TRAITS_VALUE_PARAM
  static void assign(_CharTp* out, int count, _CharTp value) {
#else
  static void assign(_CharTp* out, int count, const _CharTp& value) {
#endif
    for (int i = 0; i < count; ++i) {
      out[i] = value;
    }
  }

  static _CharTp eos() { return 0; }
  static _CharTp toupper(const _CharTp ch) {
    return (ch >= 'a' && ch <= 'z')         ? ch - 32
           : (ch >= 0xe0 && ch <= 0xfe)     ? ch - 32
           : (ch >= 0x30a0 && ch <= 0x30ff) ? ch - 96
                                            : ch;
  }

  static bool eq(const _CharTp& lhs, const _CharTp& rhs) { return toupper(lhs) == toupper(rhs); }

  static int compare(const _CharTp& lhs, const _CharTp& rhs) { return toupper(rhs) - toupper(lhs); }
};

template < typename It >
class reverse_iterator {
public:
  typedef typename iterator_traits< It >::value_type value_type;

  explicit reverse_iterator(It it) : mIt(it - 1) {}

  const value_type& operator*() const { return *mIt; }
  reverse_iterator& operator++() {
    mIt -= 1;
    return *this;
  }
  bool operator==(const reverse_iterator& other) const { return mIt == other.mIt; }
  bool operator!=(const reverse_iterator& other) const { return !(mIt == other.mIt); }

private:
  It mIt;
};

template < typename _CharTp, typename Traits = char_traits< _CharTp >,
           typename Alloc = rmemory_allocator >
class basic_string
#if RSTL_ALLOCATOR_BASE_CLASS
: private Alloc
#endif
{
  struct control {
    int mCapacity;
    int mRefCount;
  };

  const _CharTp* mPtr;
  control* mCow;
  uint mSize;
#if !RSTL_ALLOCATOR_BASE_CLASS
  Alloc mAllocator;
#endif

  void internal_prepare_to_write(int len, bool);
  void internal_allocate(int size);

  void internal_dereference();
  void internal_reference() {
    if (mCow) {
      ++mCow->mRefCount;
    }
  }

  template < typename It >
  static pair< It, int > compute_length(It data, int count) {
    It end = data;
    int len = 0;
    while ((count == -1 || len < count) && !Traits::eq(*end, Traits::eos())) {
      ++end;
      ++len;
    }
    return pair< It, int >(end, len);
  }

#if RSTL_STRING_MUTABLE_NULL
  static _CharTp mNull;
#elif RSTL_STRING_INCLASS_NULL
  static const _CharTp mNull = 0;
#else
  static const _CharTp mNull;
#endif

public:
  typedef const_linear_iterator< _CharTp, basic_string, Alloc > const_iterator;
  friend class const_linear_iterator< _CharTp, basic_string, Alloc >;

  struct literal_t {};

  basic_string() : mPtr(&mNull), mCow(nullptr), mSize(0) {}

  basic_string(literal_t, const _CharTp* data) {
    mPtr = data;
    mCow = nullptr;
#if RSTL_STRING_SIMPLE_LITERAL_CTOR
    const _CharTp* end = data;
    while (*end) {
      ++end;
    }
    mSize = end - data;
#else

    const _CharTp* it = data;
    while (*it)
      ++it;

    mSize = static_cast< uint >(it - data);
#endif
  }

#if RSTL_STRING_CHAR_CTOR_ALLOCATOR
  basic_string(_CharTp value, const Alloc& = rmemory_allocator());
#else
  basic_string(_CharTp value);
#endif

  basic_string(const basic_string& str);

  basic_string(CInputStream& in, const Alloc& = rmemory_allocator());

  template < typename It >
  basic_string(It first, It last, const Alloc& = rmemory_allocator()) {
    const int len = rstl::distance(first, last);
    internal_allocate(len + 1);
    int i = 0;
#if RSTL_PRIME1
    for (It it = first; it != last; ++it) {
      Traits::assign(const_cast< _CharTp& >(mPtr[i++]), *it);
    }
    Traits::assign(const_cast< _CharTp& >(mPtr[i]), Traits::eos());
#else
    for (It it = first; it != last; it = it + 1, ++i) {
      const_cast< _CharTp& >(mPtr[i]) = *it;
    }
    const_cast< _CharTp& >(mPtr[i]) = RSTL_STRING_RANGE_CTOR_EOS;
#endif
    mSize = len;
  }

  basic_string(const _CharTp* data, int size = -1, const Alloc& = rmemory_allocator());

#if RSTL_STRING_DTOR_PRECONDITION
  ~basic_string() {
    RSTL_PRECONDITION(mPtr != nullptr);
    internal_dereference();
  }
#else
  ~basic_string() { internal_dereference(); }
#endif

  size_t size() const { return mSize; }
  int length() const { return mSize; }
  bool empty() const { return length() == 0; }
  int refcount() { return mCow != nullptr ? mCow->mRefCount : -1; }
  void reserve(int len) { internal_prepare_to_write(len, true); }

  basic_string& assign(const basic_string&);
  basic_string& assign(const _CharTp*, int = -1);
  basic_string& operator=(const basic_string& other) {
    assign(other);
    return *this;
  }
  basic_string& operator+=(const _CharTp* other) {
    append(other, -1);
    return *this;
  }

  basic_string& append(const basic_string& other);
  basic_string& append(int, _CharTp);
  basic_string& append(const _CharTp*, int = -1);
  basic_string& erase(int pos, int count);
  void clear();

  int compare(const _CharTp* rhs, int count = -1) const;
  const _CharTp& operator[](int idx) const { return mPtr[idx]; }
#if RSTL_STRING_WRITABLE_INDEX
  _CharTp& operator[](int idx) {
    // Unshares the buffer before handing out a writable character.
    internal_prepare_to_write(mSize, true);
    return const_cast< _CharTp& >(mPtr[idx]);
  }
#endif
  const_iterator begin() const { return const_iterator(this, 0); }
  const_iterator end() const { return const_iterator(this, size()); }

  template < typename It >
  static int internal_compare(const_iterator first, const_iterator last, It otherFirst,
                              It otherLast);
  template < typename It, typename OtherIt >
#if RSTL_STRING_INLINE_SEARCH_DECL
  static inline int internal_search(It first, It last, OtherIt otherFirst, OtherIt otherLast);
#else
  static int internal_search(It first, It last, OtherIt otherFirst, OtherIt otherLast);
#endif
  template < typename It, typename OtherIt >
  static int internal_search_not_of(It first, It last, OtherIt otherFirst, OtherIt otherLast);
  template < typename It, typename OtherIt >
  static int internal_search_of(It first, It last, OtherIt otherFirst, OtherIt otherLast);
  int compare(const basic_string& other) const;
  bool operator==(const basic_string& other) const;
  bool operator!=(const basic_string& other) const;

  int find(const basic_string& other, int pos = 0) const;
  inline int find(const _CharTp* other, int pos = 0, int count = -1) const;
  inline int find(_CharTp ch, int pos = 0) const;
#if RSTL_STRING_MATCH_LOOP_SEARCH
  int rfind(const _CharTp* other, int pos = -1, int count = -1) const;
  int rfind(_CharTp ch, int pos = -1) const;
#endif
  int find_first_of(const basic_string& other, int pos = 0) const;
  int find_first_of(const _CharTp* other, int pos = 0, int count = -1) const;
  int find_first_not_of(const basic_string& other, int pos = 0) const;
  int find_last_not_of(const basic_string& other, int pos = -1) const;
  const_iterator position_iterator(int pos) const;
  pair< const_iterator, const_iterator > range_iterator(int pos, int count) const;
  basic_string substr(int pos = 0, int count = -1) const;
  int get_real_pos_for_begin(int pos) const {
    if (pos == -1 || pos >= static_cast< int >(size())) {
      return size();
    }
    return pos;
  }
  int get_real_pos_for_end(int pos) const {
    if (pos == -1 || pos >= static_cast< int >(size())) {
      return size() - 1;
    }
    return pos;
  }
  const _CharTp* c_str() const { return mPtr; }
  const _CharTp* data() const { return mPtr; }
  void PutTo(COutputStream& out) const;
  const _CharTp at(int idx) const { return data()[idx]; }
};

#if RSTL_STRING_INCLASS_NULL
template < typename _CharTp, typename Traits, typename Alloc >
const _CharTp basic_string< _CharTp, Traits, Alloc >::mNull;
#elif !defined(__MWERKS__)

#if RSTL_STRING_MUTABLE_NULL
template <>
char basic_string< char >::mNull;
template <>
wchar_t basic_string< wchar_t >::mNull;
template <>
char basic_string< char, case_insensitive_char_traits< char > >::mNull;
template <>
wchar_t basic_string< wchar_t, case_insensitive_char_traits< wchar_t > >::mNull;

#else
template <>
const char basic_string< char >::mNull;
template <>
const wchar_t basic_string< wchar_t >::mNull;
template <>
const char basic_string< char, case_insensitive_char_traits< char > >::mNull;
template <>
const wchar_t basic_string< wchar_t, case_insensitive_char_traits< wchar_t > >::mNull;

#endif

// Keep modern compilers from implicitly instantiating members whose explicit
// specializations are defined in rstl_strings.cpp or the game.
// basic_string<char>
template <>
basic_string< char >::basic_string(CInputStream& in, const rmemory_allocator& alloc);
template <>
basic_string< char >::basic_string(const char* data, int count, const rmemory_allocator& alloc);
template <>
basic_string< char >::basic_string(const basic_string& other);
template <>
#if RSTL_STRING_CHAR_CTOR_ALLOCATOR
basic_string< char >::basic_string(char value, const rmemory_allocator& alloc);
#else
basic_string< char >::basic_string(char value);
#endif
template <>
void basic_string< char >::clear();
template <>
basic_string< char >& basic_string< char >::erase(int pos, int count);

template <>
basic_string< char >& basic_string< char >::append(const basic_string& other);
template <>
basic_string< char >& basic_string< char >::append(const char* data, int count);
template <>
basic_string< char >& basic_string< char >::append(int count, char value);
template <>
basic_string< char >& basic_string< char >::assign(const basic_string& other);

template <>
void basic_string< char >::PutTo(COutputStream& out) const;
template <>
basic_string< char >::const_iterator basic_string< char >::position_iterator(int pos) const;
template <>
pair< basic_string< char >::const_iterator, basic_string< char >::const_iterator >
basic_string< char >::range_iterator(int pos, int count) const;

template <>
void basic_string< char >::internal_allocate(int size);
template <>
void basic_string< char >::internal_dereference();
template <>
void basic_string< char >::internal_prepare_to_write(int len, bool preserve);

// basic_string<wchar_t>
template <>
basic_string< wchar_t >::basic_string(const wchar_t* data, int count,
                                      const rmemory_allocator& alloc);
template <>
basic_string< wchar_t >::basic_string(const basic_string& other);

template <>
basic_string< wchar_t >& basic_string< wchar_t >::append(const basic_string& other);
template <>
basic_string< wchar_t >& basic_string< wchar_t >::append(const wchar_t* data, int count);
template <>
basic_string< wchar_t >& basic_string< wchar_t >::append(int count, wchar_t value);
template <>
basic_string< wchar_t >& basic_string< wchar_t >::assign(const basic_string& other);
template <>
basic_string< wchar_t >& basic_string< wchar_t >::assign(const wchar_t* data, int count);

template <>
void basic_string< wchar_t >::internal_allocate(int size);
template <>
void basic_string< wchar_t >::internal_dereference();
template <>
void basic_string< wchar_t >::internal_prepare_to_write(int len, bool preserve);

template <>
basic_string< wchar_t >::const_iterator basic_string< wchar_t >::position_iterator(int pos) const;
template <>
pair< basic_string< wchar_t >::const_iterator, basic_string< wchar_t >::const_iterator >
basic_string< wchar_t >::range_iterator(int pos, int count) const;

#if RSTL_STRING_ASSIGN_SPECIALIZATION
template <>
basic_string< char >& basic_string< char >::assign(const char* data, int count);
#if !RSTL_STRINGS_PROTO_LAYOUT
template <>
basic_string< char, case_insensitive_char_traits< char > >&
basic_string< char, case_insensitive_char_traits< char > >::assign(const char* data, int count);
#endif
#endif

// Case-insensitive basic_string<char>
template <>
basic_string< char, case_insensitive_char_traits< char > >::basic_string(
    const char* data, int count, const rmemory_allocator& alloc);
template <>
void basic_string< char, case_insensitive_char_traits< char > >::internal_allocate(int size);
template <>
void basic_string< char, case_insensitive_char_traits< char > >::internal_dereference();

#endif

template < typename _CharTp, typename Traits, typename Alloc >
template < typename It, typename OtherIt >
inline int basic_string< _CharTp, Traits, Alloc >::internal_search_of(It first, It last,
                                                                      OtherIt otherFirst,
                                                                      OtherIt otherLast) {
  int index = 0;
  for (It it = first; it != last; ++it, ++index) {
    for (OtherIt other = otherFirst; other != otherLast; ++other) {
      if (Traits::eq(*it, *other)) {
        return index;
      }
    }
  }
  return -1;
}

template < typename _CharTp, typename Traits, typename Alloc >
inline int basic_string< _CharTp, Traits, Alloc >::find_first_of(const basic_string& other,
                                                                 int pos) const {
  pos = get_real_pos_for_begin(pos);
  const int found = internal_search_of(begin() + pos, end(), other.begin(), other.end());
  int result = found + pos;
  if (found == -1) {
    result = found;
  }
  return result;
}

template < typename _CharTp, typename Traits, typename Alloc >
int basic_string< _CharTp, Traits, Alloc >::find_first_of(const _CharTp* other, int pos,
                                                          int count) const {
  pos = get_real_pos_for_begin(pos);
  int length = 0;
  const _CharTp* end = other;
  while ((count == -1 || length < count) && *end != Traits::eos()) {
    ++length;
    ++end;
  }
  const int found = internal_search_of(begin() + pos, this->end(), other, end);
  int result = found + pos;
  if (found == -1) {
    result = found;
  }
  return result;
}

template < typename _CharTp, typename Traits, typename Alloc >
template < typename It, typename OtherIt >
inline int basic_string< _CharTp, Traits, Alloc >::internal_search_not_of(It first, It last,
                                                                          OtherIt otherFirst,
                                                                          OtherIt otherLast) {
  int index = 0;
  for (It it = first; it != last; ++it, ++index) {
    bool notFound = true;
    for (OtherIt other = otherFirst; other != otherLast; ++other) {
      if (Traits::eq(*it, *other)) {
        notFound = false;
      }
    }
    if (notFound) {
      return index;
    }
  }
  return -1;
}

template < typename _CharTp, typename Traits, typename Alloc >
int basic_string< _CharTp, Traits, Alloc >::find_first_not_of(const basic_string& other,
                                                              int pos) const {
  pos = get_real_pos_for_begin(pos);
  const int found = internal_search_not_of(begin() + pos, end(), other.begin(), other.end());
  int result = found + pos;
  if (found == -1) {
    result = found;
  }
  return result;
}

template < typename _CharTp, typename Traits, typename Alloc >
int basic_string< _CharTp, Traits, Alloc >::find_last_not_of(const basic_string& other,
                                                             int pos) const {
  pos = get_real_pos_for_end(pos);
  const int found = internal_search_not_of(reverse_iterator< const_iterator >(begin() + pos + 1),
                                           reverse_iterator< const_iterator >(begin()),
                                           reverse_iterator< const_iterator >(other.end()),
                                           reverse_iterator< const_iterator >(other.begin()));
  int result = pos - found;
  if (found == -1) {
    result = found;
  }
  return result;
}

#if RSTL_STRING_MATCH_LOOP_SEARCH
template < typename _CharTp, typename Traits, typename Alloc >
template < typename It, typename OtherIt >
inline int basic_string< _CharTp, Traits, Alloc >::internal_search(It first, It last,
                                                                   OtherIt otherFirst,
                                                                   const OtherIt otherLast) {
  if (otherFirst == otherLast) {
    return 0;
  }
  int index = 0;
  for (It it = first; it != last; ++it, ++index) {
    if (Traits::eq(*it, *otherFirst)) {
      It match = it;
      ++match;
      OtherIt search = otherFirst;
      ++search;
      while (match != last) {
        if (!Traits::eq(*match, *search)) {
          break;
        }
        ++match;
        ++search;
        if (search == otherLast) {
          break;
        }
      }
      if (search == otherLast) {
        return index;
      }
    }
  }
  return -1;
}

#elif RSTL_STRING_OUT_OF_CLASS
template < typename _CharTp, typename Traits, typename Alloc >
template < typename It, typename OtherIt >
int basic_string< _CharTp, Traits, Alloc >::internal_search(It first, It last, OtherIt otherFirst,
                                                            const OtherIt otherLast) {
  if (otherFirst == otherLast) {
    return 0;
  }
  int index = 0;
  for (It it = first; it != last; ++it, ++index) {
    if (Traits::eq(*it, *otherFirst)) {
      It next = it;
      ++next;
      OtherIt search = otherFirst + 1;
      while (search != otherLast && next != last) {
        if (!Traits::eq(*next, *search)) {
          break;
        }
        ++next;
        ++search;
      }
      if (search == otherLast) {
        return index;
      }
    }
  }
  return -1;
}

#else
template < typename _CharTp, typename Traits, typename Alloc >
template < typename It, typename OtherIt >
inline int basic_string< _CharTp, Traits, Alloc >::internal_search(It first, It last,
                                                                   OtherIt otherFirst,
                                                                   const OtherIt otherLast) {
  if (otherFirst == otherLast) {
    return 0;
  }
  It it = first;
  int matched = 0;
  OtherIt search = otherFirst;
  for (; it != last; ++it) {
    if (Traits::eq(*it, *search)) {
      ++search;
      ++matched;
      if (search == otherLast) {
        return (it - first) - matched + 1;
      }
    } else {
      search = otherFirst;
      matched = 0;
    }
  }
  return -1;
}

#endif
template < typename _CharTp, typename Traits, typename Alloc >
#if !RSTL_STRING_OUT_OF_CLASS
inline
#endif
int basic_string< _CharTp, Traits, Alloc >::find(const basic_string& other, int pos) const {
  pos = get_real_pos_for_begin(pos);
  const int found = internal_search(begin() + pos, end(), other.begin(), other.end());
  int result = found + pos;
  if (found == -1) {
    result = found;
  }
  return result;
}

template < typename _CharTp, typename Traits, typename Alloc >
int basic_string< _CharTp, Traits, Alloc >::find(const _CharTp* other, int pos, int count) const {
  pos = get_real_pos_for_begin(pos);
  const pair< const _CharTp*, int > length = compute_length(other, count);
  const int found = internal_search(begin() + pos, this->end(), other, length.first);
  int result = found + pos;
  if (found == -1) {
    result = found;
  }
  return result;
}


template < typename _CharTp, typename Traits, typename Alloc >
int basic_string< _CharTp, Traits, Alloc >::find(_CharTp ch, int pos) const {
  pos = get_real_pos_for_begin(pos);
  const int found = internal_search(begin() + pos, end(), static_cast< const _CharTp* >(&ch),
                                    static_cast< const _CharTp* >(&ch) + 1);
  int result = found + pos;
  if (found == -1) {
    result = found;
  }
  return result;
}

#if RSTL_STRING_MATCH_LOOP_SEARCH
template < typename _CharTp, typename Traits, typename Alloc >
int basic_string< _CharTp, Traits, Alloc >::rfind(const _CharTp* other, int pos, int count) const {
  pos = get_real_pos_for_end(pos);
  int length = 0;
  const _CharTp* end = other;
  while ((count == -1 || length < count) && *end != Traits::eos()) {
    ++end;
    ++length;
  }
  const int found = internal_search(reverse_iterator< const_iterator >(begin() + pos + 1),
                                    reverse_iterator< const_iterator >(begin()),
                                    reverse_iterator< const _CharTp* >(end),
                                    reverse_iterator< const _CharTp* >(other));
  int result = pos - found - length + 1;
  if (found == -1) {
    result = found;
  }
  return result;
}

template < typename _CharTp, typename Traits, typename Alloc >
int basic_string< _CharTp, Traits, Alloc >::rfind(_CharTp ch, int pos) const {
  pos = get_real_pos_for_end(pos);
  const int found =
      internal_search(reverse_iterator< const_iterator >(begin() + pos + 1),
                      reverse_iterator< const_iterator >(begin()),
                      static_cast< const _CharTp* >(&ch), static_cast< const _CharTp* >(&ch) + 1);
  int result = pos - found;
  if (found == -1) {
    result = found;
  }
  return result;
}

#endif

#if !RSTL_STRING_OUT_OF_CLASS || RSTL_STRING_INLINE_CHAR_COMPARE
template < typename _CharTp, typename Traits, typename Alloc >
inline int basic_string< _CharTp, Traits, Alloc >::compare(const _CharTp* rhs, int count) const {
  const _CharTp* rhsEnd = compute_length(rhs, count).first;
  return internal_compare(begin(), end(), rhs, rhsEnd);
}

#elif RSTL_STRING_COMPARE_END_POINTER
template < typename _CharTp, typename Traits, typename Alloc >
int basic_string< _CharTp, Traits, Alloc >::compare(const _CharTp* rhs, int count) const {
  int rhsCharCount = 0;
  const _CharTp* rhsEnd = rhs;
  while ((count == -1 || rhsCharCount < count) && *rhsEnd != '\0') {
    ++rhsEnd;
    ++rhsCharCount;
  }
  return internal_compare(begin(), end(), rhs, rhsEnd);
}

#else
template < typename _CharTp, typename Traits, typename Alloc >
int basic_string< _CharTp, Traits, Alloc >::compare(const _CharTp* rhs, int count) const {
  int rhsCharCount = 0;
  const _CharTp* rhsStart = rhs;
  while ((count == -1 || rhsCharCount < count) && *rhs != '\0') {
    ++rhs;
    ++rhsCharCount;
  }
  return internal_compare(begin(), end(), rhsStart, rhs);
}

#endif
template < typename _CharTp, typename Traits, typename Alloc >
template < typename It >
inline int basic_string< _CharTp, Traits, Alloc >::internal_compare(const_iterator first,
                                                                    const_iterator last,
                                                                    It otherFirst, It otherLast) {
  const_iterator it = first;
  It other = otherFirst;
  for (; it != last && other != otherLast; ++it, ++other) {
    int cmp = Traits::compare(*it, *other);
    if (cmp != 0) {
      return cmp;
    }
  }
  if (it == last && other != otherLast) {
    return -1;
  }
  if (it == last) {
    return 0;
  }
  return 1;
}

template < typename _CharTp, typename Traits, typename Alloc >
inline int basic_string< _CharTp, Traits, Alloc >::compare(const basic_string& other) const {
  return internal_compare(begin(), end(), other.begin(), other.end());
}

template < typename _CharTp, typename Traits, typename Alloc >
inline bool basic_string< _CharTp, Traits, Alloc >::operator==(const basic_string& other) const {
  return compare(other) == 0;
}

template < typename _CharTp, typename Traits, typename Alloc >
inline bool basic_string< _CharTp, Traits, Alloc >::operator!=(const basic_string& other) const {
  return compare(other) != 0;
}

#ifndef __MWERKS__
// Modern compilers require these specializations before the inline operators or
// out-of-line string methods can implicitly instantiate them.
template <>
basic_string< char >::basic_string(const basic_string& other);
template <>
basic_string< char >& basic_string< char >::append(const char* data, int count);
template <>
basic_string< char >& basic_string< char >::append(int count, char value);
template <>
void basic_string< char >::internal_allocate(int size);
template <>
void basic_string< char >::internal_dereference();
template <>
void basic_string< char >::internal_prepare_to_write(int len, bool preserve);

template <>
basic_string< wchar_t >::basic_string(const basic_string& other);
template <>
basic_string< wchar_t >& basic_string< wchar_t >::append(const basic_string& other);
template <>
basic_string< wchar_t >& basic_string< wchar_t >::append(const wchar_t* data, int count);
template <>
void basic_string< wchar_t >::internal_allocate(int size);
template <>
void basic_string< wchar_t >::internal_dereference();
template <>
void basic_string< wchar_t >::internal_prepare_to_write(int len, bool preserve);

template <>
void basic_string< char, case_insensitive_char_traits< char > >::internal_allocate(int size);
template <>
void basic_string< char, case_insensitive_char_traits< char > >::internal_dereference();
#endif

#if RSTL_STRING_PROTO_TAIL
typedef basic_string< wchar_t > wstring;
typedef basic_string< char > string;
typedef basic_string< char, case_insensitive_char_traits< char > > case_insensitive_string;
typedef case_insensitive_string istring;

istring istring_l(const char* data);
inline bool operator==(const istring& a, const istring& b) { return a.compare(b) == 0; }

template <>
inline wstring::basic_string(wstring::literal_t, const wchar_t* data) {
  mPtr = data;
  mCow = 0;
  const wchar_t* end = data;
  while (*end) {
    ++end;
  }
  mSize = end - data;
}

inline bool operator<(const string& lhs, const string& rhs) { return lhs.compare(rhs) < 0; }

bool operator==(const string& lhs, const char* rhs);

bool operator==(const char* lhs, const string& rhs);
bool operator!=(const string& lhs, const char* rhs);

wstring wstring_l(const wchar_t* data);

string string_l(const char* data);

string operator+(const string& a, const string& b);
inline wstring operator+(const wstring& a, const wstring& b) {
  wstring result(a);
  result.append(b);
  return result;
}

inline string operator+(const string& a, char c) {
  string result(a);
  result.append(1, c);
  return result;
}

inline string operator+(const string& a, const char* c) {
  string result(a);
  result.append(c, -1);
  return result;
}

static inline wstring operator+(const wstring& a, const wchar_t* c) {
  wstring result(a);
  result.append(c, -1);
  return result;
}

inline wstring operator+(const wchar_t* a, const wstring& b) {
  wstring result(a);
  result.append(b);
  return result;
}

inline string operator+(const char* a, const string& b) {
  string result(a);
  result.append(b);
  return result;
}

CHECK_SIZEOF(string, 0x10)
template < typename _CharTp, typename Traits, typename Alloc >
basic_string< _CharTp, Traits, Alloc >
basic_string< _CharTp, Traits, Alloc >::substr(int pos, int count) const {
  const pair< const_iterator, const_iterator > range = range_iterator(pos, count);
  return basic_string(range.first, range.second);
}

#elif RSTL_STRING_PREDECLARED_SPECIALIZATIONS
// Declare the out-of-line specializations (defined in rstl_strings.cpp and
// CScriptStreamedMusic.cpp) before first use.
typedef basic_string< wchar_t > wstring;
typedef basic_string< char > string;
typedef basic_string< char, case_insensitive_char_traits< char > > istring;

inline bool operator<(const string& lhs, const string& rhs) { return lhs.compare(rhs) < 0; }

#if RSTL_STRING_INLINE_CHAR_COMPARE
inline bool operator==(const string& lhs, const char* rhs) { return lhs.compare(rhs) == 0; }
#else
bool operator==(const string& lhs, const char* rhs);
#endif

bool operator==(const char* lhs, const string& rhs);
bool operator!=(const string& lhs, const char* rhs);

istring istring_l(const char* data);
inline bool operator==(const istring& a, const istring& b) { return a.compare(b) == 0; }

wstring wstring_l(const wchar_t* data);

string string_l(const char* data);

#if RSTL_STRING_INLINE_CONCAT
inline string operator+(const string& a, const string& b) {
  string result(a);
  result.append(b);
  return result;
}
#else
string operator+(const string& a, const string& b);
#endif
inline wstring operator+(const wstring& a, const wstring& b) {
  wstring result(a);
  result.append(b);
  return result;
}

inline wstring operator+(const wchar_t* a, const wstring& b) {
  wstring result(a);
  result.append(b);
  return result;
}

inline string operator+(const string& a, char c) {
  string result(a);
  result.append(1, c);
  return result;
}

inline string operator+(const char* a, const string& b) {
  string result(a);
  result.append(b);
  return result;
}

inline string operator+(const string& a, const char* c) {
  string result(a);
  result.append(c, -1);
  return result;
}

inline wstring operator+(const wstring& a, const wchar_t* c) {
  wstring result(a);
  result.append(c, -1);
  return result;
}

CHECK_SIZEOF(string, (RSTL_ALLOCATOR_BASE_CLASS ? 0xc : 0x10))

template < typename _CharTp, typename Traits, typename Alloc >
basic_string< _CharTp, Traits, Alloc >
basic_string< _CharTp, Traits, Alloc >::substr(int pos, int count) const {
  const pair< const_iterator, const_iterator > range = range_iterator(pos, count);
  return basic_string(range.first, range.second);
}

#else
typedef basic_string< wchar_t > wstring;
typedef basic_string< char > string;
typedef basic_string< char, case_insensitive_char_traits< char > > istring;

inline bool operator<(const string& lhs, const string& rhs) { return lhs.compare(rhs) < 0; }

inline istring istring_l(const char* data) { return istring(istring::literal_t(), data); }
inline bool operator==(const istring& a, const istring& b) { return a.compare(b) == 0; }

inline bool operator==(const string& lhs, const char* rhs) { return lhs.compare(rhs) == 0; }
inline bool operator==(const char* lhs, const string& rhs) { return rhs.compare(lhs, -1) == 0; }
inline bool operator!=(const string& lhs, const char* rhs) { return lhs.compare(rhs) != 0; }

inline wstring wstring_l(const wchar_t* data) {
  return wstring(wstring::literal_t(), data);
}
template <>
basic_string< char >::basic_string(const basic_string& other);

template <>
basic_string< char >& basic_string< char >::append(const basic_string& other);

template <>
basic_string< char >& basic_string< char >::append(const char* data, int count);

template <>
basic_string< char >& basic_string< char >::append(int count, char value);

template <>
void basic_string< char >::internal_allocate(int size);

template <>
void basic_string< char >::internal_dereference();

template <>
void basic_string< char >::internal_prepare_to_write(int len, bool preserve);

template <>
basic_string< wchar_t >::basic_string(const basic_string& other);

template <>
basic_string< wchar_t >& basic_string< wchar_t >::append(const basic_string& other);

template <>
basic_string< wchar_t >& basic_string< wchar_t >::append(const wchar_t* data, int count);

template <>
basic_string< wchar_t >& basic_string< wchar_t >::append(int count, wchar_t value);

template <>
void basic_string< wchar_t >::internal_allocate(int size);

template <>
void basic_string< wchar_t >::internal_dereference();

template <>
void basic_string< wchar_t >::internal_prepare_to_write(int len, bool preserve);

template <>
void basic_string< char, case_insensitive_char_traits< char > >::internal_allocate(int size);

inline string string_l(const char* data) { return string(string::literal_t(), data); }

inline string operator+(const string& a, const string& b) {
  string result(a);
  result.append(b);
  return result;
}
inline wstring operator+(const wstring& a, const wstring& b) {
  wstring result(a);
  result.append(b);
  return result;
}

inline string operator+(const string& a, char c) {
  string result(a);
  result.append(1, c);
  return result;
}

inline string operator+(const string& a, const char* c) {
  string result(a);
  result.append(c, -1);
  return result;
}

static inline wstring operator+(const wstring& a, const wchar_t* c) {
  wstring result(a);
  result.append(c, -1);
  return result;
}

CHECK_SIZEOF(string, (RSTL_ALLOCATOR_BASE_CLASS ? 0xc : 0x10))

template < typename _CharTp, typename Traits, typename Alloc >
basic_string< _CharTp, Traits, Alloc >
basic_string< _CharTp, Traits, Alloc >::substr(int pos, int count) const {
  const pair< const_iterator, const_iterator > range = range_iterator(pos, count);
  return basic_string(range.first, range.second);
}
#endif

} // namespace rstl

#endif // _RSTL_STRING
