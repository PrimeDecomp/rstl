#ifndef KYOTO_STREAMS_CTEXTINSTREAM_HPP
#define KYOTO_STREAMS_CTEXTINSTREAM_HPP

#include "rstl/string.hpp"

class CInputStream;

// Guessed name. Reads newline-terminated text lines from a binary input stream.
class CTextInStream {
public:
  CTextInStream(CInputStream& input, int length);
  rstl::string GetNextLine();
  // Guessed name for the end-of-text flag.
  bool AtEnd() const { return mAtEnd; }

private:
  int mLength;
  bool mAtEnd;
  CInputStream& mStream;
};

#endif
