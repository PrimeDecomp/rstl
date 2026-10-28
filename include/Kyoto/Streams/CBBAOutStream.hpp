#ifndef _CBBAOUTSTREAM
#define _CBBAOUTSTREAM

#include "types.h"

#include "Kyoto/Streams/COutputStream.hpp"

// An output stream writing a file on the development host through the broadband adapter.
class CBBAOutStream : public COutputStream {
public:
  CBBAOutStream(const char* path, int bufferLength);
  ~CBBAOutStream();

  void Write(const void* ptr, size_t len);
  // Guessed name. The file could not be opened when the handle is -1.
  bool IsOpen() const { return mFile != -1; }

private:
  int mFile;
};
CHECK_SIZEOF(CBBAOutStream, 0x78)

#endif // _CBBAOUTSTREAM
