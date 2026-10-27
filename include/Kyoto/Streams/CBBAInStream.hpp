#ifndef _CBBAINSTREAM
#define _CBBAINSTREAM

#include "types.h"

#include "Kyoto/Streams/CInputStream.hpp"

// An input stream over a whole file read from the development host through the broadband adapter
// (CBBAInStream.cpp). The constructor loads the file into an owned buffer.
class CBBAInStream : public CInputStream {
public:
  explicit CBBAInStream(const char* path);
  ~CBBAInStream();

  // Guessed name. The bytes of the file not read yet.
  uint GetRemainingBytes() const;

private:
  // Guessed name. Reads the whole file into a new buffer, which the caller owns.
  static SBufferAndSize LoadFileBuffer(const char* path);
};

#endif // _CBBAINSTREAM
