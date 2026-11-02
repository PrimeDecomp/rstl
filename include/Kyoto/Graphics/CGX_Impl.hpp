#ifndef _CGX_IMPL
#define _CGX_IMPL
#include "rstl/RstlVersions.h"

#include "Kyoto/Graphics/CGX.hpp"

#include "Kyoto/Alloc/CMemory.hpp"

#include "Kyoto/Graphics/CColor.hpp"
#if RSTL_PRIME1
#include "Kyoto/Graphics/CTexture.hpp"

#include "dolphin/gx.h"
#endif

#ifndef CGX_INLINE
#define CGX_INLINE inline
#if RSTL_PRIME1
#define CGX_INLINE_HEADER
#endif
#endif

// Setters wrapped in "!defined(CGX_INLINE_HEADER) || RSTL_VERSION < RSTL_GM8P_00" are only
// defined in CGX.cpp from PAL on, so callers no longer inline them.

CGX_INLINE void CGX::SetNumChans(uchar num) {
  CGX_STATE.mNumChans = num;
  CGX_STATE.mFlags.numDirty = num != CGX_STATE.mPrevNumChans;
}

CGX_INLINE void CGX::SetNumTexGens(uchar num) {
  if (num != CGX_STATE.mNumTexGens) {
    CGX_STATE.mNumTexGens = num;

    GXSetNumTexGens(num);
  }
}

CGX_INLINE void CGX::SetChanAmbColor(EChannelId channel, const GXColor& color) {
  if (!CompareGXColors(color, CGX_STATE.mChanAmbColors[channel])) {
    CopyGXColor(CGX_STATE.mChanAmbColors[channel], color);

    GXSetChanAmbColor(static_cast< GXChannelID >(GX_COLOR0A0 + channel), color);
  }
}

CGX_INLINE GXColor CGX::GetChanAmbColor(EChannelId channel) {
  const GXColor color = CGX_STATE.mChanAmbColors[channel];
  return color;

}

CGX_INLINE void CGX::SetChanMatColor(EChannelId channel, const GXColor& color) {
  if (!CompareGXColors(color, CGX_STATE.mChanMatColors[channel])) {
    CopyGXColor(CGX_STATE.mChanMatColors[channel], color);

    GXSetChanMatColor(static_cast< GXChannelID >(GX_COLOR0A0 + channel), color);
  }
}

#if RSTL_PRIME1
#if !defined(CGX_INLINE_HEADER) || RSTL_VERSION < RSTL_GM8P_00
CGX_INLINE void CGX::SetChanCtrl(EChannelId channel, GXBool enable, GXColorSrc ambSrc,
                                 GXColorSrc matSrc, GXLightID lights, GXDiffuseFn diffFn,
                                 GXAttnFn attnFn) {
  ushort& state = sGXState.mChanCtrls[channel];
  ushort prevFlags = sGXState.mPrevChanCtrls[channel];
  if (lights == GX_LIGHT_NULL) {
    enable = GX_FALSE;
  }
  uint flags = MaskAndShiftLeft(enable, 1, 0) | MaskAndShiftLeft(ambSrc, 1, 1) |
               MaskAndShiftLeft(matSrc, 1, 2) | MaskAndShiftLeft(lights, 0xFF, 3) |
               MaskAndShiftLeft(diffFn, 3, 11) | MaskAndShiftLeft(attnFn, 3, 13);
  state = flags;
  sGXState.mChanFlags =
      ((flags != prevFlags) << (channel + 1)) | (sGXState.mChanFlags & ~(1 << (channel + 1)));
}
#endif

#if !defined(CGX_INLINE_HEADER) || RSTL_VERSION < RSTL_GM8P_00
CGX_INLINE void CGX::SetChanCtrl_Compressed(EChannelId channel, GXLightID lights, uint ctrl) {
  ushort& state = sGXState.mChanCtrls[channel];
  ushort prevFlags = sGXState.mPrevChanCtrls[channel];
  uint flags = lights != 0 ? ctrl | MaskAndShiftLeft(lights, 0xFF, 3) : ctrl & ~1;
  state = flags;
  sGXState.mChanFlags =
      ((flags != prevFlags) << (channel + 1)) | (sGXState.mChanFlags & ~(1 << (channel + 1)));
}
#endif
#endif

CGX_INLINE void CGX::SetNumTevStages(uchar num) {
  if (CGX_STATE.mNumTevStages != num) {
    CGX_STATE.mNumTevStages = num;

    GXSetNumTevStages(num);
  }
}

