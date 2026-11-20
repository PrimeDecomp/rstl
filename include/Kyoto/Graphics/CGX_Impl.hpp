#ifndef _CGX_IMPL
#define _CGX_IMPL

#include "rstl/RstlVersions.h"

#include "Kyoto/Graphics/CGX.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "dolphin/gx.h"
#include "Kyoto/Platform.h"

// TODO: figure out why MSVC doesn't like this
#if !defined(TARGET_PC) || defined(CGX_CPP) || CLANGD

// CGX.cpp defines CGX_INLINE empty and includes this file to define these functions out of line.
// Other includers get inline definitions of the rest: the setters only where
// RSTL_KYOTO_CGX_INLINE_SETTERS, and CGX_IMPL_PRIME's only in Prime.
#ifndef CGX_INLINE
#define CGX_INLINE inline
#endif

#if defined(CGX_CPP) || RSTL_KYOTO_CGX_INLINE_SETTERS
#define CGX_IMPL_SETTERS 1
#else
#define CGX_IMPL_SETTERS 0
#endif

#if defined(CGX_CPP) || RSTL_PRIME1
#define CGX_IMPL_PRIME 1
#else
#define CGX_IMPL_PRIME 0
#endif

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

#if CGX_IMPL_SETTERS
CGX_INLINE void CGX::SetChanCtrl(EChannelId channel, GXBool enable, GXColorSrc ambSrc,
                                 GXColorSrc matSrc, GXLightID lights, GXDiffuseFn diffFn,
                                 GXAttnFn attnFn) {
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

CGX_INLINE void CGX::SetChanCtrl_Compressed(EChannelId channel, GXLightID lights, uint ctrl) {
  ushort& state = CGX_STATE.mChanCtrls[channel];
  ushort prevFlags = CGX_STATE.mPrevChanCtrls[channel];
  uint flags = lights != 0 ? ctrl | MaskAndShiftLeft(lights, 0xFF, 3) : ctrl & ~1;
  state = flags;
  CGX_STATE.mChanFlags =
      ((flags != prevFlags) << (channel + 1)) | (CGX_STATE.mChanFlags & ~(1 << (channel + 1)));
}
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

inline void CGX::SetTevKColor(GXTevKColorID id, const CColor& color) {
  if (!(*reinterpret_cast< const CColor* >(&CGX_STATE.mKColors[id]) == color)) {
    CopyGXColor(CGX_STATE.mKColors[id], color.GetGXColor());
    GXSetTevKColor(id, color.GetGXColor());
  }
}

#if RSTL_KYOTO_CGX_CORRUPTION
CGX_INLINE void CGX::SetTevSwapMode(GXTevStageID stage, GXTevSwapSel rasSel, GXTevSwapSel texSel) {
  STevState& state = CGX_STATE.mTevStates[stage];
  const uchar flags = rasSel | (texSel << 2);
  if (flags != state.mSwapMode) {
    GXSetTevSwapMode(stage, rasSel, texSel);
    state.mSwapMode = flags;
  }
}

CGX_INLINE void CGX::SetTevSwapModeTable(GXTevSwapSel table, GXTevColorChan red,
                                         GXTevColorChan green, GXTevColorChan blue,
                                         GXTevColorChan alpha) {
  const uchar flags = red | (green << 2) | (blue << 4) | (alpha << 6);
  if (flags != CGX_STATE.mTevSwapTables[table]) {
    GXSetTevSwapModeTable(table, red, green, blue, alpha);
    CGX_STATE.mTevSwapTables[table] = flags;
  }
}
#endif

#if CGX_IMPL_SETTERS
CGX_INLINE void CGX::SetTevColorIn(GXTevStageID stageId, GXTevColorArg a, GXTevColorArg b,
                                   GXTevColorArg c, GXTevColorArg d) {
  const uint flags = (a & 0x1F) | ((b & 0x1F) << 5) | ((c & 0x1F) << 10) | ((d & 0x1F) << 15);
  STevState& state = CGX_STATE.mTevStates[stageId];
  if (flags != state.mColorInArgs) {
    state.mColorInArgs = flags;
    GXSetTevColorIn(stageId, a, b, c, d);
  }
}

CGX_INLINE void CGX::SetTevColorIn_Compressed(GXTevStageID stageId, uint flags) {
  STevState& state = CGX_STATE.mTevStates[stageId];
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
  STevState& state = CGX_STATE.mTevStates[stageId];
  if (flags != state.mAlphaInArgs) {
    state.mAlphaInArgs = flags;
    GXSetTevAlphaIn(stageId, a, b, c, d);
  }
}

CGX_INLINE void CGX::SetTevAlphaIn_Compressed(GXTevStageID stageId, uint flags) {
  STevState& state = CGX_STATE.mTevStates[stageId];
  if (flags != state.mAlphaInArgs) {
    state.mAlphaInArgs = flags;
    GXSetTevAlphaIn(stageId, static_cast< GXTevAlphaArg >(ShiftRightAndMask(flags, 0x1F, 0)),
                    static_cast< GXTevAlphaArg >(ShiftRightAndMask(flags, 0x1F, 5)),
                    static_cast< GXTevAlphaArg >(ShiftRightAndMask(flags, 0x1F, 10)),
                    static_cast< GXTevAlphaArg >(ShiftRightAndMask(flags, 0x1F, 15)));
  }
}

CGX_INLINE void CGX::SetTevColorOp(GXTevStageID stageId, GXTevOp op, GXTevBias bias,
                                   GXTevScale scale, GXBool clamp, GXTevRegID outReg) {
  const uint flags = MaskAndShiftLeft(op, 0xF, 0) | MaskAndShiftLeft(bias, 3, 4) |
                     MaskAndShiftLeft(scale, 3, 6) | MaskAndShiftLeft(clamp, 1, 8) |
                     MaskAndShiftLeft(outReg, 3, 9);
  STevState& state = CGX_STATE.mTevStates[stageId];
  if (flags != state.mColorOps) {
    state.mColorOps = flags;
    GXSetTevColorOp(stageId, op, bias, scale, clamp, outReg);
  }
}
#endif

#if CGX_IMPL_PRIME
CGX_INLINE void CGX::SetTevColorOp_Compressed(GXTevStageID stageId, uint flags) {
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
#endif

#if CGX_IMPL_SETTERS
CGX_INLINE void CGX::SetTevAlphaOp(GXTevStageID stageId, GXTevOp op, GXTevBias bias,
                                   GXTevScale scale, GXBool clamp, GXTevRegID outReg) {
  uint flags = MaskAndShiftLeft(op, 0xF, 0) | MaskAndShiftLeft(bias, 3, 4) |
               MaskAndShiftLeft(scale, 3, 6) | MaskAndShiftLeft(clamp, 1, 8) |
               MaskAndShiftLeft(outReg, 3, 9);
  STevState& state = CGX_STATE.mTevStates[stageId];
  if (flags != state.mAlphaOps) {
    state.mAlphaOps = flags;
    GXSetTevAlphaOp(stageId, op, bias, scale, clamp, outReg);
  }
}
#endif

#if CGX_IMPL_PRIME
CGX_INLINE void CGX::SetTevAlphaOp_Compressed(GXTevStageID stageId, uint flags) {
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
  STevState& state = CGX_STATE.mTevStates[stageId];
  uint flags = (texCoord & 0xFF) | ((texMap & 0xFF) << 8) | ((color & 0xFF) << 16);
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

CGX_INLINE void CGX::SetZMode(const TZModeFlag compareEnable, GXCompare func,
                              const TZModeFlag updateEnable) {
  uchar flags = MaskAndShiftLeft(compareEnable, 0xFF, 0) | MaskAndShiftLeft(updateEnable, 0xFF, 1) |
                MaskAndShiftLeft(func, 0xFF, 2);
  if (flags != CGX_STATE.mZmode) {
    CGX_STATE.mZmode = flags;
    GXSetZMode(compareEnable, func, updateEnable);
  }
}

// The other games define SetAlphaCompare later in CGX.cpp, with an always-pass fast path.
#if RSTL_PRIME1 && CGX_IMPL_SETTERS
CGX_INLINE void CGX::SetAlphaCompare(GXCompare comp0, uchar ref0, GXAlphaOp op, GXCompare comp1,
                                     uchar ref1) {
  uint flags = MaskAndShiftLeft(comp0, 7, 0) | MaskAndShiftLeft(ref0, 0xFF, 3) |
               MaskAndShiftLeft(op, 7, 11) | MaskAndShiftLeft(comp1, 7, 14) |
               MaskAndShiftLeft(ref1, 0xFF, 17);
  if (CGX_STATE.mAlphaCompare != flags) {
    CGX_STATE.mAlphaCompare = flags;
    GXSetAlphaCompare(comp0, ref0, op, comp1, ref1);
    GXSetZCompLoc(comp0 == GX_ALWAYS);
  }
}
#endif

#if CGX_IMPL_SETTERS || !RSTL_PRIME1
CGX_INLINE void
CGX::SetTevIndirect(RSTL_PRIME1_CONST GXTevStageID stageId,
                    RSTL_PRIME1_CONST GXIndTexStageID indStage,
                    RSTL_PRIME1_CONST GXIndTexFormat fmt, RSTL_PRIME1_CONST GXIndTexBiasSel biasSel,
                    RSTL_PRIME1_CONST GXIndTexMtxID mtxSel, RSTL_PRIME1_CONST GXIndTexWrap wrapS,
                    RSTL_PRIME1_CONST GXIndTexWrap wrapT, GXBool addPrev,
                    RSTL_PRIME1_CONST GXBool indLod, RSTL_PRIME1_CONST GXIndTexAlphaSel alphaSel) {
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
#endif

CGX_INLINE void CGX::SetTevDirect(GXTevStageID stageId) {
  STevState& state = CGX_STATE.mTevStates[stageId];
  if (state.mIndFlags != 0) {
    state.mIndFlags = 0;
    GXSetTevDirect(stageId);
  }
}

#if CGX_IMPL_SETTERS
CGX_INLINE void CGX::SetTexCoordGen(GXTexCoordID dstCoord, GXTexGenType fn, GXTexGenSrc src,
                                    GXTexMtx mtx, GXBool normalize, GXPTTexMtx postMtx) {
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
  uint flags = (fn & 0xF) | ((src & 0x1F) << 4) | ((vm & 0x1F) << 9) | ((normalize & 1) << 14) |
               ((vp & 0x3F) << 15);
  if (state.mCoordGen != flags) {
    state.mCoordGen = flags;
    GXSetTexCoordGen2(dstCoord, fn, src, mtx, normalize, postMtx);
  }
}

CGX_INLINE void CGX::SetTexCoordGen_Compressed(GXTexCoordID dstCoord, uint flags) {
  STexState& state = CGX_STATE.mTexStates[dstCoord];
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

CGX_INLINE void CGX::SetNumIndStages(uchar num) {
  if (CGX_STATE.mNumIndStages != num) {
    CGX_STATE.mNumIndStages = num;
    GXSetNumIndStages(num);
  }
}

CGX_INLINE void CGX::SetArray(GXAttr attr, const void* data,
                              uchar stride IF_TARGET_PC(, size_t size, bool le)) {
#if RSTL_PRIME1
  uint idx = attr - GX_VA_POS;
  if (data == nullptr || IF_TARGET_PC_OR(false, CGX_STATE.mArrayPtrs[idx] == data)) {
    return;
  }
  CGX_STATE.mArrayPtrs[idx] = data;
#else
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
    if (IF_TARGET_PC_OR(false, CGX_STATE.mArrayPtrs[idx] == data)) {
      return;
    }
    CGX_STATE.mArrayPtrs[idx] = data;
  }
#endif
#endif

#if defined(TARGET_PC)
  GXSetArray(attr, data, size, stride, le);
#else
  GXSetArray(attr, const_cast< void* >(data), stride);
#endif
}

// The other games define CallDisplayList later in CGX.cpp, and the prototype Begin and End too.
#if RSTL_PRIME1
CGX_INLINE void CGX::CallDisplayList(const void* ptr, size_t size) {
  if (CGX_STATE.mChanFlags != 0) {
    FlushState();
  }
  GXCallDisplayList(const_cast< void* >(ptr), size);
}
#endif

#if !RSTL_KYOTO_CGX_CORRUPTION
CGX_INLINE void CGX::Begin(GXPrimitive prim, GXVtxFmt fmt, ushort numVtx) {
  if (CGX_STATE.mChanFlags != 0) {
    FlushState();
  }
  GXBegin(prim, fmt, numVtx);
}

CGX_INLINE void CGX::End() { GXEnd(); }
#endif

inline void CGX::apply_fog() {
  static const GXColor black = {0, 0, 0, 0};
  GXSetFog(static_cast< GXFogType >(CGX_STATE.mFogType), CGX_STATE.mFogParams.mFogStartZ,
           CGX_STATE.mFogParams.mFogEndZ, CGX_STATE.mFogParams.mFogNearZ,
           CGX_STATE.mFogParams.mFogFarZ,
           (CGX_STATE.mBlendMode & (7 << 5)) == (GX_BL_ONE << 5) ? black
                                                                 : CGX_STATE.mFogParams.mFogColor);
}

#if CGX_IMPL_PRIME
CGX_INLINE void CGX::SetFog(GXFogType type, float startZ, float endZ, float nearZ, float farZ,
                            const GXColor& color) {
  CGX_STATE.mFogType = type;
  CGX_STATE.mFogParams.mFogStartZ = startZ;
  CGX_STATE.mFogParams.mFogEndZ = endZ;
  CGX_STATE.mFogParams.mFogNearZ = nearZ;
  CGX_STATE.mFogParams.mFogFarZ = farZ;
  CopyGXColor(CGX_STATE.mFogParams.mFogColor, color);
  apply_fog();
}

CGX_INLINE void CGX::SetLineWidth(uchar width, GXTexOffset offset) {
  ushort flags = width | offset << 8;
  if (flags != CGX_STATE.mLineWidthAndOffset) {
    CGX_STATE.mLineWidthAndOffset = flags;
    GXSetLineWidth(width, offset);
  }
}
#endif

#undef CGX_IMPL_SETTERS
#undef CGX_IMPL_PRIME

#endif // !defined(TARGET_PC) || defined(CGX_CPP)

#endif // _CGX_IMPL
