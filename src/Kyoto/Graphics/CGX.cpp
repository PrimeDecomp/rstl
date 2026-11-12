#include "rstl/RstlVersions.h"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CTexture.hpp"

#include "Kyoto/Alloc/Assert.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#if RSTL_PRIME1
#include "dolphin/gx/GXManage.h"
#include "dolphin/gx/GXTexture.h"
#endif

#include "limits.h"

#if !RSTL_PRIME1 && !RSTL_KYOTO_CGX_CORRUPTION
#ifndef TARGET_PC
struct GXData {
  ushort cpSRreg;
  ushort cpCRreg;
};
extern GXData* __GXData;
#endif
#endif

#if !RSTL_CGX_STATE_POINTER
CGX::SGXState CGX::sGXState;
#else
static CGX::SGXState sGXState; // Guessed scope: TU-local state behind gpGXState.
CGX::SGXState* CGX::gpGXState = &sGXState;
#endif

#if !RSTL_PRIME1
static const uint sAlwaysAlphaCompare = CGX::MaskAndShiftLeft(GX_ALWAYS, 7, 0) |
                                        CGX::MaskAndShiftLeft(GX_AOP_OR, 7, 11) |
                                        CGX::MaskAndShiftLeft(GX_ALWAYS, 7, 14);
#endif

#if NONMATCHING
// Doesn't need to be so big
static GXVtxDescList sVtxDescList[12];
#else
static GXVtxDescList sVtxDescList[GX_MAX_VTXDESCLIST_SZ];
#endif

#if RSTL_PRIME1
#define CGX_INLINE
#define CGX_CPP
#elif !RSTL_KYOTO_CGX_CORRUPTION
inline void CGX::apply_fog() {
  static const GXColor black = {0, 0, 0, 0};
  GXSetFog(static_cast< GXFogType >(CGX_STATE.mFogType), CGX_STATE.mFogParams.mFogStartZ,
           CGX_STATE.mFogParams.mFogEndZ, CGX_STATE.mFogParams.mFogNearZ,
           CGX_STATE.mFogParams.mFogFarZ,
           (CGX_STATE.mBlendMode & (7 << 5)) == (GX_BL_ONE << 5) ? black
                                                                 : CGX_STATE.mFogParams.mFogColor);
}
#endif

#if !RSTL_PRIME1
void CGX::SetNumChans(uchar num) {
  CGX_STATE.mNumChans = num;
  CGX_STATE.mFlags.numDirty = num != CGX_STATE.mPrevNumChans;
}
#endif

#if RSTL_PRIME1
#include "Kyoto/Graphics/CGX_Impl.hpp" // IWYU pragma: keep
#else
void CGX::SetNumTexGens(uchar num) {
  if (num != CGX_STATE.mNumTexGens) {
    CGX_STATE.mNumTexGens = num;
    GXSetNumTexGens(num);
  }
}
#endif

#if !RSTL_PRIME1
void CGX::SetChanAmbColor(EChannelId channel, const GXColor& color) {
  if (!CompareGXColors(color, CGX_STATE.mChanAmbColors[channel])) {
    CopyGXColor(CGX_STATE.mChanAmbColors[channel], color);
    GXSetChanAmbColor(static_cast< GXChannelID >(GX_COLOR0A0 + channel), color);
  }
}

GXColor CGX::GetChanAmbColor(EChannelId channel) {
  const GXColor color = CGX_STATE.mChanAmbColors[channel];
  return color;
}

void CGX::SetChanMatColor(EChannelId channel, const GXColor& color) {
  if (!CompareGXColors(color, CGX_STATE.mChanMatColors[channel])) {
    CopyGXColor(CGX_STATE.mChanMatColors[channel], color);
    GXSetChanMatColor(static_cast< GXChannelID >(GX_COLOR0A0 + channel), color);
  }
}

void CGX::SetChanCtrl(EChannelId channel, GXBool enable, GXColorSrc ambSrc, GXColorSrc matSrc,
                      GXLightID lights, GXDiffuseFn diffFn, GXAttnFn attnFn) {
  ushort& state = CGX_STATE.mChanCtrls[channel];
  ushort prevFlags = CGX_STATE.mPrevChanCtrls[channel];
  if (lights == GX_LIGHT_NULL) {
    enable = GX_FALSE;
  }
  uint flags = MaskAndShiftLeft(enable, 1, 0) | MaskAndShiftLeft(ambSrc, 1, 1) |
               MaskAndShiftLeft(matSrc, 1, 2) | MaskAndShiftLeft(lights, 0xFF, 3) |
               MaskAndShiftLeft(diffFn, 3, 11) | MaskAndShiftLeft(attnFn, 3, 13);
  state = flags;
  CGX_STATE.mChanFlags =
      ((flags != prevFlags) << (channel + 1)) | (CGX_STATE.mChanFlags & ~(1 << (channel + 1)));
}

void CGX::SetChanCtrl_Compressed(EChannelId channel, GXLightID lights, uint ctrl) {
  ushort& state = CGX_STATE.mChanCtrls[channel];
  ushort prevFlags = CGX_STATE.mPrevChanCtrls[channel];
  uint flags = ctrl & ~1;
  if (lights != GX_LIGHT_NULL) {
    flags = ctrl | MaskAndShiftLeft(lights, 0xFF, 3);
  }
  state = flags;
  CGX_STATE.mChanFlags =
      ((flags != prevFlags) << (channel + 1)) | (CGX_STATE.mChanFlags & ~(1 << (channel + 1)));
}

void CGX::SetNumTevStages(uchar num) {
  if (CGX_STATE.mNumTevStages != num) {
    CGX_STATE.mNumTevStages = num;
    GXSetNumTevStages(num);
  }
}

void CGX::SetTevKColor(GXTevKColorID id, const GXColor& color) {
  if (!CompareGXColors(CGX_STATE.mKColors[id], color)) {
    CopyGXColor(CGX_STATE.mKColors[id], color);
    GXSetTevKColor(id, color);
  }
}

