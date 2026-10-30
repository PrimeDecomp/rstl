#include "rstl/RstlVersions.h"
#include "Kyoto/Graphics/CColor.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

#include "rstl/math.hpp"
#if RSTL_PRIME1
#include "GameVersions.h"
#endif

CColor::CColor(CInputStream& in) {
  float r = in.ReadFloat();
  float g = in.ReadFloat();
  float b = in.ReadFloat();
  Set(r, g, b, in.ReadFloat());
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#ifdef __MWERKS__
CColor::CColor(float r, float g, float b, float a) {
  mR = r * 255.f;
  mG = g * 255.f;
  mB = b * 255.f;
  mA = a * 255.f;
}
#else
CColor::CColor(float r, float g, float b, float a) {
  mR = static_cast< uchar >(CCast::FtoL(r * 255.f));
  mG = static_cast< uchar >(CCast::FtoL(g * 255.f));
  mB = static_cast< uchar >(CCast::FtoL(b * 255.f));
  mA = static_cast< uchar >(CCast::FtoL(a * 255.f));
}
#endif
#else
CColor::CColor(const float r, const float g, const float b, const float a) {
  mR = r * 255.f;
  mG = g * 255.f;
  mB = b * 255.f;
  mA = a * 255.f;
}
#endif

void CColor::Set(float r, float g, float b, float a) {
  mR = CCast::ToUint8(r * 255.f);
  mG = CCast::ToUint8(g * 255.f);
  mB = CCast::ToUint8(b * 255.f);
  mA = CCast::ToUint8(a * 255.f);
}

void CColor::Get(float& r, float& g, float& b, float& a) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  r = CCast::ToReal32(mR) * (1 / 255.f);
  g = CCast::ToReal32(mG) * (1 / 255.f);
  b = CCast::ToReal32(mB) * (1 / 255.f);
  a = CCast::ToReal32(mA) * (1 / 255.f);
#else
  r = CCast::ToReal32(mR) * (1.f / 255.f);
  g = CCast::ToReal32(mG) * (1.f / 255.f);
  b = CCast::ToReal32(mB) * (1.f / 255.f);
  a = CCast::ToReal32(mA) * (1.f / 255.f);
#endif

}

void CColor::Get(float& r, float& g, float& b) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  r = CCast::ToReal32(mR) * (1 / 255.f);
  g = CCast::ToReal32(mG) * (1 / 255.f);
  b = CCast::ToReal32(mB) * (1 / 255.f);
#else
  r = CCast::ToReal32(mR) * (1.f / 255.f);
  g = CCast::ToReal32(mG) * (1.f / 255.f);
  b = CCast::ToReal32(mB) * (1.f / 255.f);
#endif

}

CColor CColor::Lerp(const CColor& a, const CColor& b, float t) {
  const float omt = 1.f - t;
  return CColor(omt * a.GetRed() + t * b.GetRed(), omt * a.GetGreen() + t * b.GetGreen(),
                omt * a.GetBlue() + t * b.GetBlue(), omt * a.GetAlpha() + t * b.GetAlpha());
}

uint CColor::Lerp(uint a, uint b, float t) {
  uint alpha = t * 256.f;

  uint dstrb = a & 0xff00ff;
  uint dstag = a >> 8 & 0xff00ff;

  uint srcrb = b & 0xff00ff;
  uint srcag = b >> 8 & 0xff00ff;

  uint drb = srcrb - dstrb;
  uint dag = srcag - dstag;

  drb *= alpha;
  dag *= alpha;
  drb >>= 8;
  dag >>= 8;

  const uint rb = (drb + dstrb) & 0x00ff00ff;
  const uint ag = (dag + dstag) << 8 & 0xff00ff00;

  return rb | ag;
}

