#ifndef _CZIPINPUTSTREAM
#define _CZIPINPUTSTREAM

#include "rstl/RstlVersions.h"

#include "types.h"

#include "Kyoto/Streams/CInputStream.hpp"

#include "rstl/auto_ptr.hpp"

#if RSTL_KYOTO_BLOCK_INPUT_STREAM
#include "rstl/single_ptr.hpp"

#include <zlib.h>

// Inflates blocks of the owned input stream as they are read.
class CZipInputStream : public CInputStream {
public:
  CZipInputStream(rstl::auto_ptr< CInputStream > in);
  ~CZipInputStream() override;
  size_t Read(void* dest, size_t len) override;

private:
  rstl::single_ptr< uchar > mCompBuf;
  rstl::auto_ptr< CInputStream > mStream;
  rstl::single_ptr< z_stream_s > mZStream;
};
#else
// Guessed name. Inflates the zip data read from the owned input stream.
class CZipInputStream : public CInputStream {
public:
  CZipInputStream(rstl::auto_ptr< CInputStream >& input, uint compressedLength,
                  uint uncompressedLength);
  virtual ~CZipInputStream();
};
#endif

#endif // _CZIPINPUTSTREAM