#if RSTL_KYOTO_CGX_CORRUPTION
void CGX::SetTevSwapMode(GXTevStageID stage, GXTevSwapSel rasSel, GXTevSwapSel texSel) {
  STevState& state = CGX_STATE.mTevStates[stage];
  const uchar flags = rasSel | (texSel << 2);
  if (flags != state.mSwapMode) {
    GXSetTevSwapMode(stage, rasSel, texSel);
    state.mSwapMode = flags;
  }
}

void CGX::SetTevSwapModeTable(GXTevSwapSel table, GXTevColorChan red, GXTevColorChan green,
                              GXTevColorChan blue, GXTevColorChan alpha) {
  const uchar flags = red | (green << 2) | (blue << 4) | (alpha << 6);
  if (flags != CGX_STATE.mTevSwapTables[table]) {
    GXSetTevSwapModeTable(table, red, green, blue, alpha);
    CGX_STATE.mTevSwapTables[table] = flags;
  }
}
#endif

void CGX::SetTevColorIn(GXTevStageID stageId, GXTevColorArg a, GXTevColorArg b, GXTevColorArg c,
                        GXTevColorArg d) {
  uint flags = MaskAndShiftLeft(a, 0x1F, 0) | MaskAndShiftLeft(b, 0x1F, 5) |
               MaskAndShiftLeft(c, 0x1F, 10) | MaskAndShiftLeft(d, 0x1F, 15);
  STevState& state = CGX_STATE.mTevStates[stageId];
  if (flags != state.mColorInArgs) {
    state.mColorInArgs = flags;
    GXSetTevColorIn(stageId, a, b, c, d);
  }
}

void CGX::SetTevColorIn_Compressed(GXTevStageID stageId, uint flags) {
  STevState& state = CGX_STATE.mTevStates[stageId];
  if (flags != state.mColorInArgs) {
    state.mColorInArgs = flags;
    GXSetTevColorIn(stageId, static_cast< GXTevColorArg >(ShiftRightAndMask(flags, 0x1F, 0)),
                    static_cast< GXTevColorArg >(ShiftRightAndMask(flags, 0x1F, 5)),
                    static_cast< GXTevColorArg >(ShiftRightAndMask(flags, 0x1F, 10)),
                    static_cast< GXTevColorArg >(ShiftRightAndMask(flags, 0x1F, 15)));
  }
}

void CGX::SetTevAlphaIn(GXTevStageID stageId, GXTevAlphaArg a, GXTevAlphaArg b, GXTevAlphaArg c,
                        GXTevAlphaArg d) {
  uint flags = MaskAndShiftLeft(a, 0x1F, 0) | MaskAndShiftLeft(b, 0x1F, 5) |
               MaskAndShiftLeft(c, 0x1F, 10) | MaskAndShiftLeft(d, 0x1F, 15);
  STevState& state = CGX_STATE.mTevStates[stageId];
  if (flags != state.mAlphaInArgs) {
    state.mAlphaInArgs = flags;
    GXSetTevAlphaIn(stageId, a, b, c, d);
  }
}

void CGX::SetTevAlphaIn_Compressed(GXTevStageID stageId, uint flags) {
  STevState& state = CGX_STATE.mTevStates[stageId];
  if (flags != state.mAlphaInArgs) {
    state.mAlphaInArgs = flags;
    GXSetTevAlphaIn(stageId, static_cast< GXTevAlphaArg >(ShiftRightAndMask(flags, 0x1F, 0)),
                    static_cast< GXTevAlphaArg >(ShiftRightAndMask(flags, 0x1F, 5)),
                    static_cast< GXTevAlphaArg >(ShiftRightAndMask(flags, 0x1F, 10)),
                    static_cast< GXTevAlphaArg >(ShiftRightAndMask(flags, 0x1F, 15)));
  }
}

void CGX::SetTevColorOp(GXTevStageID stageId, GXTevOp op, GXTevBias bias, GXTevScale scale,
                        GXBool clamp, GXTevRegID outReg) {
  uint flags = MaskAndShiftLeft(op, 0xF, 0) | MaskAndShiftLeft(bias, 3, 4) |
               MaskAndShiftLeft(scale, 3, 6) | MaskAndShiftLeft(clamp, 1, 8) |
               MaskAndShiftLeft(outReg, 3, 9);
  STevState& state = CGX_STATE.mTevStates[stageId];
  if (flags != state.mColorOps) {
    state.mColorOps = flags;
    GXSetTevColorOp(stageId, op, bias, scale, clamp, outReg);
  }
}

void CGX::SetTevColorOp_Compressed(GXTevStageID stageId, uint flags) {
  STevState& state = CGX_STATE.mTevStates[stageId];
  if (flags != state.mColorOps) {
    state.mColorOps = flags;
    GXSetTevColorOp(stageId, static_cast< GXTevOp >(ShiftRightAndMask(flags, 0xF, 0)),
                    static_cast< GXTevBias >(ShiftRightAndMask(flags, 3, 4)),
                    static_cast< GXTevScale >(ShiftRightAndMask(flags, 3, 6)),
                    static_cast< GXBool >(ShiftRightAndMask(flags, 1, 8)),
                    static_cast< GXTevRegID >(ShiftRightAndMask(flags, 3, 9)));
  }
}

void CGX::SetTevAlphaOp(GXTevStageID stageId, GXTevOp op, GXTevBias bias, GXTevScale scale,
                        GXBool clamp, GXTevRegID outReg) {
  uint flags = MaskAndShiftLeft(op, 0xF, 0) | MaskAndShiftLeft(bias, 3, 4) |
               MaskAndShiftLeft(scale, 3, 6) | MaskAndShiftLeft(clamp, 1, 8) |
               MaskAndShiftLeft(outReg, 3, 9);
  STevState& state = CGX_STATE.mTevStates[stageId];
  if (flags != state.mAlphaOps) {
    state.mAlphaOps = flags;
    GXSetTevAlphaOp(stageId, op, bias, scale, clamp, outReg);
  }
}

