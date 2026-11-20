#ifndef KYOTO_TEXT_CSTRINGTOKENIZER_HPP
#define KYOTO_TEXT_CSTRINGTOKENIZER_HPP

#include "Kyoto/Text/CTextTokenizer.hpp"

// Compatibility for callers that still use the prototype's earlier names.
class CStringTokenizer : public CTextTokenizer {
public:
  CStringTokenizer(const char* input) : CTextTokenizer(input) {}

  rstl::string ReadToken(int* quoted, char quote) {
    if (quoted == 0) {
      return GetNextToken(0, quote);
    }

    ETokenType tokenType;
    rstl::string token = GetNextToken(&tokenType, quote);
    *quoted = tokenType;
    return token;
  }

  bool IsExhausted() const { return IsAtEnd(); }
};

#endif
