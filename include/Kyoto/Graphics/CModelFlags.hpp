#ifndef _CMODELFLAGS
#define _CMODELFLAGS
#include "rstl/RstlVersions.h"

#include "types.h"

#include "Kyoto/Graphics/CColor.hpp"

class CModelFlags {
public:
  enum ETrans {
    kT_Opaque = 0,
    kT_One = 1,   // ?
    kT_Two = 2,   // ?
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    kT_Three = 3, // ?
    kT_Four = 4,  // ?
#else
    kT_ColorLerp = 3, // Guessed name; native two-stage TEV color interpolation.
#endif

    kT_Blend = 5,
    kT_Additive = 7,
    kT_Additive2 = 8,
  };
  enum EFlags {
#if RSTL_PRIME1
    kF_Nothing = 0,
#endif

    kF_DepthCompare = 0x1,
    kF_DepthUpdate = 0x2,
    kF_NoTextureLock = 0x4,
    kF_DepthGreater = 0x8,
    kF_DepthNonInclusive = 0x10,
    kF_DrawNormal = 0x20,
    kF_ThermalUnsortedOnly = 0x40,
#if !RSTL_PRIME1
    kF_Unknown80 = 0x80,
    kF_Unknown100 = 0x100,
    kF_Unknown200 = 0x200,
    kF_Unknown400 = 0x400,
#endif

  };

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CModelFlags(const ETrans trans, const float rgba)
  : mBlendMode(trans)
  , mMatSetIdx(0)
  , mFlags(kF_DepthCompare | kF_DepthUpdate)
  , mColor(1.f, 1.f, 1.f, rgba) {}
#else
  CModelFlags(ETrans trans, float rgba);
#endif
  CModelFlags(RSTL_PRIME1_CONST ETrans trans, const CColor& color)
  : mBlendMode(trans)
  , mMatSetIdx(0)
  , mFlags(kF_DepthCompare | kF_DepthUpdate)
  , mColor(color) {}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CModelFlags(const uchar blendMode, const uchar shadIdx, const uint flags, const CColor& col)
#else
  CModelFlags(ETrans blendMode, uchar shadIdx, EFlags flags, const CColor& col)
#endif

  : mBlendMode(blendMode), mMatSetIdx(shadIdx), mFlags(flags), mColor(col) {}

  CModelFlags(const CModelFlags& flags, RSTL_PRIME1_CONST uint otherFlags)

  : mBlendMode(flags.mBlendMode)
  , mMatSetIdx(flags.mMatSetIdx)
  , mFlags(otherFlags)
  , mColor(flags.mColor) {}
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
  // Guessed: a second flag-copying constructor that reads the shader set through its getter,
  // used by AddFlags.
  CModelFlags(uint otherFlags, const CModelFlags& flags)
  : mBlendMode(flags.mBlendMode)
  , mMatSetIdx(flags.GetMatSetIdx())
  , mFlags(otherFlags)
  , mColor(flags.mColor) {}
#endif
  CModelFlags(const CModelFlags& flags, RSTL_PRIME1_CONST bool b /* TODO what's this? */,
              RSTL_PRIME1_CONST int shaderSet)
  : mBlendMode(flags.GetBlendMode())
  , mMatSetIdx(shaderSet)
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  , mFlags(flags.mFlags)
#else
  , mFlags(flags.GetOtherFlags())
#endif

  , mColor(flags.mColor) {}

#if !RSTL_PRIME1
  // ?
  CModelFlags(const CModelFlags& flags, ETrans trans, CColor color)
  : mBlendMode(trans), mMatSetIdx(flags.mMatSetIdx), mFlags(flags.mFlags), mColor(color) {}
#endif

  // CModelFlags(const CModelFlags& other)
  // : x0_blendMode(other.x0_blendMode)
  // , x1_matSetIdx(other.x1_matSetIdx)
  // , x2_flags(other.x2_flags)
  // , x4_color(other.x4_color) {}
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CModelFlags& operator=(const CModelFlags& other) {
    mBlendMode = other.GetBlendMode();
    mMatSetIdx = other.mMatSetIdx;
    mFlags = other.GetOtherFlags();
    mColor = other.GetColor();
    return *this;
  }
#endif

  CModelFlags UseShaderSet(int matSet) const { return CModelFlags(*this, false, matSet); }
  CModelFlags DontLoadTextures() const {
    return CModelFlags(*this, GetOtherFlags() | kF_NoTextureLock);
  }
#if !RSTL_PRIME1
  // Guessed name for the additional flag operation used by GUI model passes.
  CModelFlags WithAdditionalFlags(uint flags) const {
    return CModelFlags(*this, GetOtherFlags() | flags);
  }
  // Guessed name
  CModelFlags AddFlags(uint flags) const { return CModelFlags(GetOtherFlags() | flags, *this); }
#endif