void CGX::SetTevAlphaOp_Compressed(GXTevStageID stageId, uint flags) {
  STevState& state = CGX_STATE.mTevStates[stageId];
  if (flags != state.mAlphaOps) {
    state.mAlphaOps = flags;
    GXSetTevAlphaOp(stageId, static_cast< GXTevOp >(ShiftRightAndMask(flags, 0xF, 0)),
                    static_cast< GXTevBias >(ShiftRightAndMask(flags, 3, 4)),
                    static_cast< GXTevScale >(ShiftRightAndMask(flags, 3, 6)),
                    static_cast< GXBool >(ShiftRightAndMask(flags, 1, 8)),
                    static_cast< GXTevRegID >(ShiftRightAndMask(flags, 3, 9)));
  }
}

void CGX::SetTevKColorSel(GXTevStageID stageId, GXTevKColorSel sel) {
  STevState& state = CGX_STATE.mTevStates[stageId];
  if (sel != state.mKColorSel) {
    state.mKColorSel = sel;
    GXSetTevKColorSel(stageId, sel);
  }
}

void CGX::SetTevKAlphaSel(GXTevStageID stageId, GXTevKAlphaSel sel) {
  STevState& state = CGX_STATE.mTevStates[stageId];
  if (sel != state.mKAlphaSel) {
    state.mKAlphaSel = sel;
    GXSetTevKAlphaSel(stageId, sel);
  }
}

void CGX::SetTevOrder(GXTevStageID stageId, GXTexCoordID texCoord, GXTexMapID texMap,
                      GXChannelID color) {
  STevState& state = CGX_STATE.mTevStates[stageId];
  uint flags = MaskAndShiftLeft(texCoord, 0xFF, 0) | MaskAndShiftLeft(texMap, 0xFF, 8) |
               MaskAndShiftLeft(color, 0xFF, 16);
  if (state.mTevOrderFlags != flags) {
    state.mTevOrderFlags = flags;
    GXSetTevOrder(stageId, texCoord, texMap, color);
  }
}

void CGX::SetBlendMode(GXBlendMode mode, GXBlendFactor srcFac, GXBlendFactor dstFac, GXLogicOp op) {
  uint flags = MaskAndShiftLeft(mode, 3, 0) | MaskAndShiftLeft(srcFac, 7, 2) |
               MaskAndShiftLeft(dstFac, 7, 5) | MaskAndShiftLeft(op, 0xF, 8);
  if (flags != CGX_STATE.mBlendMode) {
    update_fog(flags);
    CGX_STATE.mBlendMode = flags;
    GXSetBlendMode(mode, srcFac, dstFac, op);
  }
}

void CGX::SetZMode(const TZModeFlag compareEnable, GXCompare func, const TZModeFlag updateEnable) {
  uchar flags = MaskAndShiftLeft(compareEnable, 0xFF, 0) | MaskAndShiftLeft(updateEnable, 0xFF, 1) |
                MaskAndShiftLeft(func, 0xFF, 2);
  if (flags != CGX_STATE.mZmode) {
    CGX_STATE.mZmode = flags;
    GXSetZMode(compareEnable, func, updateEnable);
  }
}

void CGX::SetTevIndirect(GXTevStageID stageId, GXIndTexStageID indStage, GXIndTexFormat fmt,
                         GXIndTexBiasSel biasSel, GXIndTexMtxID mtxSel, GXIndTexWrap wrapS,
                         GXIndTexWrap wrapT, GXBool addPrev, GXBool indLod,
                         GXIndTexAlphaSel alphaSel) {
  STevState& state = CGX_STATE.mTevStates[stageId];
  uint flags = MaskAndShiftLeft(indStage, 3, 0) | MaskAndShiftLeft(fmt, 3, 2) |
               MaskAndShiftLeft(biasSel, 7, 4) | MaskAndShiftLeft(mtxSel, 15, 7) |
               MaskAndShiftLeft(wrapS, 7, 11) | MaskAndShiftLeft(wrapT, 7, 14) |
               MaskAndShiftLeft(addPrev, 1, 17) | MaskAndShiftLeft(indLod, 1, 18) |
               MaskAndShiftLeft(alphaSel, 3, 19);
  if (state.mIndFlags != flags) {
    state.mIndFlags = flags;
    GXSetTevIndirect(stageId, indStage, fmt, biasSel, mtxSel, wrapS, wrapT, addPrev, indLod,
                     alphaSel);
  }
}

void CGX::SetTevDirect(GXTevStageID stageId) {
  STevState& state = CGX_STATE.mTevStates[stageId];
  if (state.mIndFlags != 0) {
    state.mIndFlags = 0;
    GXSetTevDirect(stageId);
  }
}

void CGX::SetTexCoordGen(GXTexCoordID dstCoord, GXTexGenType fn, GXTexGenSrc src, GXTexMtx mtx,
                         GXBool normalize, GXPTTexMtx postMtx) {
  STexState& state = CGX_STATE.mTexStates[dstCoord];
#if RSTL_KYOTO_CGX_CORRUPTION
  uint vm = mtx / 3;
#else
  uint vm = (mtx - GX_TEXMTX0) / 3;
#endif
  uint vp = postMtx - GX_PTTEXMTX0;
#if NONMATCHING
  // Similarly to GXTexMtx, this should also be divided by 3
  vp /= 3;
#endif
  uint flags = MaskAndShiftLeft(fn, 0xF, 0) | MaskAndShiftLeft(src, 0x1F, 4) |
               MaskAndShiftLeft(vm, 0x1F, 9) | MaskAndShiftLeft(normalize, 1, 14) |
               MaskAndShiftLeft(vp, 0x3F, 15);
  if (state.mCoordGen != flags) {
    state.mCoordGen = flags;
    GXSetTexCoordGen2(dstCoord, fn, src, mtx, normalize, postMtx);
  }
}

