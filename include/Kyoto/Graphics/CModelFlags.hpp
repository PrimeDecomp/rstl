#ifndef _CMODELFLAGS
#define _CMODELFLAGS
#include "rstl/RstlVersions.h"

#include "types.h"

#include "Kyoto/Graphics/CColor.hpp"

class CModelFlags {
public:
  enum ETrans {
    kT_Opaque = 0,
    kT_One = 1,       // ?
    kT_Two = 2,       // ?
    kT_Three = 3,     // ?
    kT_ColorLerp = 3, // Guessed name; two-stage TEV color interpolation.
    kT_Four = 4,      // ?
    kT_Blend = 5,
    kT_Additive = 7,
    kT_Additive2 = 8,
  };
  enum EFlags {
    kF_Nothing = 0,
    kF_DepthCompare = 0x1,
    kF_DepthUpdate = 0x2,
    kF_NoTextureLock = 0x4,
    kF_DepthGreater = 0x8,
    kF_DepthNonInclusive = 0x10,
    kF_DrawNormal = 0x20,
    kF_ThermalUnsortedOnly = 0x40,
    kF_Unknown80 = 0x80,
    kF_Unknown100 = 0x100,
    kF_Unknown200 = 0x200,
    kF_Unknown400 = 0x400,
  };

  CModelFlags(RSTL_PRIME1_CONST ETrans trans, RSTL_PRIME1_CONST float rgba);
  CModelFlags(RSTL_PRIME1_CONST ETrans trans, const CColor& color)
  : mBlendMode(trans), mMatSetIdx(0), mFlags(kF_DepthCompare | kF_DepthUpdate), mColor(color) {}
  CModelFlags(const uchar blendMode, const uchar shadIdx, const uint flags, const CColor& col)
  : mBlendMode(blendMode), mMatSetIdx(shadIdx), mFlags(flags), mColor(col) {}
  CModelFlags(ETrans blendMode, uchar shadIdx, EFlags flags, const CColor& col)
  : mBlendMode(blendMode), mMatSetIdx(shadIdx), mFlags(flags), mColor(col) {}
  CModelFlags(const CModelFlags& flags, RSTL_PRIME1_CONST uint otherFlags)
  : mBlendMode(flags.mBlendMode)
  , mMatSetIdx(flags.mMatSetIdx)
  , mFlags(otherFlags)
  , mColor(flags.mColor) {}
  // Guessed: a second flag-copying constructor that reads the shader set through its getter,
  // used by AddFlags.
  CModelFlags(uint otherFlags, const CModelFlags& flags)
  : mBlendMode(flags.mBlendMode)
  , mMatSetIdx(flags.GetMatSetIdx())
  , mFlags(otherFlags)
  , mColor(flags.mColor) {}
  CModelFlags(const CModelFlags& flags, RSTL_PRIME1_CONST bool b /* TODO what's this? */,
              RSTL_PRIME1_CONST int shaderSet)
  : mBlendMode(flags.GetBlendMode())
  , mMatSetIdx(shaderSet)
#if RSTL_PRIME1
  , mFlags(flags.mFlags)
#else
  , mFlags(flags.GetOtherFlags())
#endif
  , mColor(flags.mColor) {
  }
  CModelFlags(const CModelFlags& flags, ETrans trans, CColor color)
  : mBlendMode(trans), mMatSetIdx(flags.mMatSetIdx), mFlags(flags.mFlags), mColor(color) {}

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
  // Guessed name for the additional flag operation used by GUI model passes.
  CModelFlags WithAdditionalFlags(uint flags) const {
    return CModelFlags(*this, GetOtherFlags() | flags);
  }
  // Guessed name
  CModelFlags AddFlags(uint flags) const { return CModelFlags(GetOtherFlags() | flags, *this); }

