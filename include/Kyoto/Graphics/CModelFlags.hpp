#ifndef _CMODELFLAGS
#define _CMODELFLAGS
#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"
#endif

#if RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#endif
#include "rstl/types.h"

#include "Kyoto/Graphics/CColor.hpp"

class CModelFlags {
public:
  enum ETrans {
    kT_Opaque = 0,
    kT_One = 1,       // ?
    kT_Two = 2,       // ?
#if !RSTL_CORRUPTION
    kT_Three = 3,     // ?
#endif
    kT_ColorLerp = 3, // Guessed name; two-stage TEV color interpolation.
#if RSTL_CORRUPTION
    kT_ThresholdAlpha = 4, // Guessed name
    kT_Unknown5 = 5,
    kT_Blend = 6,
    kT_Additive = 8,
    kT_Additive2 = 9,
#else
    kT_Four = 4,      // ?
    kT_Blend = 5,
    kT_Additive = 7,
    kT_Additive2 = 8,
#endif
  };
  enum EFlags {
#if RSTL_CORRUPTION
    kF_NoDepthCompare = 0x1,
    kF_NoDepthUpdate = 0x2,
#else
    kF_Nothing = 0,
    kF_DepthCompare = 0x1,
    kF_DepthUpdate = 0x2,
#endif
    kF_NoTextureLock = 0x4,
    kF_DepthGreater = 0x8,
    kF_DepthNonInclusive = 0x10,
    kF_DrawNormal = 0x20,
    kF_ThermalUnsortedOnly = 0x40,
    kF_Unknown80 = 0x80,
    kF_Unknown100 = 0x100,
    kF_Unknown200 = 0x200,
    kF_Unknown400 = 0x400,
#if RSTL_CORRUPTION
    kF_ForceDepthUpdate = 0x2000, // Guessed name
    kF_Unknown4000 = 0x4000,
#endif
  };

#if RSTL_CORRUPTION
  CModelFlags(ETrans trans, float rgba)
  : mBlendMode(trans)
  , mMatSetIdx(0)
  , mFlags(0)
  , mDisabledVisibility(0)
  , mEnabledVisibility(0)
  , mColor(1.f, 1.f, 1.f, rgba) {}
  CModelFlags(ETrans trans, const CColor& color)
  : mBlendMode(trans)
  , mMatSetIdx(0)
  , mFlags(0)
  , mDisabledVisibility(0)
  , mEnabledVisibility(0)
  , mColor(color) {}

#else
  CModelFlags(RSTL_PRIME1_CONST ETrans trans, RSTL_PRIME1_CONST float rgba);
  CModelFlags(RSTL_PRIME1_CONST ETrans trans, const CColor& color)
  : mBlendMode(trans), mMatSetIdx(0), mFlags(kF_DepthCompare | kF_DepthUpdate), mColor(color) {}
  CModelFlags(const uchar blendMode, const uchar shadIdx, const uint flags, const CColor& col)
  : mBlendMode(blendMode), mMatSetIdx(shadIdx), mFlags(flags), mColor(col) {}
#endif
  CModelFlags(ETrans blendMode, uchar shadIdx, EFlags flags, const CColor& col)
#if RSTL_CORRUPTION
  : mBlendMode(blendMode)
  , mMatSetIdx(shadIdx)
  , mFlags(flags)
  , mDisabledVisibility(0)
  , mEnabledVisibility(0)
  , mColor(col) {}

  CModelFlags(const CModelFlags& flags, uint otherFlags)
#else
  : mBlendMode(blendMode), mMatSetIdx(shadIdx), mFlags(flags), mColor(col) {}
  CModelFlags(const CModelFlags& flags, RSTL_PRIME1_CONST uint otherFlags)
#endif
  : mBlendMode(flags.mBlendMode)
  , mMatSetIdx(flags.mMatSetIdx)
  , mFlags(otherFlags)
#if RSTL_CORRUPTION
  , mDisabledVisibility(flags.mDisabledVisibility)
  , mEnabledVisibility(flags.mEnabledVisibility)
  , mColor(flags.mColor) {}
  CModelFlags(const CModelFlags& flags, bool b /* TODO what's this? */, int shaderSet)
  : mBlendMode(flags.mBlendMode)
  , mMatSetIdx(shaderSet)
  , mFlags(flags.mFlags)
  , mDisabledVisibility(flags.mDisabledVisibility)
  , mEnabledVisibility(flags.mEnabledVisibility)
#endif
  , mColor(flags.mColor) {}
  // Guessed: a second flag-copying constructor that reads the shader set through its getter,
  // used by AddFlags.
#if !RSTL_CORRUPTION
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
#endif
  CModelFlags(const CModelFlags& flags, ETrans trans, CColor color)
#if RSTL_CORRUPTION
  : mBlendMode(trans)
  , mMatSetIdx(flags.mMatSetIdx)
  , mFlags(flags.mFlags)
  , mDisabledVisibility(flags.mDisabledVisibility)
  , mEnabledVisibility(flags.mEnabledVisibility)
  , mColor(color) {}
#else
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
#endif

