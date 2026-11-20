#include "Kyoto/Text/CTextTokenizer.hpp"

#include <string.h>

CTextTokenizer::CTextTokenizer(const char* input) : mInput(input) {}

bool CTextTokenizer::IsAtEnd() const {
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

rstl::string CTextTokenizer::GetNextToken(ETokenType* tokenType, char quote) {
  static const char* whitespace = "\r\n\t ";
  static char quotedStops[] = " \r\n\t";
  quotedStops[0] = quote;
  if (tokenType != 0) {
    *tokenType = kTT_Unquoted;
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
    if (tokenType != 0) {
      *tokenType = kTT_Quoted;
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

const char* CTextTokenizer::GetRemainingText() const { return mInput; }

