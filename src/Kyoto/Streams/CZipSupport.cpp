#include "Kyoto/Streams/CZipSupport.hpp"
#include "rstl/RstlVersions.h"
#include "Kyoto/Alloc/Assert.hpp"
#include "Kyoto/Alloc/CMemory.hpp"

#if RSTL_KYOTO_ZIP_TYPES_DECLARED
#include "Kyoto/Streams/ZipTypes.hpp"
#else
#include <zlib.h>

static const char* hack() { return ZLIB_VERSION; }
#endif

void* CZipSupport::Alloc(void* ptr1, uint w1, uint w2) { return rs_new_line(19) uchar[w1 * w2]; }

void CZipSupport::Free(void* ptr1, void* ptr2) {
  if (ptr2 == nullptr) {
    return;
  }
  delete[] static_cast< uchar* >(ptr2);
}

#if !RSTL_KYOTO_BLOCK_INPUT_STREAM
bool CZipSupport::Decompress(const void* source, uint sourceLength, void* output,
                             uint outputLength) {
  z_stream stream;
  stream.zalloc = Alloc;
  stream.zfree = Free;
  stream.opaque = 0;
  stream.next_in = static_cast< unsigned char* >(const_cast< void* >(source));
  stream.avail_in = sourceLength;

  int err = inflateInit_(&stream, "1.1.3", sizeof(z_stream));
  RS_VERIFY_THROW(67, err == Z_OK, false, "Error in inflateInit");

  stream.next_out = static_cast< unsigned char* >(output);
  stream.avail_out = outputLength;

  while (err != Z_STREAM_END) {
    err = inflate(&stream, 0);
    RS_VERIFY_THROW(76, err == Z_OK || err == Z_STREAM_END, false,
                    "CInputStream::kException_StreamError");
  }
  inflateEnd(&stream);
  return true;
}
#endif
