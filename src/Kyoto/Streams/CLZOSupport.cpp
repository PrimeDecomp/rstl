#include "Kyoto/Streams/CLZOSupport.hpp"

#if IS_TARGET_PC
#include <lzokay.hpp>
#else
#include "LZO/lzo1x.h"

extern "C" int __lzo_init_done;
#endif

int CLZOSupport::Inflate(const uchar* source, uint sourceLen, uchar* dest, uint& destLen) {
#if IS_TARGET_PC
  std::size_t out_size;
  auto result = lzokay::decompress(source, sourceLen, dest, destLen, out_size);
  destLen = out_size;
  return static_cast<int>(result);
#else
  if (!__lzo_init_done) {
    lzo_init();
  }
  return lzo1x_decompress(source, sourceLen, dest, &destLen, nullptr);
#endif
}