CColor CColor::Modulate(const CColor& a, const CColor& b) {
  return CColor((uchar)((a.GetRedu8() * b.GetRedu8()) / (uchar)255),
                (a.GetGreenu8() * b.GetGreenu8()) / (uchar)255,
                (a.GetBlueu8() * b.GetBlueu8()) / (uchar)255,
                (a.GetAlphau8() * b.GetAlphau8()) / (uchar)255);
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CColor CColor::Add(const CColor& arg0, const CColor& arg1) {
  return CColor((uchar)rstl::min_val< uint >(255, arg0.GetRedu8() + arg1.GetRedu8()),
                (uchar)rstl::min_val< uint >(255, arg0.GetGreenu8() + arg1.GetGreenu8()),
                (uchar)rstl::min_val< uint >(255, arg0.GetBlueu8() + arg1.GetBlueu8()),
                (uchar)rstl::min_val< uint >(255, arg0.GetAlphau8() + arg1.GetAlphau8()));
#else
CColor CColor::Add(const CColor& a, const CColor& b) {
  const uchar alpha = rstl::min_val< uint >(255, a.GetAlphau8() + b.GetAlphau8());
  const uchar blue = rstl::min_val< uint >(255, a.GetBlueu8() + b.GetBlueu8());
  const uchar green = rstl::min_val< uint >(255, a.GetGreenu8() + b.GetGreenu8());
  const uchar red = rstl::min_val< uint >(255, a.GetRedu8() + b.GetRedu8());
  return CColor(red, green, blue, alpha);
#endif

}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION >= VERSION_GM8P_00
CColor CColor::FromRGB5A3(uint color) {
  const ushort c = color;
  if (c & 0x8000) {
    const GXColor col = {static_cast< uchar >(((c & 0x7c00) >> 7) | ((c & 0x7000) >> 13)),
                         static_cast< uchar >(((c & 0x3e0) >> 2) | ((c & 0x380) >> 7)),
                         static_cast< uchar >(((c & 0x1f) << 3) | ((c & 0x1c) >> 2)), 0xff};
    return CColor(col.r, col.g, col.b, col.a);
  } else {
    const GXColor col = {static_cast< uchar >(((c & 0xf00) >> 4) | ((c & 0xf00) >> 8)),
                         static_cast< uchar >((c & 0xf0) | ((c & 0xf0) >> 4)),
                         static_cast< uchar >(((c & 0xf) << 4) | (c & 0xf)),
                         static_cast< uchar >(((c & 0x7000) >> 7) | ((c & 0x7000) >> 10))};
    return CColor(col.r, col.g, col.b, col.a);
  }
}
#endif
#else
// Guessed name: target routine at 0x80320440 expands RGB5A3.
CColor CColor::FromRGB5A3(ushort value) {
  const ushort c = value;
  if (c & 0x8000) {
    const GXColor col = {static_cast< uchar >(((c & 0x7c00) >> 7) | ((c & 0x7000) >> 13)),
                         static_cast< uchar >(((c & 0x3e0) >> 2) | ((c & 0x380) >> 7)),
                         static_cast< uchar >(((c & 0x1f) << 3) | ((c & 0x1c) >> 2)), 0xff};
    return CColor(col.r, col.g, col.b, col.a);
  } else {
    const GXColor col = {static_cast< uchar >(((c & 0xf00) >> 4) | ((c & 0xf00) >> 8)),
                         static_cast< uchar >((c & 0xf0) | ((c & 0xf0) >> 4)),
                         static_cast< uchar >(((c & 0xf) << 4) | (c & 0xf)),
                         static_cast< uchar >(((c & 0x7000) >> 7) | ((c & 0x7000) >> 10))};
    return CColor(col.r, col.g, col.b, col.a);
  }
}

// Guessed name: target routine at 0x80320420 packs RGB565.
ushort CColor::ToRGB565() const {
  return ((mB & 0xf8) >> 3) | ((mG & 0xfc) << 3) | ((mR & 0xf8) << 8);
}
#endif

ushort CColor::ToRGB5A3() const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  uchar r = GetRedu8();
  uchar g = GetGreenu8();
  uchar b = GetBlueu8();
  uchar a = GetAlphau8();
#else
  const uchar r = GetRedu8();
  const uchar g = GetGreenu8();
  const uchar b = GetBlueu8();
  const uchar a = GetAlphau8();
#endif

  ushort ret = (1 << 15) | ((b & 0xf8) >> 3) | ((g & 0xf8) << 2) | ((r & 0xf8) << 7);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (a != 0xFF) {
#else
  if (a != 0xff)
#endif

    ret = ((a & 0xe0) << 7) | ((b & 0xf0) >> 4) | (g & 0xf0) | ((r & 0xf0) << 4);
#if RSTL_PRIME1
  }
#endif

  return ret;
}

