#ifndef _CLZOINPUTSTREAM
#define _CLZOINPUTSTREAM

#include "types.h"

#include "Kyoto/Streams/CInputStream.hpp"

#include "rstl/auto_ptr.hpp"

class CLZOInputStream : public CInputStream {
public:
  CLZOInputStream(const rstl::auto_ptr< CInputStream >& in, unsigned long compressedLen,
                  unsigned long decompressedLen);
  ~CLZOInputStream() override;
#if RSTL_KYOTO_BLOCK_INPUT_STREAM
  size_t Read(void* dest, size_t len) override;
  rstl::auto_ptr< uchar > ReleaseBuffer();
#endif
};

CHECK_CHILD_SIZEOF(CLZOInputStream, CInputStream, 0x0)

#endif // _CLZOINPUTSTREAM
