#include "Kyoto/Basics/CBasics.hpp"

#include "Kyoto/Basics/CStopwatch.hpp"
#include "rstl/RstlVersions.h"

#include "dolphin/dvd.h"
#include "dolphin/os.h"
#if RSTL_KYOTO_REVOLUTION_SDK
#include "revolution/nand.h"
#endif
#include <string.h>

static bool gInitialized = false;
bool CBasics::Init() {
  if (!gInitialized) {
    OSInit();
    OSInitFastCast();
    DVDInit();
#if RSTL_KYOTO_REVOLUTION_SDK
    NANDInit();
#endif

    CStopwatch::InitGlobalTimer();
    gInitialized = true;
  }
  return gInitialized;
}

#if RSTL_KYOTO_BLOCK_MEMORY_FUNCTIONS
void CBasics::CopyMemory(void* dst, const void* src, uint size) {
  uchar* out = static_cast< uchar* >(dst);
  const uchar* in = static_cast< const uchar* >(src);
  if ((reinterpret_cast< uint >(out) & 31) != (reinterpret_cast< uint >(in) & 31) || size < 32) {
    memcpy(dst, src, size);
  } else if (((reinterpret_cast< uint >(out) | size) & 31) == 0) {
    while (size != 0) {
      __dcbz(out, 0);
      size -= 32;
      double a = reinterpret_cast< const double* >(in)[0];
      double b = reinterpret_cast< const double* >(in)[1];
      double c = reinterpret_cast< const double* >(in)[2];
      double d = reinterpret_cast< const double* >(in)[3];
      in += 32;
      reinterpret_cast< double* >(out)[0] = a;
      reinterpret_cast< double* >(out)[1] = b;
      reinterpret_cast< double* >(out)[2] = c;
      reinterpret_cast< double* >(out)[3] = d;
      out += 32;
    }
  } else {
    uchar* alignedDst = reinterpret_cast< uchar* >((reinterpret_cast< uint >(out) + 31) & ~31);
    const uchar* alignedSrc =
        reinterpret_cast< const uchar* >((reinterpret_cast< uint >(in) + 31) & ~31);
    int head = alignedDst - out;
    if (head != 0) {
      memcpy(out, in, head);
    }
    int blocks = (size - head) & ~31;
    if (blocks != 0) {
      uint n = blocks;
      uchar* blockDst = alignedDst;
      const uchar* blockSrc = alignedSrc;
      while (n != 0) {
        __dcbz(blockDst, 0);
        n -= 32;
        double a = reinterpret_cast< const double* >(blockSrc)[0];
        double b = reinterpret_cast< const double* >(blockSrc)[1];
        double c = reinterpret_cast< const double* >(blockSrc)[2];
        double d = reinterpret_cast< const double* >(blockSrc)[3];
        blockSrc += 32;
        reinterpret_cast< double* >(blockDst)[0] = a;
        reinterpret_cast< double* >(blockDst)[1] = b;
        reinterpret_cast< double* >(blockDst)[2] = c;
        reinterpret_cast< double* >(blockDst)[3] = d;
        blockDst += 32;
      }
    }
    int tail = (int)size - head - blocks;
    if (tail != 0) {
      alignedSrc += blocks;
      alignedDst += blocks;
      memcpy(alignedDst, alignedSrc, tail);
    }
  }
}

void CBasics::ZeroMemory(void* dst, uint size) {
  uchar* out = static_cast< uchar* >(dst);
  if (size < 32) {
    memset(out, 0, size);
  } else if (((reinterpret_cast< uint >(out) | size) & 31) == 0) {
    while (size != 0) {
      __dcbz(out, 0);
      size -= 32;
      out += 32;
    }
  } else {
    uchar* aligned = reinterpret_cast< uchar* >((reinterpret_cast< uint >(out) + 31) & ~31);
    int head = aligned - out;
    if (head != 0) {
      memset(out, 0, head);
    }
    int blocks = (size - head) & ~31;
    if (blocks != 0) {
      uint n = blocks;
      uchar* block = aligned;
      while (n != 0) {
        __dcbz(block, 0);
        n -= 32;
        block += 32;
      }
    }
    int tail = (int)size - head - blocks;
    if (tail != 0) {
      aligned += blocks;
      memset(aligned, 0, tail);
    }
  }
}
#endif

#if RSTL_KYOTO_BLOCK_MEMORY_FILL
static inline double RepeatedByte(uchar value) {
  union {
    uint words[2];
    double block;
  } pattern;
  const uint word = value | (value << 8) | (value << 16) | (static_cast< uint >(value) << 24);
  pattern.words[0] = word;
  pattern.words[1] = word;
  return pattern.block;
}

static inline void FillCacheBlocks(uchar* dst, uint size, uchar value) {
  const double pattern = RepeatedByte(value);
  while (size != 0) {
    __dcbz(dst, 0);
    size -= 32;
    reinterpret_cast< double* >(dst)[0] = pattern;
    reinterpret_cast< double* >(dst)[1] = pattern;
    reinterpret_cast< double* >(dst)[2] = pattern;
    reinterpret_cast< double* >(dst)[3] = pattern;
    dst += 32;
  }
}

void CBasics::FillMemory(void* dst, uchar value, uint size) {
  uchar* out = static_cast< uchar* >(dst);
  if (size < 32) {
    memset(out, value, size);
  } else if (((reinterpret_cast< uint >(out) | size) & 31) == 0) {
    FillCacheBlocks(out, size, value);
  } else {
    uchar* aligned = reinterpret_cast< uchar* >((reinterpret_cast< uint >(out) + 31) & ~31);
    int head = aligned - out;
    if (head != 0) {
      memset(out, value, head);
    }
    int blocks = (size - head) & ~31;
    if (blocks != 0) {
      FillCacheBlocks(aligned, blocks, value);
    }
    int tail = (int)size - head - blocks;
    if (tail != 0) {
      aligned += blocks;
      memset(aligned, value, tail);
    }
  }
}
#endif