void CGX::SetTexCoordGen_Compressed(GXTexCoordID dstCoord, uint flags) {
  STexState& state = CGX_STATE.mTexStates[dstCoord];
  if (state.mCoordGen != flags) {
    GXTexMtx mtx = static_cast< GXTexMtx >(GX_TEXMTX0 + ShiftRightAndMask(flags, 0x1F, 9));
    GXPTTexMtx postMtx =
        static_cast< GXPTTexMtx >(GX_PTTEXMTX0 + ShiftRightAndMask(flags, 0x3F, 15));
    state.mCoordGen = flags;
    GXSetTexCoordGen2(dstCoord, static_cast< GXTexGenType >(ShiftRightAndMask(flags, 0xF, 0)),
                      static_cast< GXTexGenSrc >(ShiftRightAndMask(flags, 0x1F, 4)), mtx,
                      static_cast< GXBool >(ShiftRightAndMask(flags, 1, 14)), postMtx);
  }
}

void CGX::SetNumIndStages(uchar num) {
  if (CGX_STATE.mNumIndStages != num) {
    CGX_STATE.mNumIndStages = num;
    GXSetNumIndStages(num);
  }
}

void CGX::SetArray(GXAttr attr, const void* data, uchar stride) {
  if (data == nullptr) {
    return;
  }
#if RSTL_KYOTO_CGX_CORRUPTION
  if (attr >= GX_VA_POS && attr < GX_VA_MAX_ATTR) {
    uint idx = attr - GX_VA_POS;
    if (CGX_STATE.mArrayPtrs[idx] == data && CGX_STATE.mArrayStrides[idx] == stride) {
      return;
    }
    CGX_STATE.mArrayPtrs[idx] = data;
    CGX_STATE.mArrayStrides[idx] = stride;
  }
#else
  if (attr >= GX_VA_POS && attr < GX_POS_MTX_ARRAY) {
    uint idx = attr - GX_VA_POS;
    if (CGX_STATE.mArrayPtrs[idx] == data) {
      return;
    }
    CGX_STATE.mArrayPtrs[idx] = data;
  }
#endif
  GXSetArray(attr, const_cast< void* >(data), stride);
}

#if !RSTL_KYOTO_CGX_CORRUPTION
void CGX::Begin(GXPrimitive prim, GXVtxFmt fmt, ushort numVtx) {
  if (CGX_STATE.mChanFlags != 0) {
    FlushState();
  }
  GXBegin(prim, fmt, numVtx);
}

void CGX::End() { GXEnd(); }
#endif

void CGX::SetFog(GXFogType type, float startZ, float endZ, float nearZ, float farZ,
                 const GXColor& color) {
  CGX_STATE.mFogType = type;
  CGX_STATE.mFogParams.mFogStartZ = startZ;
  CGX_STATE.mFogParams.mFogEndZ = endZ;
  CGX_STATE.mFogParams.mFogNearZ = nearZ;
  CGX_STATE.mFogParams.mFogFarZ = farZ;
  CopyGXColor(CGX_STATE.mFogParams.mFogColor, color);
  apply_fog();
}

void CGX::SetLineWidth(uchar width, GXTexOffset offset) {
  ushort flags = width | offset << 8;
  if (flags != CGX_STATE.mLineWidthAndOffset) {
    CGX_STATE.mLineWidthAndOffset = flags;
    GXSetLineWidth(width, offset);
  }
}

CGX::STevState::STevState()
: mColorInArgs(0)
, mAlphaInArgs(0)
, mColorOps(0)
, mAlphaOps(0)
, mIndFlags(0)
, mTevOrderFlags(UINT_MAX)
, mKColorSel(UCHAR_MAX)
, mKAlphaSel(UCHAR_MAX)
#if RSTL_KYOTO_CGX_CORRUPTION
, mSwapMode(UCHAR_MAX)
#endif
{}

CGX::STexState::STexState() : mCoordGen(0) {}
#endif

