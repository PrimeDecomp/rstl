#ifndef KYOTO_STREAMS_CFIOOUTSTREAM_HPP
#define KYOTO_STREAMS_CFIOOUTSTREAM_HPP

#include "Kyoto/Streams/COutputStream.hpp"

// Guessed name. Buffered output stream to a host file.
class CFIOOutStream : public COutputStream {
public:
  CFIOOutStream(const char* path, int bufferSize);
  virtual ~CFIOOutStream();
  virtual void Write(const void* data, size_t length);

private:
  // Guessed name. The host file handle, -1 when the file is not open.
  int mFile;
};

#endif
