#include "Kyoto/Streams/CTextInStream.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

CTextInStream::CTextInStream(CInputStream& input, int length)
: mLength(length), mAtEnd(false), mStream(input) {}

rstl::string CTextInStream::GetNextLine() {
  rstl::string line;
  line.reserve(128);
  while (!mAtEnd) {
    const char c = mStream.ReadInt8();
    if (c == '\0') {
      mAtEnd = true;
      continue;
    }
    const uint position = mStream.GetReadPosition();
    if (position == mLength) {
      mAtEnd = true;
    }
    if (c == '\n') {
      break;
    }
    line.append(rstl::string(c));
  }
  return line;
}