CGX::SGXState::SGXState()
: mDescList(0)
, mPrevNumChans(2)
, mNumChans(0)
, mNumTexGens(UCHAR_MAX)
, mNumTevStages(UCHAR_MAX)
, mNumIndStages(UCHAR_MAX)
, mZmode(UCHAR_MAX)
, mFogType(0)
, mLineWidthAndOffset(USHRT_MAX)
, mBlendMode(USHRT_MAX)
, mAlphaCompare(UINT_MAX)
#if !RSTL_PRIME1
, mDstAlphaEnabled(false)
, mDstAlpha(0)
#endif
{
  const GXColor sGXClear = {0, 0, 0, 0};
  const GXColor sGXWhite = {255, 255, 255, 255};
  for (int i = 0; i < 2; ++i) {
    mPrevChanCtrls[i] = USHRT_MAX;
    mChanCtrls[i] = USHRT_MAX;
    mChanAmbColors[i] = sGXClear;
    mChanMatColors[i] = sGXWhite;
  }
  for (int i = 0; i < 4; ++i) {
    mKColors[i] = sGXClear;
  }
#if RSTL_KYOTO_CGX_CORRUPTION
  for (int i = 0; i < 4; ++i) {
    mTevSwapTables[i] = UCHAR_MAX;
  }
#endif
  SetChanCtrl(Channel0, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
  SetChanCtrl(Channel1, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
#if RSTL_KYOTO_CGX_CORRUPTION
  SetChanCtrl(Channel2, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
  SetChanCtrl(Channel3, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
  for (int fmt = 0; fmt < 8; ++fmt) {
    for (int attr = 0; attr < GX_VA_MAX_ATTR; ++attr) {
      mVtxAttrSizes[fmt][attr] = attr < GX_VA_POS ? 1 : 0;
    }
  }
#endif
}

void CGX::ResetGXStates() {
  CGX_STATE.mDescList = 0;

  GXClearVtxDesc();
#if RSTL_KYOTO_CGX_CORRUPTION
  for (int i = 0; i < 17; i++) {
    CGX_STATE.mArrayPtrs[i] = reinterpret_cast< const void* >(1);
    CGX_STATE.mArrayStrides[i] = 0;
  }
#else
  for (int i = 0; i < 12; i++) {
    CGX_STATE.mArrayPtrs[i] = reinterpret_cast< const void* >(1);
  }
#endif
  for (int i = 0; i < 8; i++) {
    CTexture::InvalidateTexmap(static_cast< GXTexMapID >(i));
  }
  for (int i = 0; i < 4; i++) {
    GXSetTevKColor(static_cast< GXTevKColorID >(i), CGX_STATE.mKColors[i]);
  }
#if RSTL_KYOTO_CGX_CORRUPTION
  SetChanCtrl(Channel0, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
  SetChanCtrl(Channel1, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
  SetChanCtrl(Channel2, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
  SetChanCtrl(Channel3, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
  CGX_STATE.mChanFlags = UCHAR_MAX;
  FlushState();

  GXSetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_ALPHA);
#endif
  GXSetTevSwapModeTable(GX_TEV_SWAP1, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_RED);
  GXSetTevSwapModeTable(GX_TEV_SWAP2, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_GREEN);
  GXSetTevSwapModeTable(GX_TEV_SWAP3, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_BLUE);
#if RSTL_KYOTO_CGX_CORRUPTION
  SetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_ALPHA);
  SetTevSwapModeTable(GX_TEV_SWAP1, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_RED);
  SetTevSwapModeTable(GX_TEV_SWAP2, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_GREEN);
  SetTevSwapModeTable(GX_TEV_SWAP3, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_BLUE);
#endif
  SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
  GXSetCurrentMtx(GX_PNMTX0);
  SetNumIndStages(0);
  for (int i = 0; i < 4; i++) {
    GXSetIndTexCoordScale(static_cast< GXIndTexStageID >(i), GX_ITS_1, GX_ITS_1);
  }
  for (int i = 0; i < 16; i++) {
    SetTevDirect(static_cast< GXTevStageID >(i));
#if RSTL_KYOTO_CGX_CORRUPTION
    SetTevSwapMode(static_cast< GXTevStageID >(i), GX_TEV_SWAP0, GX_TEV_SWAP0);
#endif
  }
  for (int i = 0; i < 8; i++) {
    GXSetTexCoordScaleManually(static_cast< GXTexCoordID >(i), false, 0, 0);
  }
  GXSetZTexture(GX_ZT_DISABLE, GX_TF_Z8, 0);
#if RSTL_PRIME1
#if RSTL_VERSION == RSTL_GM8J_00
  GXSetMisc(GX_MT_XF_FLUSH, 8);
  GXSetDither(GX_FALSE);
#endif
#elif RSTL_KYOTO_CGX_CORRUPTION
  GXSetMisc(GX_MT_XF_FLUSH, 8);
  GXSetDither(GX_TRUE);
  GXSetDstAlpha(GX_FALSE, 0);

  SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
  SetVtxAttrFmt(GX_VTXFMT1, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
  SetVtxAttrFmt(GX_VTXFMT2, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
  SetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
  SetVtxAttrFmt(GX_VTXFMT1, GX_VA_NRM, GX_NRM_XYZ, GX_S16, 14);
  SetVtxAttrFmt(GX_VTXFMT2, GX_VA_NRM, GX_NRM_XYZ, GX_S16, 14);
  SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
  SetVtxAttrFmt(GX_VTXFMT1, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
  SetVtxAttrFmt(GX_VTXFMT2, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
  SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
  SetVtxAttrFmt(GX_VTXFMT1, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
  SetVtxAttrFmt(GX_VTXFMT2, GX_VA_TEX0, GX_TEX_ST, GX_U16, 15);
  for (int i = 1; i <= 7; ++i) {
    const GXAttr attr = static_cast< GXAttr >(GX_VA_TEX0 + i);
    SetVtxAttrFmt(GX_VTXFMT0, attr, GX_TEX_ST, GX_F32, 0);
    SetVtxAttrFmt(GX_VTXFMT1, attr, GX_TEX_ST, GX_F32, 0);
    SetVtxAttrFmt(GX_VTXFMT2, attr, GX_TEX_ST, GX_F32, 0);
  }
#else
  GXSetMisc(GX_MT_XF_FLUSH, 8);
  GXSetDither(GX_FALSE);
  GXSetDstAlpha(GX_FALSE, 0);
#endif

}

void CGX::ResetGXStatesFull() {
  new (&CGX_STATE) SGXState();
  ResetGXStates();
}

#if RSTL_PRIME1
static inline void __SetChanCtrlCompressed(GXChannelID channel, uint flags) {
  GXBool enable = CGX::ShiftRightAndMask(flags, 1, 0);
  GXColorSrc ambSrc = static_cast< GXColorSrc >(CGX::ShiftRightAndMask(flags, 1, 1));
  GXColorSrc matSrc = static_cast< GXColorSrc >(CGX::ShiftRightAndMask(flags, 1, 2));
  GXLightID lightMask = static_cast< GXLightID >(CGX::ShiftRightAndMask(flags, 0xFF, 3));
  GXDiffuseFn diffFn = static_cast< GXDiffuseFn >(CGX::ShiftRightAndMask(flags, 3, 11));
  GXAttnFn attnFn = static_cast< GXAttnFn >(CGX::ShiftRightAndMask(flags, 3, 13));

  GXSetChanCtrl(channel, enable, ambSrc, matSrc, lightMask, diffFn, attnFn);
#else
void CGX::FlushChanCtrl(GXChannelID chan, ushort flags) {
  GXBool enable = ShiftRightAndMask(flags, 1, 0);
  GXColorSrc ambSrc = static_cast< GXColorSrc >(ShiftRightAndMask(flags, 1, 1));
  GXColorSrc matSrc = static_cast< GXColorSrc >(ShiftRightAndMask(flags, 1, 2));
  uint lightMask = ShiftRightAndMask(flags, 0xFF, 3);
  GXDiffuseFn diffFn = static_cast< GXDiffuseFn >(ShiftRightAndMask(flags, 3, 11));
  GXAttnFn attnFn = static_cast< GXAttnFn >(ShiftRightAndMask(flags, 3, 13));
  GXSetChanCtrl(chan, enable, ambSrc, matSrc, lightMask, diffFn, attnFn);
#endif

}

void CGX::FlushState() {
  if (CGX_STATE.mChanFlags & 1) {
    GXSetNumChans(CGX_STATE.mNumChans);
    CGX_STATE.mPrevNumChans = CGX_STATE.mNumChans;
  }
#if RSTL_KYOTO_CGX_CORRUPTION
  for (int i = 0; i < 4; ++i) {
    if (CGX_STATE.mChanFlags & (1 << (i + 1))) {
      FlushChanCtrl(static_cast< GXChannelID >(GX_COLOR0 + i), CGX_STATE.mChanCtrls[i]);
      CGX_STATE.mPrevChanCtrls[i] = CGX_STATE.mChanCtrls[i];
    }
  }
#else
  if (CGX_STATE.mChanFlags & 2) {
#if RSTL_PRIME1
    __SetChanCtrlCompressed(GX_COLOR0, CGX_STATE.mChanCtrls[0]);
#else
    FlushChanCtrl(GX_COLOR0, CGX_STATE.mChanCtrls[0]);
#endif
    CGX_STATE.mPrevChanCtrls[0] = CGX_STATE.mChanCtrls[0];
  }
  if (CGX_STATE.mChanFlags & 4) {
#if RSTL_PRIME1
    __SetChanCtrlCompressed(GX_COLOR1, CGX_STATE.mChanCtrls[1]);
#else
    FlushChanCtrl(GX_COLOR1, CGX_STATE.mChanCtrls[1]);
#endif
    CGX_STATE.mPrevChanCtrls[1] = CGX_STATE.mChanCtrls[1];
  }
#endif
  CGX_STATE.mChanFlags = 0;
}

void CGX::SetIndTexMtxSTPointFive(GXIndTexMtxID id, s8 scaleExp) {
  static const float indMtx[2][3] = {
      {0.5f, 0.f, 0.f},
      {0.f, 0.5f, 0.f},
  };
  GXSetIndTexMtx(id, const_cast< float (*)[3] >(indMtx), scaleExp);
}

void CGX::SetVtxDescv_Compressed(uint flags) {
  if (flags == CGX_STATE.mDescList) {

    return;
  }
  GXVtxDescList* list = sVtxDescList;
  for (uint idx = 0; idx < 11; ++idx) {
    uint shift = idx * 2;
#if RSTL_PRIME1
    if ((flags & 3 << shift) == (CGX_STATE.mDescList & 3 << shift)) {
#else
    const uint mask = 3 << shift;
    if ((flags & mask) == (CGX_STATE.mDescList & mask)) {
#endif

      continue;
    }
    list->attr = static_cast< GXAttr >(GX_VA_POS + idx);
    list->type = static_cast< GXAttrType >(flags >> shift & 3);
    ++list;
  }
#if !RSTL_PRIME1
  if ((flags & 0xff000000) != (CGX_STATE.mDescList & 0xff000000)) {
    for (uint idx = 0; idx < 8; ++idx) {
      uint shift = idx + 24;
      const uint mask = 1 << shift;
      if ((flags & mask) == (CGX_STATE.mDescList & mask)) {
        continue;
      }
      list->attr = static_cast< GXAttr >(GX_VA_PNMTXIDX + idx);
      list->type = static_cast< GXAttrType >(flags >> shift & 1);
      ++list;
    }
  }
#endif

  list->attr = GX_VA_NULL;
  list->type = GX_NONE;
  GXSetVtxDescv(sVtxDescList);
  CGX_STATE.mDescList = flags;
}

void CGX::SetVtxDesc(GXAttr attr, GXAttrType type) {
#if RSTL_PRIME1
  uint lshift = (attr - GX_VA_POS) * 2;
  uint rshift = 3 << lshift;
  uint flags = type << lshift;
  if (flags != (CGX_STATE.mDescList & rshift)) {
    CGX_STATE.mDescList = flags | (CGX_STATE.mDescList & ~rshift);
#else
  uint mask;
  uint flags;
  if (attr <= GX_VA_TEX6MTXIDX) {
    int shift = attr + 24;
    mask = 1 << shift;
    flags = (type != GX_NONE) << shift;
  } else if (attr >= GX_VA_POS) {
    uint shift = (attr - GX_VA_POS) * 2;
    mask = 3 << shift;
    flags = type << shift;
  }
  if (flags != (CGX_STATE.mDescList & mask)) {
    CGX_STATE.mDescList = flags | (CGX_STATE.mDescList & ~mask);
#endif

    GXSetVtxDesc(attr, type);
  }
}

void CGX::ResetVtxDescv() {
  static const GXVtxDescList vtxDescList[2] = {
      {GX_VA_POS, GX_INDEX16},
      {GX_VA_NULL, GX_NONE},
  };
  SetVtxDescv(vtxDescList);
}

void CGX::SetVtxDescv(const GXVtxDescList* list) {
  uint flags = 0;
  for (; list->attr != GX_VA_NULL; ++list) {
#if !RSTL_PRIME1
    if (list->attr <= GX_VA_TEX6MTXIDX) {
      flags |= (list->type != GX_NONE) << (list->attr + 24);
    } else {
#endif

    flags |= (list->type & 3) << (list->attr - GX_VA_POS) * 2;
#if !RSTL_PRIME1
    }
#endif

  }
  SetVtxDescv_Compressed(flags);
}

void CGX::SetStandardDirectTev_Compressed(GXTevStageID stageId, uint colorArgs, uint alphaArgs,
                                          uint colorOps, uint alphaOps) {
  STevState& state = CGX_STATE.mTevStates[stageId];

  if (state.mIndFlags != 0) {
    state.mIndFlags = 0;
    GXSetTevDirect(stageId);
  }
  if (state.mColorInArgs != colorArgs) {
    state.mColorInArgs = colorArgs;
    GXSetTevColorIn(stageId, static_cast< GXTevColorArg >(ShiftRightAndMask(colorArgs, 31, 0)),
                    static_cast< GXTevColorArg >(ShiftRightAndMask(colorArgs, 31, 5)),
                    static_cast< GXTevColorArg >(ShiftRightAndMask(colorArgs, 31, 10)),
                    static_cast< GXTevColorArg >(ShiftRightAndMask(colorArgs, 31, 15)));
  }
  if (state.mAlphaInArgs != alphaArgs) {
    state.mAlphaInArgs = alphaArgs;
    GXSetTevAlphaIn(stageId, static_cast< GXTevAlphaArg >(ShiftRightAndMask(alphaArgs, 31, 0)),
                    static_cast< GXTevAlphaArg >(ShiftRightAndMask(alphaArgs, 31, 5)),
                    static_cast< GXTevAlphaArg >(ShiftRightAndMask(alphaArgs, 31, 10)),
                    static_cast< GXTevAlphaArg >(ShiftRightAndMask(alphaArgs, 31, 15)));
  }
  if (colorOps != alphaOps || (colorOps & 0x1FF) != 0x100) {
    SetTevColorOp_Compressed(stageId, colorOps);
    SetTevAlphaOp_Compressed(stageId, alphaOps);
  } else if (colorOps != state.mColorOps || colorOps != state.mAlphaOps) {
    // Fast path for GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true
    state.mAlphaOps = colorOps;
    state.mColorOps = colorOps;
    GXTevRegID outReg = static_cast< GXTevRegID >(ShiftRightAndMask(colorOps, 3, 9));
    GXSetTevColorOp(stageId, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, outReg);
    GXSetTevAlphaOp(stageId, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, outReg);
  }
}

void CGX::SetStandardTevColorAlphaOp(GXTevStageID stageId) {
  STevState& state = CGX_STATE.mTevStates[stageId];

  if (state.mColorOps != 0x100 || state.mAlphaOps != 0x100) {
    state.mAlphaOps = 0x100;
    state.mColorOps = 0x100;
    GXSetTevColorOp(stageId, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
    GXSetTevAlphaOp(stageId, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  }
}

void CGX::GetFog(GXFogType* fogType, float* fogStartZ, float* fogEndZ, float* fogNearZ,
                 float* fogFarZ, GXColor* fogColor) {
  if (fogType != nullptr) {
    *fogType = static_cast< GXFogType >(CGX_STATE.mFogType);
  }
  if (fogStartZ != nullptr) {
    *fogStartZ = CGX_STATE.mFogParams.mFogStartZ;
  }
  if (fogEndZ != nullptr) {
    *fogEndZ = CGX_STATE.mFogParams.mFogEndZ;
  }
  if (fogNearZ != nullptr) {
    *fogNearZ = CGX_STATE.mFogParams.mFogNearZ;
  }
  if (fogFarZ != nullptr) {
    *fogFarZ = CGX_STATE.mFogParams.mFogFarZ;
  }
  if (fogColor != nullptr) {
#if RSTL_PRIME1
    CopyGXColor(*fogColor, CGX_STATE.mFogParams.mFogColor);
#else
    CopyGXColor(*fogColor, CGX_STATE.mFogParams.mFogColor);
  }
}

void CGX::CallDisplayList(const void* ptr, size_t size) {
  if (CGX_STATE.mChanFlags != 0) {
    FlushState();
  }
  GXCallDisplayList(const_cast< void* >(ptr), size);
}

#if RSTL_KYOTO_CGX_CORRUPTION
void CGX::Begin(GXPrimitive prim, GXVtxFmt fmt, ushort numVtx) {
  if (CGX_STATE.mChanFlags != 0) {
    FlushState();
  }
  GXBegin(prim, fmt, numVtx);
}

void CGX::End() { GXEnd(); }
#endif

void CGX::SetDstAlpha(bool enable, RSTL_KYOTO_SCALAR_PARAM_CONST uchar alpha) {
  if (!enable) {
    if (CGX_STATE.mDstAlphaEnabled) {
      CGX_STATE.mDstAlphaEnabled = false;
      GXSetDstAlpha(false, 0);
    }
  } else if (!CGX_STATE.mDstAlphaEnabled || CGX_STATE.mDstAlpha != alpha) {
    CGX_STATE.mDstAlphaEnabled = true;
    CGX_STATE.mDstAlpha = alpha;
    GXSetDstAlpha(enable, alpha);
  }
}

void CGX::SetAlphaCompare(GXCompare comp0, uchar ref0, GXAlphaOp op, GXCompare comp1, uchar ref1) {
  if (comp0 == GX_ALWAYS) {
    if (CGX_STATE.mAlphaCompare != sAlwaysAlphaCompare) {
      GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
      CGX_STATE.mAlphaCompare = sAlwaysAlphaCompare;
      GXSetZCompLoc(true);
    }
    return;
  }
  uint flags = MaskAndShiftLeft(comp0, 7, 0) | MaskAndShiftLeft(ref0, 0xFF, 3) |
               MaskAndShiftLeft(op, 7, 11) | MaskAndShiftLeft(comp1, 7, 14) |
               MaskAndShiftLeft(ref1, 0xFF, 17);
  if (CGX_STATE.mAlphaCompare != flags) {
    if (CGX_STATE.mAlphaCompare == sAlwaysAlphaCompare) {
      GXSetZCompLoc(false);
    }
    CGX_STATE.mAlphaCompare = flags;
    GXSetAlphaCompare(comp0, ref0, op, comp1, ref1);
  }
}

uchar CGX::GetNumTexGens() { return CGX_STATE.mNumTexGens; }

uchar CGX::GetNumTevStages() { return CGX_STATE.mNumTevStages; }

#if RSTL_KYOTO_CGX_CORRUPTION
void CGX::GetTexCoordGen(GXTexCoordID coord, GXTexGenType* type, GXTexGenSrc* source,
                         int* matrixIndex, GXBool* normalize, GXPTTexMtx* postMatrix) {
  const STexState& state = CGX_STATE.mTexStates[coord];
  const uint flags = state.mCoordGen;
  if (type != nullptr) {
    *type = static_cast< GXTexGenType >(ShiftRightAndMask(flags, 0xF, 0));
  }
  if (source != nullptr) {
    *source = static_cast< GXTexGenSrc >(ShiftRightAndMask(flags, 0x1F, 4));
  }
  if (matrixIndex != nullptr) {
    *matrixIndex = ShiftRightAndMask(flags, 0x1F, 9);
  }
  if (normalize != nullptr) {
    *normalize = ShiftRightAndMask(flags, 1, 14);
  }
  if (postMatrix != nullptr) {
    *postMatrix = static_cast< GXPTTexMtx >(ShiftRightAndMask(flags, 0x3F, 15) + GX_PTTEXMTX0);
  }
}

void CGX::SetVtxAttrFmt(GXVtxFmt fmt, GXAttr attr, GXCompCnt count, GXCompType type, uchar frac) {
  int components = 0;
  int componentSize = 0;
  bool error = false;
  if (attr == GX_VA_POS) {
    switch (count) {
    case GX_POS_XYZ:
      components = 3;
      break;
    case GX_POS_XY:
      components = 2;
      break;
    default:
      error = true;
      break;
    }
  } else if (attr == GX_VA_NRM) {
    if (count == GX_NRM_XYZ) {
      components = 3;
    } else {
      error = true;
    }
  } else if (attr == GX_VA_CLR0 || attr == GX_VA_CLR1) {
    if (count == GX_CLR_RGB) {
      components = 3;
    } else if (count == GX_CLR_RGBA) {
      components = 4;
    } else {
      error = true;
    }
  } else if (attr >= GX_VA_TEX0 && attr <= GX_VA_TEX7) {
    if (count == GX_TEX_ST) {
      components = 2;
    } else if (count == GX_TEX_S) {
      components = 1;
    } else {
      error = true;
    }
  } else {
    error = true;
  }

  if (attr == GX_VA_POS || (attr >= GX_VA_TEX0 && attr <= GX_VA_TEX7)) {
    switch (type) {
    case GX_U8:
    case GX_S8:
      componentSize = 1;
      break;
    case GX_U16:
    case GX_S16:
      componentSize = 2;
      break;
    case GX_F32:
      componentSize = 4;
      break;
    default:
      error = true;
      break;
    }
  } else if (attr == GX_VA_NRM) {
    switch (type) {
    case GX_S8:
      componentSize = 1;
      break;
    case GX_S16:
      componentSize = 2;
      break;
    case GX_F32:
      componentSize = 4;
      break;
    default:
      error = true;
      break;
    }
  } else if (attr == GX_VA_CLR0 || attr == GX_VA_CLR1) {
    switch (type) {
    case GX_RGB565:
    case GX_RGBA4:
      components = 1;
      componentSize = 2;
      break;
    case GX_RGB8:
    case GX_RGBA6:
      components = 1;
      componentSize = 3;
      break;
    case GX_RGBX8:
    case GX_RGBA8:
      components = 1;
      componentSize = 4;
      break;
    default:
      error = true;
      break;
    }
  }

  RS_VERIFY_THROW_DIRECT(1349, !error, false, "Error setting vertex attributes");
  if (!error) {
    CGX_STATE.mVtxAttrSizes[fmt][attr] = components * componentSize;
  }
  GXSetVtxAttrFmt(fmt, attr, count, type, frac);
}

void CGX::LoadPosMtxImm(const float mtx[][4], unsigned long id) {
  GXLoadPosMtxImm(const_cast< MtxPtr >(mtx), id);
}

void CGX::LoadNrmMtxImm(const float mtx[][4], unsigned long id) {
  GXLoadNrmMtxImm(const_cast< MtxPtr >(mtx), id);
}
#endif

#if !defined(TARGET_PC) && !RSTL_KYOTO_CGX_CORRUPTION
static inline void write_bp_cmd(u32 cmd) {
  GXWGFifo.u8 = GX_LOAD_BP_REG;
  GXWGFifo.u32 = cmd;
  __GXData->cpCRreg = 0;
}
#endif

void CGX::update_fog(uint flags) {
  if (CGX_STATE.mFogType == 0) {
    return;
  }
  if ((CGX_STATE.mBlendMode & 0xE0) == (flags & 0xE0)) {
    return;
  }
  if ((flags & 0xE0) == 0x20) {
#if defined(TARGET_PC) || RSTL_KYOTO_CGX_CORRUPTION
    static const GXColor sGXClear = {0, 0, 0, 0};
    GXSetFogColor(sGXClear);
#else
    write_bp_cmd(0xf2000000);
#endif
  } else {
#if defined(TARGET_PC) || RSTL_KYOTO_CGX_CORRUPTION
    GXSetFogColor(CGX_STATE.mFogParams.mFogColor);
#else
    write_bp_cmd((CGX_STATE.mFogParams.mFogColor.b) | (CGX_STATE.mFogParams.mFogColor.g << 8) |
                 (CGX_STATE.mFogParams.mFogColor.r << 16) | 0xf2000000);
#endif
#endif

  }
}
