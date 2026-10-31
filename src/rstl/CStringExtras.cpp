#include "rstl/RstlVersions.h"
// The prototype's string helpers live in RstlExtras.cpp.
#if !RSTL_STRING_EXTRAS_IN_RSTL_EXTRAS
#include "rstl/StringExtras.hpp"
#include "rstl/math.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/CCast.hpp"
#if RSTL_STRING_LIST_FACTORY
#if RSTL_PRIME1
#include "Kyoto/CFactoryMgr.hpp"
#else
#include "Kyoto/CFactoryMgr.hpp"
#endif
#endif
#include "Kyoto/Streams/CInputStream.hpp"

#if RSTL_STRING_CREATE_FROM_REAL
#include <stdio.h>
#endif
#include <string.h>

int CStringExtras::IndexOfSubstring(const rstl::string& left, const rstl::string& right) {
  int rightSize = right.length();
  if (rightSize == 0) {
    return 0;
  }
  int leftSize = left.length();
  if (leftSize == 0) {
    return -1;
  }

  for (int i = 0; i < (leftSize - rightSize) + 1; ++i) {
    int j = 0;
    for (; j < rightSize; ++j) {
      const int index = i + j;
#if !RSTL_STRING_EXTRAS_AT_ACCESS
      if (ConvertToUpperCase(right[j]) != ConvertToUpperCase(left[index])) {
#else
      const int leftUpper = ConvertToUpperCase(left.at(index));
      const int rightUpper = ConvertToUpperCase(right.at(j));
      if (rightUpper != leftUpper) {
#endif
        break;
      }
    }
    if (j == rightSize) {
      return i;
    }
  }
  return -1;
}

int CStringExtras::CompareCaseInsensitive(const rstl::string& left, const rstl::string& right) {
#if !RSTL_STRING_EXTRAS_AT_ACCESS
  int leftLength = left.length();
#endif
  int rightLength = right.length();
#if RSTL_STRING_EXTRAS_AT_ACCESS
  int leftLength = left.length();
#endif
  int commonLength = rstl::min_val(leftLength, rightLength);

  for (int idx = 0; idx < commonLength; ++idx) {
#if !RSTL_STRING_EXTRAS_AT_ACCESS
    if (ConvertToUpperCase(left[idx]) < ConvertToUpperCase(right[idx])) {
#else
    int rightUpper = ConvertToUpperCase(right.at(idx));
    if (ConvertToUpperCase(left.at(idx)) < rightUpper) {
#endif
      return -1;
    }

#if !RSTL_STRING_EXTRAS_AT_ACCESS
    if (ConvertToUpperCase(left[idx]) > ConvertToUpperCase(right[idx])) {
#else
    rightUpper = ConvertToUpperCase(right.at(idx));
    if (ConvertToUpperCase(left.at(idx)) > rightUpper) {
#endif
      return 1;
    }
  }
  if (leftLength < rightLength) {
    return -1;
  } else if (leftLength > rightLength) {
    return 1;
  } else {
    return 0;
  }
}

rstl::string CStringExtras::ConvertToLowerCase(const rstl::string& str) {
  rstl::string ret(str);
  for (int i = 0; i < ret.length(); ++i) {
    const unsigned char& c = reinterpret_cast< const unsigned char* >(ret.data())[i];
    ret.reserve(ret.length());
    char& out = const_cast< char* >(ret.data())[i];
    out = ConvertToLowerCase(c);
  }
  return ret;
}

rstl::string CStringExtras::CreatePrefix(const rstl::string& str, int count) {
  const char* data = str.data();
  if (str.length() < count) {
    count = str.length();
  }
  return rstl::string(data, count);
}

char CStringExtras::ConvertToUpperCase(char c) {
  if (c >= CCast::ToChar('a') && c <= CCast::ToChar('z')) {
    return (c - CCast::ToChar(' ')) & 0xFF;
  }

  return c;
}

int CStringExtras::ConvertToLowerCase(int c) {
  const char value = static_cast< char >(c);
  if (value >= CCast::ToChar('A') && value <= CCast::ToChar('Z')) {
    return CCast::ToChar(c + CCast::ToChar(' '));
  }

  return c;
}

rstl::string CStringExtras::CreateFromInteger(int v) {
  int magnitude = v < 0 ? -v : v;
  int count = 0;
#if !RSTL_STRING_EXTRAS_AT_ACCESS
  schar digits[24];
#else
  signed char digits[24];
#endif
  if (v == 0) {
    return rstl::string_l("0");
  }

  while (magnitude > 0) {
    digits[count] = '0' + magnitude % 10;
    ++count;
    magnitude /= 10;
  }

  rstl::string ret;
  if (v < 0) {
    ret.append("-", -1);
  }

  for (int i = 0; i < count; ++i) {
    ret.append(1, digits[count - i - 1]);
  }

  return ret;
}

#if RSTL_STRING_CREATE_FROM_REAL
rstl::string CStringExtras::CreateFromReal(float v, int i) {
  char a[16];
  char b[136];

  sprintf(b, "%%.%df", (i > 12) ? 12 : i);
  sprintf(b, a, v);
  return rstl::string(b);
}
#endif

rstl::string CStringExtras::ConvertToANSI(const rstl::wstring& str) {
  rstl::string ret;
  ret.reserve(str.size() + 1);

  for (int i = 0; i < static_cast< int >(str.size()); ++i) {
#if !RSTL_STRING_EXTRAS_AT_ACCESS
    ret.assign(ret + static_cast< char >(str.at(i)));
#else
    const char c = static_cast< char >(str.at(i));
    if (c != 0) {
      ret.append(1, c);
    }
#endif
  }
  return ret;
}

rstl::wstring CStringExtras::ConvertToUNICODE(const rstl::string& str) {
#if RSTL_UNICODE_EMPTY_EARLY_RETURN
  if (str.length() == 0) {
    return rstl::wstring();
  }

#endif
  rstl::wstring ret;
  ret.reserve(str.size() + 1);

  for (int i = 0; i < static_cast< int >(str.size()); ++i) {
    ret.append(1, static_cast< wchar_t >(str.at(i)));
  }
  return ret;
}

rstl::string CStringExtras::ReadString(CInputStream& in) {
  rstl::string ret;
  int strLen = in.ReadInt32();
  ret.reserve(strLen);
  while (strLen > 0) {
    int len = strLen > 512u ? 512 : strLen;
    char tmp[512];
    in.ReadBytes(tmp, len);
    ret.append(tmp, len);
    strLen -= len;
  }

  return ret;
}

rstl::vector< rstl::string > CStringExtras::TokenizeString(const rstl::string& string,
                                                           const char* delims, int expectedSize) {
  rstl::vector< rstl::string > ret;
  if (expectedSize > 0) {
    ret.reserve(expectedSize);
  }

  const int size = string.length();
  int pos = 0;
  while (pos < size) {
    while (pos < size && strchr(delims, string[pos]) != nullptr) {
      ++pos;
    }
    if (pos == size) {
      break;
    }

    int end = pos + 1;
    while (end < size && strchr(delims, string[end]) == nullptr) {
      ++end;
    }
#if !RSTL_VECTOR_PUSH_BACK_UNSAFE
    ret.push_back(string.substr(pos, end - pos));
#else
    ret.push_back_unsafe(string.substr(pos, end - pos));
#endif
    pos = end + 1;
  }

  return ret;
}

#if RSTL_STRING_LIST_FACTORY
KYOTO_START_GAME_NAMESPACE
CFactoryFnReturn FStringListFactory(const SObjectTag&, CInputStream& in, const CVParamTransfer&) {
  return rs_new rstl::vector< rstl::string >(in);
}
KYOTO_END_GAME_NAMESPACE
#endif
#endif // !RSTL_STRING_EXTRAS_IN_RSTL_EXTRAS
