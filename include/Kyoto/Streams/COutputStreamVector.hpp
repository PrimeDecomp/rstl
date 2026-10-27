#ifndef _COUTPUTSTREAMVECTOR
#define _COUTPUTSTREAMVECTOR

#include "types.h"

#include "Kyoto/Streams/COutputStream.hpp"
#include "rstl/vector.hpp"

// Guessed name. An output stream collecting its bytes in a vector.
class COutputStreamVector : public COutputStream {
public:
  explicit COutputStreamVector(int bufferLength);
  virtual ~COutputStreamVector();
  virtual void Write(const void* data, size_t length);
  // Flushes the pending bytes first, so it is not const.
  const rstl::vector< uchar >& GetBuffer();

private:
  rstl::vector< uchar > mBytes;
};

#endif // _COUTPUTSTREAMVECTOR
