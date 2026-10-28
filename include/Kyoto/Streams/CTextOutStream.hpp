#ifndef KYOTO_STREAMS_CTEXTOUTSTREAM_HPP
#define KYOTO_STREAMS_CTEXTOUTSTREAM_HPP

#include "rstl/string.hpp"

class COutputStream;

// Guessed name. Formats text lines onto a binary output stream.
class CTextOutStream {
public:
  explicit CTextOutStream(COutputStream& output);
  void WriteString(const char* format, ...);
  // Guessed name. Writes the text's bytes to the output stream.
  void WriteString(const rstl::string& text);

private:
  COutputStream& mOutput;
};

#endif