  CModelFlags DepthCompareUpdate(const bool compare, const bool update) const {
    const uint newFlags = static_cast< uint >(compare) | (static_cast< uint >(update) << 1);
    return CModelFlags(*this, (mFlags & ~(kF_DepthCompare | kF_DepthUpdate)) | newFlags);
  }
  CModelFlags DepthBackwards() const {
#if RSTL_PRIME1
    const uint flags = GetOtherFlags() | kF_DepthGreater;
    return CModelFlags(GetTrans(), mMatSetIdx, flags, mColor);
#elif RSTL_KYOTO_MODEL_FLAGS_DEPTH_BACKWARDS_SET_ONLY
    return CModelFlags(*this, GetOtherFlags() | kF_DepthGreater);
#else
    return CModelFlags(*this, (GetOtherFlags() & ~kF_Unknown100) | kF_DepthGreater | kF_Unknown200);
#endif
  }
  // Guessed name; the counterpart of DepthBackwards used when drawing the sky ripple.
  CModelFlags DepthForwards() const {
    return CModelFlags((GetOtherFlags() & ~(kF_DepthGreater | kF_DepthNonInclusive)) |
                           kF_Unknown100 | kF_Unknown200,
                       *this);
  }
  CModelFlags DrawNormal(bool draw) const {
    return CModelFlags(*this, draw ? mFlags | kF_DrawNormal : mFlags & ~kF_DrawNormal);
  }

  const uchar GetBlendMode() const { return mBlendMode; }
  RSTL_PRIME1_CONST ETrans GetTrans() const {
#if RSTL_PRIME1
    return static_cast< ETrans >(mBlendModeChar);
#else
    return static_cast< ETrans >(mBlendMode);
#endif
  }
  // Reads the blend mode as a signed byte (lbz + extsb into one register); a value cast to
  // signed char splits that into two registers instead.
  ETrans GetTransSigned() const {
    return static_cast< ETrans >(*reinterpret_cast< const signed char* >(&mBlendMode));
  }
  // The shader set is stored as an unsigned byte but read back sign-extended, in one register.
  RSTL_PRIME1_CONST int GetShaderSet() const {
    return *reinterpret_cast< const signed char* >(&mMatSetIdx);
  }
  const uchar GetMatSetIdx() const { return mMatSetIdx; } // Guessed name
  RSTL_PRIME1_CONST CColor GetColor() const { return mColor; }
#if RSTL_PRIME1
  const int GetOtherFlags() const { return mFlags; }
#else
  const ushort GetOtherFlags() const { return mFlags; }
#endif

  const CColor& GetColorRef() const { return mColor; }
  void SetColor(const CColor& color) { mColor = color; }

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
  static CModelFlags AlphaBlendedDepthCompareUpdate(const float alpha, const bool compare,
                                                    const bool update) {
    return AlphaBlended(alpha).DepthCompareUpdate(compare, update);
  }
  static CModelFlags AlphaBlendedDepthCompareUpdate(const CColor& color, const bool compare,
                                                    const bool update) {
    return AlphaBlended(color).DepthCompareUpdate(compare, update);
  }
  static CModelFlags Additive(RSTL_PRIME1_CONST float f) { return CModelFlags(kT_Additive, f); }
  static CModelFlags Additive(const CColor& color) { return CModelFlags(kT_Additive, color); }
  static CModelFlags AdditiveDepthCompareUpdate(const CColor& color, const bool compare,
                                                const bool update) {
    return Additive(color).DepthCompareUpdate(compare, update);
  }
  static CModelFlags AdditiveRGB(const CColor& color) { return CModelFlags(kT_Additive2, color); }
  static CModelFlags ColorModulate(const CColor& color) { return CModelFlags(kT_One, color); }

#if RSTL_PRIME1
  union {
    uchar mBlendMode;
    char mBlendModeChar; // Read by GetTrans.
  };
  mutable char mMatSetIdx;
#else
  // Unknown; copied as one word.
  union {
    uchar x0_[4];
    uint x0w_;
  };
  uchar mBlendMode;
  uchar mMatSetIdx;
#endif
  ushort mFlags;
  CColor mColor;
};
CHECK_SIZEOF(CModelFlags, RSTL_PRIME1 ? 0x8 : 0xc)

#if !RSTL_PRIME1 && defined(__MWERKS__)
#pragma cpp_extensions on
#endif
inline CModelFlags::CModelFlags(RSTL_PRIME1_CONST ETrans trans, RSTL_PRIME1_CONST float rgba)
: mBlendMode(trans)
, mMatSetIdx(0)
, mFlags(kF_DepthCompare | kF_DepthUpdate)
, mColor(1.f, 1.f, 1.f, rgba) {}
#if !RSTL_PRIME1 && defined(__MWERKS__)
#pragma cpp_extensions reset
#endif

#endif // _CMODELFLAGS
