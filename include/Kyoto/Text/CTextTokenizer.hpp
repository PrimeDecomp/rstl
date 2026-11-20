#ifndef KYOTO_TEXT_CTEXTTOKENIZER_HPP
#define KYOTO_TEXT_CTEXTTOKENIZER_HPP

#include "rstl/string.hpp"

class CTextTokenizer {
public:
  // Guessed names for the values.
  enum ETokenType {
    kTT_Unquoted,
    kTT_Quoted,
  };

  CTextTokenizer(const char* input);

  rstl::string GetNextToken(ETokenType* tokenType, char quote);
  bool IsAtEnd() const;
  const char* GetRemainingText() const; // Guessed name

private:
  const char* mInput;
};

#endif
