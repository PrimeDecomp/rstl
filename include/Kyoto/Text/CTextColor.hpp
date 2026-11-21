#ifndef _CTEXTCOLOR
#define _CTEXTCOLOR
#include "rstl/RstlVersions.h"

#if !RSTL_CORRUPTION
#include "rstl/macros.hpp"
#endif
#include "rstl/types.h"

#ifdef __MWERKS__
#pragma cpp_extensions on
#endif

class CTextColor {
public:
  // TODO: Verify component ordering, there is evidence this might actually be ABGR instead of RGBA
  CTextColor(uchar r, uchar g, uchar b, uchar a) : mR(r), mG(g), mB(b), mA(a) {}

  CTextColor(const CTextColor& other) : mR(other.mR), mG(other.mG), mB(other.mB), mA(other.mA) {}

  const CTextColor& operator=(const CTextColor& other) {
    mR = other.mR;
    mG = other.mG;
    mB = other.mB;
    mA = other.mA;
    return *this;
  }

#if RSTL_CORRUPTION
  uint GetRGBA() const { return mRgba; }
#else
#ifdef __MWERKS__
  uint GetRGBA() const { return mRgba; }
#else
  uint GetRGBA() const { return (uint(mR) << 24) | (uint(mG) << 16) | (uint(mB) << 8) | mA; }
#endif

  const uchar GetAlpha() const { return mA; }
  const uchar GetBlue() const { return mB; }
  const uchar GetGreen() const { return mG; }
  const uchar GetRed() const { return mR; }
#endif

private:
#if RSTL_CORRUPTION
  union {
    struct {
      uchar mR;
      uchar mG;
      uchar mB;
      uchar mA;
    };
    uint mRgba;
  };
#else
#ifdef __MWERKS__
  union {
    struct {
      uchar mR;
      uchar mG;
      uchar mB;
      uchar mA;
    };
    uint mRgba;
  };
#else
  ALIGNAS(uint) uchar mR;
  uchar mG;
  uchar mB;
  uchar mA;
#endif
#endif
};

#ifdef __MWERKS__
#if RSTL_CORRUPTION
#pragma cpp_extensions off
#else
#pragma cpp_extensions reset
#endif
#endif

#endif // _CTEXTCOLOR
