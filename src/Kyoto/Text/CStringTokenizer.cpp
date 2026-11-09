#include "Kyoto/Text/CStringTokenizer.hpp"

#include <string.h>

CStringTokenizer::CStringTokenizer(const char* input) : mInput(input) {}

bool CStringTokenizer::IsExhausted() const {
  if (mInput == 0) {
    return true;
  }
  static const char* whitespace = "\r\n\t ";
  for (const char* p = mInput; *p != '\0'; ++p) {
    if (strchr(whitespace, *p) == 0) {
      return false;
    }
  }
  return true;
}

rstl::string CStringTokenizer::ReadToken(int* quoted, char quote) {
  static const char* whitespace = "\r\n\t ";
  static char quotedStops[] = " \r\n\t";
  quotedStops[0] = quote;
  if (quoted != 0) {
    *quoted = 0;
  }
  if (mInput == 0) {
    return rstl::string_l("");
  }
  while (*mInput != '\0' && strchr(whitespace, *mInput) != 0) {
    ++mInput;
  }
  if (*mInput == '\0') {
    mInput = 0;
    return rstl::string_l("");
  }

  bool isQuoted = false;
  if (*mInput == quote) {
    ++mInput;
    isQuoted = true;
    if (quoted != 0) {
      *quoted = 1;
    }
  }
  const char* start = mInput;
  while (*mInput != '\0' && strchr(isQuoted ? quotedStops : whitespace, *mInput) == 0) {
    ++mInput;
  }
  if (isQuoted && *mInput == quote) {
    rstl::string token(start, mInput);
    ++mInput;
    return token;
  }
  if (mInput == start) {
    mInput = 0;
    return rstl::string_l("");
  }
  return rstl::string(start, mInput);
}

const char* CStringTokenizer::GetRemainingText() const { return mInput; }

