#include "Kyoto/Streams/CLZOInputStream.hpp"
#include "rstl/RstlVersions.h"

#include "Kyoto/Alloc/CMemory.hpp"
#if RSTL_KYOTO_DEBUG_BUILD
#include "Kyoto/Alloc/Assert.hpp"
#endif
#include "Kyoto/Streams/CLZOSupport.hpp"

static const CInputStream::SBufferAndSize
get_buffer_and_size(CInputStream& in, unsigned long compressedLen, unsigned long decompressedLen) {
  rstl::auto_ptr< uchar > buffer(
      static_cast< uchar* >(RSTL_KYOTO_ALIGNED_NEW(decompressedLen, 13)));
  unsigned long written = 0;
  uchar* dest = buffer.get();
  while (written != decompressedLen) {
    uint blockLen = 0x4000;
    uint compressedBlockLen = static_cast< ushort >(in.ReadUint16());
    const uchar* source = static_cast< const uchar* >(in.Get(compressedBlockLen));
#if RSTL_KYOTO_DEBUG_BUILD
    int decompressResult = CLZOSupport::Inflate(source, compressedBlockLen, dest, blockLen);
    RS_VERIFY_THROW(28, decompressResult >= 0, false,
                    "CLZOSupport::Inflate failed in CInputStream::SBufferAndSize const "
                    "get_buffer_and_size.");
#else
    CLZOSupport::Inflate(source, compressedBlockLen, dest, blockLen);
#endif
    dest += blockLen;
    written += blockLen;
  }
  return CInputStream::SBufferAndSize(buffer.release(), decompressedLen);
}

CLZOInputStream::CLZOInputStream(const rstl::auto_ptr< CInputStream >& in,
                                 unsigned long compressedLen, unsigned long decompressedLen)
: CInputStream(get_buffer_and_size(*in, compressedLen, decompressedLen), true) {
  // Consume ownership after decompression without retaining the source stream.
  rstl::auto_ptr< CInputStream > stream(in);
}

CLZOInputStream::~CLZOInputStream() {}