  CModelFlags UseShaderSet(int matSet) const { return CModelFlags(*this, false, matSet); }
#if !RSTL_CORRUPTION
  CModelFlags DontLoadTextures() const {
    return CModelFlags(*this, GetOtherFlags() | kF_NoTextureLock);
  }
#endif
  // Guessed name for the additional flag operation used by GUI model passes.
  CModelFlags WithAdditionalFlags(uint flags) const {
    return CModelFlags(*this, GetOtherFlags() | flags);
  }
  // Guessed name
#if RSTL_CORRUPTION
  CModelFlags DisableVisibility(int index) const {
    return CModelFlags(*this, mEnabledVisibility,
                       static_cast< ushort >(mDisabledVisibility | (1 << index)));
#else
  CModelFlags AddFlags(uint flags) const { return CModelFlags(GetOtherFlags() | flags, *this); }

  CModelFlags DepthCompareUpdate(const bool compare, const bool update) const {
    const uint newFlags = static_cast< uint >(compare) | (static_cast< uint >(update) << 1);
    return CModelFlags(*this, (mFlags & ~(kF_DepthCompare | kF_DepthUpdate)) | newFlags);
#endif
  }
#if RSTL_CORRUPTION
  CModelFlags DontLoadTextures() const {
    return CModelFlags(*this, GetOtherFlags() | kF_NoTextureLock);
#else
  CModelFlags DepthBackwards() const {
#if RSTL_PRIME1
    const uint flags = GetOtherFlags() | kF_DepthGreater;
    return CModelFlags(GetTrans(), mMatSetIdx, flags, mColor);
#elif RSTL_KYOTO_MODEL_FLAGS_DEPTH_BACKWARDS_SET_ONLY
    return CModelFlags(*this, GetOtherFlags() | kF_DepthGreater);
#else
    return CModelFlags(*this, (GetOtherFlags() & ~kF_Unknown100) | kF_DepthGreater | kF_Unknown200);
#endif
#endif
  }
  // Guessed name; the counterpart of DepthBackwards used when drawing the sky ripple.
#if RSTL_CORRUPTION
  CModelFlags DepthCompare(bool compare) const {
    return CModelFlags(*this,
                       compare ? (mFlags & ~kF_NoDepthCompare) : (mFlags | kF_NoDepthCompare));
#else
  CModelFlags DepthForwards() const {
    return CModelFlags((GetOtherFlags() & ~(kF_DepthGreater | kF_DepthNonInclusive)) |
                           kF_Unknown100 | kF_Unknown200,
                       *this);
#endif
  }
#if RSTL_CORRUPTION
  CModelFlags DepthUpdate(bool update) const {
    return CModelFlags(*this, update ? ((mFlags & ~kF_NoDepthUpdate) | kF_ForceDepthUpdate)
                                     : ((mFlags & ~kF_ForceDepthUpdate) | kF_NoDepthUpdate));
#else
  CModelFlags DrawNormal(bool draw) const {
    return CModelFlags(*this, draw ? mFlags | kF_DrawNormal : mFlags & ~kF_DrawNormal);
#endif
  }
  // Guessed name; enabled values retain the existing depth behavior.
#if RSTL_CORRUPTION
  CModelFlags DepthCompareUpdate(bool compare, bool update) const {
    CModelFlags flags = *this;
    if (!compare) {
      flags = flags.DepthCompare(false);
    }
    if (!update) {
      flags = flags.DepthUpdate(false);
    }
    return flags;
  }
  CModelFlags DepthBackwards() const {
    return CModelFlags(
        *this, (GetOtherFlags() & ~(kF_NoDepthCompare | kF_DepthNonInclusive | kF_Unknown100)) |
                   kF_DepthGreater | kF_Unknown200);
#else
  const uchar GetBlendMode() const { return mBlendMode; }
  RSTL_PRIME1_CONST ETrans GetTrans() const {
#if RSTL_PRIME1
    return static_cast< ETrans >(mBlendModeChar);
#else
    return static_cast< ETrans >(mBlendMode);
#endif
#endif
  }
  // Reads the blend mode as a signed byte (lbz + extsb into one register); a value cast to
  // Guessed name
#if RSTL_CORRUPTION
  CModelFlags DepthForwards() const {
    return CModelFlags(
        *this, (GetOtherFlags() & ~(kF_NoDepthCompare | kF_DepthGreater | kF_DepthNonInclusive)) |
                   kF_Unknown100 | kF_Unknown200);
#else
  ETrans GetTransSigned() const {
    return static_cast< ETrans >(*reinterpret_cast< const signed char* >(&mBlendMode));
#endif
  }
  // The shader set is stored as an unsigned byte but read back sign-extended, in one register.
#if RSTL_CORRUPTION
  ETrans GetTrans() const { return static_cast< ETrans >(mBlendMode); }
  int GetShaderSet() const { return static_cast< signed char >(mMatSetIdx); }
  uint GetOtherFlags() const { return mFlags; }
  uint GetDisabledVisibility() const { return mDisabledVisibility; } // Guessed name
  uint GetEnabledVisibility() const { return mEnabledVisibility; }   // Guessed name
  CColor GetColor() const { return mColor; }
#else
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

#endif
  const CColor& GetColorRef() const { return mColor; }
  void SetColor(const CColor& color) { mColor = color; } // Guessed name

  bool operator==(const CModelFlags& other) const {
    // TODO: cast to char for extsb; see CScriptActor::PreRender
    return static_cast< char >(mBlendMode) == static_cast< char >(other.mBlendMode) &&
           static_cast< char >(mMatSetIdx) == static_cast< char >(other.mMatSetIdx) &&
           mFlags == other.mFlags && mColor == other.mColor;
  }

  static CModelFlags Normal() { return CModelFlags(kT_Opaque, 1.f); }
#if RSTL_CORRUPTION
  static CModelFlags AlphaBlended(float alpha) { return CModelFlags(kT_Blend, alpha); }
#else
  static CModelFlags AlphaBlended(RSTL_PRIME1_CONST float alpha) {
    return CModelFlags(kT_Blend, alpha);
  }
#endif
  static CModelFlags AlphaBlended(const CColor& color) { return CModelFlags(kT_Blend, color); }
#if RSTL_CORRUPTION
  static CModelFlags Additive(float f) { return CModelFlags(CModelFlags::kT_Additive, f); }
  static CModelFlags Additive(const CColor& color) {
    return CModelFlags(CModelFlags::kT_Additive, color);
#else
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
#endif
  }
  static CModelFlags AdditiveRGB(const CColor& color) { return CModelFlags(kT_Additive2, color); }
  static CModelFlags ColorModulate(const CColor& color) { return CModelFlags(kT_One, color); }

#if RSTL_CORRUPTION
private:
  CModelFlags(const CModelFlags& flags, ushort enabledVisibility, ushort disabledVisibility)
  : mBlendMode(flags.mBlendMode)
  , mMatSetIdx(flags.mMatSetIdx)
  , mFlags(flags.mFlags)
  , mDisabledVisibility(disabledVisibility)
  , mEnabledVisibility(enabledVisibility)
  , mColor(flags.mColor) {}
  // Unknown; copied as one word.
    uchar x0_[4];
  uchar mBlendMode;
  uchar mMatSetIdx;
#else
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
#endif
  ushort mFlags;
#if RSTL_CORRUPTION
  ushort mDisabledVisibility; // Guessed name
  ushort mEnabledVisibility;  // Guessed name
#endif
  CColor mColor;
};
#if RSTL_CORRUPTION
CHECK_SIZEOF(CModelFlags, 0x10)
#else
CHECK_SIZEOF(CModelFlags, RSTL_PRIME1 ? 0x8 : 0xc)
#endif

#if !RSTL_CORRUPTION
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
#endif

#endif // _CMODELFLAGS