CGX_INLINE void CGX::SetTevKColor(GXTevKColorID id, const GXColor& color) {
  if (!CompareGXColors(CGX_STATE.mKColors[id], color)) {
    CopyGXColor(CGX_STATE.mKColors[id], color);

    GXSetTevKColor(id, color);
  }
}

#if RSTL_PRIME1
CGX_INLINE void CGX::SetTevKColor(GXTevKColorID id, const CColor& color) {
  if (!(*reinterpret_cast< const CColor* >(&sGXState.mKColors[id]) == color)) {
    CopyGXColor(sGXState.mKColors[id], color.GetGXColor());
#else
inline void CGX::SetTevKColor(GXTevKColorID id, const CColor& color) {
  if (!(*reinterpret_cast< const CColor* >(&gpGXState->mKColors[id]) == color)) {
    CopyGXColor(gpGXState->mKColors[id], color.GetGXColor());
#endif

    GXSetTevKColor(id, color.GetGXColor());
  }
}

#if RSTL_PRIME1
#if !defined(CGX_INLINE_HEADER) || RSTL_VERSION < RSTL_GM8P_00
CGX_INLINE void CGX::SetTevColorIn(GXTevStageID stageId, GXTevColorArg a, GXTevColorArg b,
                                   GXTevColorArg c, GXTevColorArg d) {
  const uint flags = (a & 0x1F) | ((b & 0x1F) << 5) | ((c & 0x1F) << 10) | ((d & 0x1F) << 15);
  STevState& state = sGXState.mTevStates[stageId];
  if (flags != state.mColorInArgs) {
    state.mColorInArgs = flags;
    GXSetTevColorIn(stageId, a, b, c, d);
  }
}

CGX_INLINE void CGX::SetTevColorIn_Compressed(GXTevStageID stageId, uint flags) {
  STevState& state = sGXState.mTevStates[stageId];
  if (flags != state.mColorInArgs) {
    state.mColorInArgs = flags;
    GXSetTevColorIn(stageId, static_cast< GXTevColorArg >(ShiftRightAndMask(flags, 0x1F, 0)),
                    static_cast< GXTevColorArg >(ShiftRightAndMask(flags, 0x1F, 5)),
                    static_cast< GXTevColorArg >(ShiftRightAndMask(flags, 0x1F, 10)),
                    static_cast< GXTevColorArg >(ShiftRightAndMask(flags, 0x1F, 15)));
  }
}

CGX_INLINE void CGX::SetTevAlphaIn(GXTevStageID stageId, GXTevAlphaArg a, GXTevAlphaArg b,
                                   GXTevAlphaArg c, GXTevAlphaArg d) {
  uint flags = MaskAndShiftLeft(a, 0x1F, 0) | MaskAndShiftLeft(b, 0x1F, 5) |
               MaskAndShiftLeft(c, 0x1F, 10) | MaskAndShiftLeft(d, 0x1F, 15);
  STevState& state = sGXState.mTevStates[stageId];
  if (flags != state.mAlphaInArgs) {
    state.mAlphaInArgs = flags;
    GXSetTevAlphaIn(stageId, a, b, c, d);
  }
}

CGX_INLINE void CGX::SetTevAlphaIn_Compressed(GXTevStageID stageId, uint flags) {
  STevState& state = sGXState.mTevStates[stageId];
  if (flags != state.mAlphaInArgs) {
    state.mAlphaInArgs = flags;
    GXSetTevAlphaIn(stageId, static_cast< GXTevAlphaArg >(ShiftRightAndMask(flags, 0x1F, 0)),
                    static_cast< GXTevAlphaArg >(ShiftRightAndMask(flags, 0x1F, 5)),
                    static_cast< GXTevAlphaArg >(ShiftRightAndMask(flags, 0x1F, 10)),
                    static_cast< GXTevAlphaArg >(ShiftRightAndMask(flags, 0x1F, 15)));
  }
}
#endif

#if !defined(CGX_INLINE_HEADER) || RSTL_VERSION < RSTL_GM8P_00
CGX_INLINE void CGX::SetTevColorOp(GXTevStageID stageId, GXTevOp op, GXTevBias bias,
                                   GXTevScale scale, GXBool clamp, GXTevRegID outReg) {
  const uint flags = MaskAndShiftLeft(op, 0xF, 0) | MaskAndShiftLeft(bias, 3, 4) |
                     MaskAndShiftLeft(scale, 3, 6) | MaskAndShiftLeft(clamp, 1, 8) |
                     MaskAndShiftLeft(outReg, 3, 9);
  STevState& state = sGXState.mTevStates[stageId];
  if (flags != state.mColorOps) {
    state.mColorOps = flags;
    GXSetTevColorOp(stageId, op, bias, scale, clamp, outReg);
  }
}
#endif

CGX_INLINE void CGX::SetTevColorOp_Compressed(GXTevStageID stageId, uint flags) {
  STevState& state = sGXState.mTevStates[stageId];
  if (flags != state.mColorOps) {
    state.mColorOps = flags;
    GXSetTevColorOp(stageId, static_cast< GXTevOp >(ShiftRightAndMask(flags, 0xF, 0)),
                    static_cast< GXTevBias >(ShiftRightAndMask(flags, 3, 4)),
                    static_cast< GXTevScale >(ShiftRightAndMask(flags, 3, 6)),
                    static_cast< GXBool >(ShiftRightAndMask(flags, 1, 8)),
                    static_cast< GXTevRegID >(ShiftRightAndMask(flags, 3, 9)));
  }
}

#if !defined(CGX_INLINE_HEADER) || RSTL_VERSION < RSTL_GM8P_00
CGX_INLINE void CGX::SetTevAlphaOp(GXTevStageID stageId, GXTevOp op, GXTevBias bias,
                                   GXTevScale scale, GXBool clamp, GXTevRegID outReg) {
  uint flags = MaskAndShiftLeft(op, 0xF, 0) | MaskAndShiftLeft(bias, 3, 4) |
               MaskAndShiftLeft(scale, 3, 6) | MaskAndShiftLeft(clamp, 1, 8) |
               MaskAndShiftLeft(outReg, 3, 9);
  STevState& state = sGXState.mTevStates[stageId];
  if (flags != state.mAlphaOps) {
    state.mAlphaOps = flags;
    GXSetTevAlphaOp(stageId, op, bias, scale, clamp, outReg);
  }
}
#endif

CGX_INLINE void CGX::SetTevAlphaOp_Compressed(GXTevStageID stageId, uint flags) {
  STevState& state = sGXState.mTevStates[stageId];
  if (flags != state.mAlphaOps) {
    state.mAlphaOps = flags;
    GXSetTevAlphaOp(stageId, static_cast< GXTevOp >(ShiftRightAndMask(flags, 0xF, 0)),
                    static_cast< GXTevBias >(ShiftRightAndMask(flags, 3, 4)),
                    static_cast< GXTevScale >(ShiftRightAndMask(flags, 3, 6)),
                    static_cast< GXBool >(ShiftRightAndMask(flags, 1, 8)),
                    static_cast< GXTevRegID >(ShiftRightAndMask(flags, 3, 9)));
  }
}
#endif

CGX_INLINE void CGX::SetTevKColorSel(GXTevStageID stageId, GXTevKColorSel sel) {
  STevState& state = CGX_STATE.mTevStates[stageId];

  if (sel != state.mKColorSel) {
    state.mKColorSel = sel;
    GXSetTevKColorSel(stageId, sel);
  }
}

CGX_INLINE void CGX::SetTevKAlphaSel(GXTevStageID stageId, GXTevKAlphaSel sel) {
  STevState& state = CGX_STATE.mTevStates[stageId];

  if (sel != state.mKAlphaSel) {
    state.mKAlphaSel = sel;
    GXSetTevKAlphaSel(stageId, sel);
  }
}

CGX_INLINE void CGX::SetTevOrder(GXTevStageID stageId, GXTexCoordID texCoord, GXTexMapID texMap,
                                 GXChannelID color) {
#if RSTL_PRIME1
  STevState& state = sGXState.mTevStates[stageId];
  uint flags = (texCoord & 0xFF) | ((texMap & 0xFF) << 8) | ((color & 0xFF) << 16);
#else
  STevState& state = gpGXState->mTevStates[stageId];
  uint flags = MaskAndShiftLeft(texCoord, 0xFF, 0) | MaskAndShiftLeft(texMap, 0xFF, 8) |
               MaskAndShiftLeft(color, 0xFF, 16);
#endif

  if (state.mTevOrderFlags != flags) {
    state.mTevOrderFlags = flags;
    GXSetTevOrder(stageId, texCoord, texMap, color);
  }
}

CGX_INLINE void CGX::SetBlendMode(GXBlendMode mode, GXBlendFactor srcFac, GXBlendFactor dstFac,
                                  GXLogicOp op) {
  uint flags = MaskAndShiftLeft(mode, 3, 0) | MaskAndShiftLeft(srcFac, 7, 2) |
               MaskAndShiftLeft(dstFac, 7, 5) | MaskAndShiftLeft(op, 0xF, 8);
  if (flags != CGX_STATE.mBlendMode) {

    update_fog(flags);
    CGX_STATE.mBlendMode = flags;

    GXSetBlendMode(mode, srcFac, dstFac, op);
  }
}

#if RSTL_PRIME1
CGX_INLINE void CGX::SetZMode(const bool compareEnable, GXCompare func,
                              const bool updateEnable) {
#else
CGX_INLINE void CGX::SetZMode(const GXBool compareEnable, GXCompare func,
                              const GXBool updateEnable) {
#endif

  uchar flags = MaskAndShiftLeft(compareEnable, 0xFF, 0) | MaskAndShiftLeft(updateEnable, 0xFF, 1) |
                MaskAndShiftLeft(func, 0xFF, 2);
  if (flags != CGX_STATE.mZmode) {
    CGX_STATE.mZmode = flags;

    GXSetZMode(compareEnable, func, updateEnable);
  }
}

#if RSTL_PRIME1
#if !defined(CGX_INLINE_HEADER) || RSTL_VERSION < RSTL_GM8P_00
CGX_INLINE void CGX::SetAlphaCompare(GXCompare comp0, uchar ref0, GXAlphaOp op, GXCompare comp1,
                                     uchar ref1) {
  uint flags = MaskAndShiftLeft(comp0, 7, 0) | MaskAndShiftLeft(ref0, 0xFF, 3) |
               MaskAndShiftLeft(op, 7, 11) | MaskAndShiftLeft(comp1, 7, 14) |
               MaskAndShiftLeft(ref1, 0xFF, 17);
  if (sGXState.mAlphaCompare != flags) {
    sGXState.mAlphaCompare = flags;
    GXSetAlphaCompare(comp0, ref0, op, comp1, ref1);
    GXSetZCompLoc(comp0 == GX_ALWAYS);
  }
}
#endif

#if !defined(CGX_INLINE_HEADER) || RSTL_VERSION < RSTL_GM8P_00
CGX_INLINE void CGX::SetTevIndirect(const GXTevStageID stageId, const GXIndTexStageID indStage,
                                    const GXIndTexFormat fmt, const GXIndTexBiasSel biasSel,
                                    const GXIndTexMtxID mtxSel, const GXIndTexWrap wrapS,
                                    const GXIndTexWrap wrapT, GXBool addPrev, const GXBool indLod,
                                    const GXIndTexAlphaSel alphaSel) {
  STevState& state = sGXState.mTevStates[stageId];
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
#endif
#else
CGX_INLINE void CGX::SetTevIndirect(GXTevStageID stageId, GXIndTexStageID indStage,
                                    GXIndTexFormat fmt, GXIndTexBiasSel biasSel,
                                    GXIndTexMtxID mtxSel, GXIndTexWrap wrapS, GXIndTexWrap wrapT,
                                    GXBool addPrev, GXBool indLod, GXIndTexAlphaSel alphaSel) {
  STevState& state = gpGXState->mTevStates[stageId];
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
#endif

CGX_INLINE void CGX::SetTevDirect(GXTevStageID stageId) {
  STevState& state = CGX_STATE.mTevStates[stageId];

  if (state.mIndFlags != 0) {
    state.mIndFlags = 0;
    GXSetTevDirect(stageId);
  }
}

#if RSTL_PRIME1
#if !defined(CGX_INLINE_HEADER) || RSTL_VERSION < RSTL_GM8P_00
CGX_INLINE void CGX::SetTexCoordGen(GXTexCoordID dstCoord, GXTexGenType fn, GXTexGenSrc src,
                                    GXTexMtx mtx, GXBool normalize, GXPTTexMtx postMtx) {
  STexState& state = sGXState.mTexStates[dstCoord];
  uint vm = (mtx - GX_TEXMTX0) / 3;
  uint vp = postMtx - GX_PTTEXMTX0;
#if NONMATCHING
  // Similarly to GXTexMtx, this should also be divided by 3
  vp /= 3;
#endif
  uint flags = (fn & 0xF) | ((src & 0x1F) << 4) | ((vm & 0x1F) << 9) | ((normalize & 1) << 14) |
               ((vp & 0x3F) << 15);
  if (state.mCoordGen != flags) {
    state.mCoordGen = flags;
    GXSetTexCoordGen2(dstCoord, fn, src, mtx, normalize, postMtx);
  }
}
#endif

#if !defined(CGX_INLINE_HEADER) || RSTL_VERSION < RSTL_GM8P_00
CGX_INLINE void CGX::SetTexCoordGen_Compressed(GXTexCoordID dstCoord, uint flags) {
  STexState& state = sGXState.mTexStates[dstCoord];
  if (state.mCoordGen != flags) {
    GXTexMtx vm = static_cast< GXTexMtx >((flags >> 9 & 0x1F) + GX_TEXMTX0);
    GXPTTexMtx vp = static_cast< GXPTTexMtx >((flags >> 15 & 0x3F) + GX_PTTEXMTX0);
    state.mCoordGen = flags;
    GXSetTexCoordGen2(dstCoord, static_cast< GXTexGenType >(flags & 0xF),
                      static_cast< GXTexGenSrc >(flags >> 4 & 0x1F), vm,
                      static_cast< GXBool >(flags >> 14 & 1), vp);
  }
}
#endif
#endif

CGX_INLINE void CGX::SetNumIndStages(uchar num) {
  if (CGX_STATE.mNumIndStages != num) {
    CGX_STATE.mNumIndStages = num;

    GXSetNumIndStages(num);
  }
}

CGX_INLINE void CGX::SetArray(GXAttr attr, const void* data, uchar stride) {
#if RSTL_PRIME1
  uint idx = attr - GX_VA_POS;
  if (data == nullptr || sGXState.mArrayPtrs[idx] == data) {
#else
  if (data == nullptr) {
#endif

    return;
  }
#if RSTL_PRIME1
  sGXState.mArrayPtrs[idx] = data;
#else
  if (attr >= GX_VA_POS && attr < GX_POS_MTX_ARRAY) {
    uint idx = attr - GX_VA_POS;
    if (gpGXState->mArrayPtrs[idx] == data) {
      return;
    }
    gpGXState->mArrayPtrs[idx] = data;
  }
#endif

  GXSetArray(attr, data, stride);
}

#if RSTL_PRIME1
CGX_INLINE void CGX::CallDisplayList(const void* ptr, size_t size) {
  if (sGXState.mChanFlags != 0) {
    FlushState();
  }
  GXCallDisplayList(ptr, size);
}
#endif

CGX_INLINE void CGX::Begin(GXPrimitive prim, GXVtxFmt fmt, ushort numVtx) {
  if (CGX_STATE.mChanFlags != 0) {

    FlushState();
  }
  GXBegin(prim, fmt, numVtx);
}

CGX_INLINE void CGX::End() { GXEnd(); }

#if RSTL_PRIME1
CGX_INLINE void CGX::apply_fog() {
  static const GXColor black = {0, 0, 0, 0};
  GXSetFog(static_cast< GXFogType >(sGXState.mFogType), sGXState.mFogParams.mFogStartZ,
           sGXState.mFogParams.mFogEndZ, sGXState.mFogParams.mFogNearZ,
           sGXState.mFogParams.mFogFarZ,
           (sGXState.mBlendMode & (7 << 5)) == (GX_BL_ONE << 5)
               ? black
               : sGXState.mFogParams.mFogColor);
}
#endif

#if RSTL_PRIME1
CGX_INLINE void CGX::SetFog(GXFogType type, float startZ, float endZ, float nearZ, float farZ,
                            const GXColor& color) {
  sGXState.mFogType = type;
  sGXState.mFogParams.mFogStartZ = startZ;
  sGXState.mFogParams.mFogEndZ = endZ;
  sGXState.mFogParams.mFogNearZ = nearZ;
  sGXState.mFogParams.mFogFarZ = farZ;
  CopyGXColor(sGXState.mFogParams.mFogColor, color);
  apply_fog();
}

CGX_INLINE void CGX::SetLineWidth(uchar width, GXTexOffset offset) {
  ushort flags = width | offset << 8;
  if (flags != sGXState.mLineWidthAndOffset) {
    sGXState.mLineWidthAndOffset = flags;
    GXSetLineWidth(width, offset);
  }
}
#endif

#endif // _CGX_IMPL
