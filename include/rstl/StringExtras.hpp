#ifndef _RSTL_STRINGEXTRAS
#define _RSTL_STRINGEXTRAS

#include "rstl/RstlVersions.h"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

class CStringExtras {
public:
  static int IndexOfSubstring(const rstl::string&, const rstl::string&);
  static int CompareCaseInsensitive(const rstl::string&, const rstl::string&);
#if RSTL_STRING_CASE_CONVERSION_API
  static char ConvertToUpperCase(char c);
#if RSTL_STRING_EXTRAS_PROTO_API
  static char ConvertToLowerCase(char c);  // Guessed name
#else
  static int ConvertToLowerCase(int c);
#endif
  static rstl::string ConvertToLowerCase(const rstl::string& str);
  static rstl::string CreatePrefix(const rstl::string& str, int count);
#else
  static char ConvertToUpperCase(const char c);
#endif
  static rstl::string CreateFromInteger(int v);
#if RSTL_STRING_EXTRAS_PROTO_API
  static rstl::string CreateFromReal(float v, int precision);
  static int ConvertToInteger(const rstl::string& str);  // Guessed name
  static float ConvertToReal(const rstl::string& str);   // Guessed name
#elif RSTL_STRING_CREATE_FROM_REAL
  static rstl::string CreateFromReal(float v, int i);
#endif
  static rstl::string ConvertToANSI(const rstl::wstring& str);
  static rstl::wstring ConvertToUNICODE(const rstl::string& str);
  static rstl::string ReadString(CInputStream& in);
#if RSTL_STRING_EXTRAS_PROTO_API
  static rstl::string Format(const char* fmt, ...);  // Guessed name
#endif
  static rstl::vector< rstl::string > TokenizeString(const rstl::string&, const char*, int);
};

#endif // _RSTL_STRINGEXTRAS
