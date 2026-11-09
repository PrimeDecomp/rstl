#ifndef _CSTRINGTOKENIZER
#define _CSTRINGTOKENIZER

#include "rstl/string.hpp"

// Guessed names
class CStringTokenizer {
public:
  CStringTokenizer(const char* input);

  rstl::string ReadToken(int* quoted, char quote);
  bool IsExhausted() const;
  const char* GetRemainingText() const;

private:
  const char* mInput;
};

#endif // _CSTRINGTOKENIZER