  CModelFlags DepthCompareUpdate(const bool compare, const bool update) const {
    const uint newFlags = static_cast< uint >(compare) | (static_cast< uint >(update) << 1);
    return CModelFlags(*this, (mFlags & ~(kF_DepthCompare | kF_DepthUpdate)) | newFlags);
  }
  CModelFlags DepthBackwards() const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    const uint flags = GetOtherFlags() | kF_DepthGreater;
    return CModelFlags(GetTrans(), mMatSetIdx, flags, mColor);
#else
    return CModelFlags(*this, (GetOtherFlags() & ~kF_Unknown100) | kF_DepthGreater | kF_Unknown200);
#endif

  }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CModelFlags DrawNormal(bool draw) const {
    return CModelFlags(*this, draw ? mFlags | kF_DrawNormal : mFlags & ~kF_DrawNormal);
#else
  // Guessed name; the counterpart of DepthBackwards used when drawing the sky ripple.
  CModelFlags DepthForwards() const {
    return CModelFlags(*this, (GetOtherFlags() & ~(kF_DepthGreater | kF_DepthNonInclusive)) |
                                  kF_Unknown100 | kF_Unknown200);
#endif

  }

  const uchar GetBlendMode() const { return mBlendMode; }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const ETrans GetTrans() const { return static_cast< ETrans >(mBlendModeChar); }
  const int GetShaderSet() const { return mMatSetIdx; }
  const int GetOtherFlags() const { return mFlags; }
  const CColor GetColor() const { return mColor; }
#else
  ETrans GetTrans() const { return static_cast< ETrans >(mBlendMode); }
  // Some native paths read the blend mode as a signed byte (lbz + extsb into one register);
  // a value cast to signed char splits that into two registers instead.
  ETrans GetTransSigned() const {
    return static_cast< ETrans >(*reinterpret_cast< const signed char* >(&mBlendMode));
  }
  // The shader set is copied as an unsigned byte but read back sign-extended, in one register.
  int GetShaderSet() const { return *reinterpret_cast< const signed char* >(&mMatSetIdx); }
  const ushort GetOtherFlags() const { return mFlags; }
  const uchar GetMatSetIdx() const { return mMatSetIdx; } // Guessed name
  CColor GetColor() const { return mColor; }
#endif

  const CColor& GetColorRef() const { return mColor; }
#if !RSTL_PRIME1
  void SetColor(const CColor& color) { mColor = color; }
#endif

  bool operator==(const CModelFlags& other) const {
    // TODO: cast to char for extsb; see CScriptActor::PreRender
    return static_cast< char >(mBlendMode) == static_cast< char >(other.mBlendMode) &&
           static_cast< char >(mMatSetIdx) == static_cast< char >(other.mMatSetIdx) &&
           mFlags == other.mFlags && mColor == other.mColor;
  }

  static CModelFlags Normal() { return CModelFlags(kT_Opaque, 1.f); }
  static CModelFlags AlphaBlended(RSTL_PRIME1_CONST float alpha) {
    return CModelFlags(kT_Blend, alpha);
  }

  static CModelFlags AlphaBlended(const CColor& color) { return CModelFlags(kT_Blend, color); }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static CModelFlags AlphaBlendedDepthCompareUpdate(const float alpha, const bool compare,
                                                    const bool update) {
    return AlphaBlended(alpha).DepthCompareUpdate(compare, update);
  }
  static CModelFlags AlphaBlendedDepthCompareUpdate(const CColor& color, const bool compare,
                                                    const bool update) {
    return AlphaBlended(color).DepthCompareUpdate(compare, update);
  }
#endif
  static CModelFlags Additive(RSTL_PRIME1_CONST float f) { return CModelFlags(kT_Additive, f); }
  static CModelFlags Additive(const CColor& color) { return CModelFlags(kT_Additive, color); }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static CModelFlags AdditiveDepthCompareUpdate(const CColor& color, const bool compare,
                                                const bool update) {
    return Additive(color).DepthCompareUpdate(compare, update);
  }
#endif
  static CModelFlags AdditiveRGB(const CColor& color) { return CModelFlags(kT_Additive2, color); }
  static CModelFlags ColorModulate(const CColor& color) { return CModelFlags(kT_One, color); }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
public:
  // Dumb hack, need to figure this out
  union {
#else
private:
  // Unknown. No constructor initializes it, and copies move it as one 4-byte aggregate
  // (CScriptColorModulate::End copies it from uninitialized stack).
  uchar x0_[4];
#endif

    uchar mBlendMode;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    char mBlendModeChar;
  };
  mutable char mMatSetIdx;
#else
  uchar mMatSetIdx;
#endif

  ushort mFlags;
  CColor mColor;
};
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CHECK_SIZEOF(CModelFlags, 0x8)
#else
CHECK_SIZEOF(CModelFlags, 0xc)

#ifdef __MWERKS__
#pragma cpp_extensions on
#endif
inline CModelFlags::CModelFlags(ETrans trans, float rgba)
: mBlendMode(trans)
, mMatSetIdx(0)
, mFlags(kF_DepthCompare | kF_DepthUpdate)
, mColor(1.f, 1.f, 1.f, rgba) {}
#ifdef __MWERKS__
#pragma cpp_extensions reset
#endif
#endif

#endif // _CMODELFLAGS
