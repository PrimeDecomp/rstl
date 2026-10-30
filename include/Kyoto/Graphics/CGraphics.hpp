#ifndef _CGRAPHICS
#define _CGRAPHICS
#include "rstl/RstlVersions.h"

#if RSTL_PRIME1
#include "GameVersions.h"
#endif

#include "types.h"

#include "Kyoto/CTimeProvider.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Graphics/CLight.hpp"
#include "Kyoto/Graphics/CTevCombiners.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CMatrix4f.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector2i.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "rstl/reserved_vector.hpp"

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#include "dolphin/gx/GXFifo.h"
#include "dolphin/gx/GXStruct.h"
#include "dolphin/gx/GXTexture.h"
#include "dolphin/mtx/GeoTypes.h"
#else
#include "dolphin/gx.h"
#include "dolphin/mtx.h"
#endif

class COsContext;

enum ERglFogMode {
  kRFM_None = GX_FOG_NONE,

  kRFM_PerspLin = GX_FOG_PERSP_LIN,
  kRFM_PerspExp = GX_FOG_PERSP_EXP,
  kRFM_PerspExp2 = GX_FOG_PERSP_EXP2,
  kRFM_PerspRevExp = GX_FOG_PERSP_REVEXP,
  kRFM_PerspRevExp2 = GX_FOG_PERSP_REVEXP2,

  kRFM_OrthoLin = GX_FOG_ORTHO_LIN,
  kRFM_OrthoExp = GX_FOG_ORTHO_EXP,
  kRFM_OrthoExp2 = GX_FOG_ORTHO_EXP2,
  kRFM_OrthoRevExp = GX_FOG_ORTHO_REVEXP,
  kRFM_OrthoRevExp2 = GX_FOG_ORTHO_REVEXP2,
};

enum ERglTevStage {
  kTS_Stage0,
  kTS_Stage1,
  kTS_Stage2,
  kTS_Stage3,
  kTS_Stage4,
  kTS_Stage5,
  kTS_Stage6,
  kTS_Stage7,
  kTS_Stage8,
  kTS_Stage9,
  kTS_Stage10,
  kTS_Stage11,
  kTS_Stage12,
  kTS_Stage13,
  kTS_Stage14,
  kTS_Stage15,
  kTS_MaxStage,
};

enum ERglPrimitive {
  kP_Quads = GX_QUADS,
  kP_Triangles = GX_TRIANGLES,
  kP_TriangleStrip = GX_TRIANGLESTRIP,
  kP_TriangleFan = GX_TRIANGLEFAN,
  kP_Lines = GX_LINES,
  kP_LineStrip = GX_LINESTRIP,
  kP_Points = GX_POINTS,
};

enum ERglBlendMode {
  kBM_None = GX_BM_NONE,
  kBM_Blend = GX_BM_BLEND,
  kBM_Logic = GX_BM_LOGIC,
  kBM_Subtract = GX_BM_SUBTRACT,
  kBM_Max = GX_MAX_BLENDMODE,
};

enum ERglBlendFactor {
  kBF_Zero = GX_BL_ZERO,
  kBF_One = GX_BL_ONE,
  kBF_SrcColor = GX_BL_SRCCLR,
  kBF_InvSrcColor = GX_BL_INVSRCCLR,
  kBF_SrcAlpha = GX_BL_SRCALPHA,
  kBF_InvSrcAlpha = GX_BL_INVSRCALPHA,
  kBF_DstAlpha = GX_BL_DSTALPHA,
  kBF_InvDstAlpha = GX_BL_INVDSTALPHA,
  kBF_DstColor = GX_BL_DSTCLR,
  kBF_InvDstColor = GX_BL_INVDSTCLR,
};

enum ERglLogicOp {
  kLO_Clear = GX_LO_CLEAR,
  kLO_And = GX_LO_AND,
  kLO_RevAnd = GX_LO_REVAND,
  kLO_Copy = GX_LO_COPY,
  kLO_InvAnd = GX_LO_INVAND,
  kLO_NoOp = GX_LO_NOOP,
  kLO_Xor = GX_LO_XOR,
  kLO_Or = GX_LO_OR,
  kLO_Nor = GX_LO_NOR,
  kLO_Equiv = GX_LO_EQUIV,
  kLO_Inv = GX_LO_INV,
  kLO_RevOr = GX_LO_REVOR,
  kLO_InvCopy = GX_LO_INVCOPY,
  kLO_InvOr = GX_LO_INVOR,
  kLO_NAnd = GX_LO_NAND,
  kLO_Set = GX_LO_SET,
};

enum ERglAlphaFunc {
  kAF_Never = GX_NEVER,
  kAF_Less = GX_LESS,
  kAF_Equal = GX_EQUAL,
  kAF_LEqual = GX_LEQUAL,
  kAF_Greater = GX_GREATER,
  kAF_NEqual = GX_NEQUAL,
  kAF_GEqual = GX_GEQUAL,
  kAF_Always = GX_ALWAYS,
};

enum ERglAlphaOp {
  kAO_And = GX_AOP_AND,
  kAO_Or = GX_AOP_OR,
  kAO_Xor = GX_AOP_XOR,
  kAO_XNor = GX_AOP_XNOR,
  kAO_Max = GX_MAX_ALPHAOP,
};

enum ERglEnum {
  kE_Never = GX_NEVER,
  kE_Less = GX_LESS,
  kE_Equal = GX_EQUAL,
  kE_LEqual = GX_LEQUAL,
  kE_Greater = GX_GREATER,
  kE_NEqual = GX_NEQUAL,
  kE_GEqual = GX_GEQUAL,
  kE_Always = GX_ALWAYS,
};

enum ERglCullMode {
  kCM_None = GX_CULL_NONE,
  kCM_Front = GX_CULL_FRONT,
  kCM_Back = GX_CULL_BACK,
  kCM_All = GX_CULL_ALL,
};

enum ERglLight {
  kLight0,
  kLight1,
  kLight2,
  kLight3,
  kLight4,
  kLight5,
  kLight6,
  kLight7,
  kLightMax,
};

enum ERglTexOffset {
  kTO_Zero = GX_TO_ZERO,
  kTO_Sixteenth = GX_TO_SIXTEENTH,
  kTO_Eighth = GX_TO_EIGHTH,
  kTO_Fourth = GX_TO_FOURTH,
  kTO_Half = GX_TO_HALF,
  kTO_One = GX_TO_ONE,
};

struct CViewport {
  int mLeft;
  int mTop;
  int mWidth;
  int mHeight;
  float mHalfWidth;
  float mHalfHeight;
};

class CTexture;

// TODO
typedef struct {
  float x;
  float y;
} Vec2, *Vec2Ptr;

class CGraphics {
#if RSTL_PRIME1
  friend class CCubeRenderer;
#endif

public:
  class CRenderState {
  public:
    CRenderState();

    void Flush();
    void ResetFlushAll();
    int SetVtxState(const float* pos, const float* nrm, const uint* clr);

    // In map this takes two args, but x4 is unused?
    void Set(int v0) { x0_ = v0; }

  private:
    int x0_;
    int x4_;
  };

  class CProjectionState {
  public:
    CProjectionState(bool persp, float left, float right, float top, float bottom, float near,
                     float far)
    : mPersp(persp)
    , mLeft(left)
    , mRight(right)
    , mTop(top)
    , mBottom(bottom)
    , mNear(near)
    , mFar(far) {}

    bool IsPerspective() const { return mPersp; }
    float GetLeft() const { return mLeft; }
    float GetRight() const { return mRight; }
    float GetTop() const { return mTop; }
    float GetBottom() const { return mBottom; }
    float GetNear() const { return mNear; }
    float GetFar() const { return mFar; }

  private:
    bool mPersp;
    float mLeft;
    float mRight;
    // TODO: I think top/bottom are flipped
    float mTop;
    float mBottom;
    float mNear;
    float mFar;
  };

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  class CClippedScreenRect {
#else
  // Guessed name; clipped screen bounds and texture coordinates for four corners.
  class CClippedScreenQuad {
#endif

  public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    CClippedScreenRect() : mValid(false) {}
    CClippedScreenRect(const int x, const int y, const int width, const int height, const int texWidth, const float minU, const float maxU,
                       const float minV, const float maxV)
#else
    CClippedScreenQuad() : mValid(false) {}
    CClippedScreenQuad(int x, int y, int width, int texWidth, int height,
                       rstl::reserved_vector< CVector2f, 4 > texCoords)
#endif

    : mValid(true)
    , mX(x)
    , mY(y)
    , mWidth(width)
    , mHeight(height)
    , mTexWidth(texWidth)
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    , mMinU(minU)
    , mMaxU(maxU)
    , mMinV(minV)
    , mMaxV(maxV) {}
#else
    , mTexCoords(texCoords) {}
#endif

    bool IsValid() const { return mValid; }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    const int GetX() const { return mX; }
    const int GetY() const { return mY; }
    const int GetWidth() const { return mWidth; }
    const int GetHeight() const { return mHeight; }
    const int GetTexWidth() const { return mTexWidth; }
    float GetMinU() const { return mMinU; }
    float GetMaxU() const { return mMaxU; }
    float GetMinV() const { return mMinV; }
    float GetMaxV() const { return mMaxV; }
#else
    int GetX() const { return mX; }
    int GetY() const { return mY; }
    int GetWidth() const { return mWidth; }
    int GetHeight() const { return mHeight; }
    int GetTexWidth() const { return mTexWidth; }
    const CVector2f& GetTexCoord(int index) const { return mTexCoords[index]; }
#endif

  private:
    bool mValid;
    int mX;
    int mY;
    int mWidth;
    int mHeight;
    int mTexWidth;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    float mMinU;
    float mMaxU;
    float mMinV;
    float mMaxV;
#else
    rstl::reserved_vector< CVector2f, 4 > mTexCoords;
#endif

  };

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static bool Startup(const COsContext& osContext, uint fifoSize, void* fifoBase);
  static bool Startup(const COsContext& osContext, bool rgb60);
#else
  static bool Startup(const COsContext& osContext, bool progressive);
  static void InitGraphicsFifo(GXFifoObj* obj, void* base, uint fifoSize);
#endif

  static GXTexRegion* TexRegionCallback(const GXTexObj* obj, GXTexMapID id);
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_GM8J_00
  static void InitGraphicsFifo(GXFifoObj* obj, void* base, uint fifoSize);
#endif
#endif

  static void InitGraphicsVariables();
  static void Shutdown();
  static void InitGraphicsDefaults();
#if RSTL_PRIME1
  static void ConfigureFrameBuffer(const COsContext& osContext);
#endif

  static void ConfigureFrameBuffer();
#if RSTL_PRIME1
  static void ConfigureVideoMode(bool initial, bool rgb60); // Guessed name. PAL arena/video setup.
#endif

  static void EnableLight(ERglLight light);
  static void LoadLight(ERglLight light, const CLight& info);
  static void SetLightState(uchar lights);
#if RSTL_PRIME1
  static uint GetLightMask() { return mLightActive; }
#endif

  static void SetViewMatrix();
  static void SetScissor(int left, int bottom, int width, int height);
  static void SetLineWidth(float w, ERglTexOffset offs);
  static void ClearBackAndDepthBuffers();

  static void SetIdentityViewPointMatrix();
  static void SetIdentityModelMatrix();
  static void SetViewport(int left, int bottom, int width, int height);
  static void SetPerspective(float fovy, float aspect, float znear, float zfar);
  static void SetCopyClear(const CColor& color, float depth);
  static void SetClearColor(const CColor& color);
#if !RSTL_PRIME1
  static CColor GetClearColor();         // Guessed name.
  static int GetViewportTop(int bottom); // Guessed name.
#endif

  static void SetDepthRange(float near, float far);
  static void FlushProjection();
  static void SetDefaultVtxAttrFmt();
  static CMatrix4f GetPerspectiveProjectionMatrix();
  static CMatrix4f CalculatePerspectiveMatrix(float fovy, float aspect, float znear, float zfar);
  static void ResetGfxStates();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static void LoadDolphinSpareTexture(int width, int height, GXTexFmt fmt, void* data,
#else
  static void LoadDolphinSpareTexture(int width, int height, GXTexFmt fmt, const void* data,
#endif

                                      GXTexMapID texId);
  static void LoadDolphinSpareTexture(int width, int height, GXCITexFmt fmt, GXTlut tlut,
                                      void* data, GXTexMapID texId);
  static void TickRenderTimings();
  static const CProjectionState& GetProjectionState();
  static void SetProjectionState(const CProjectionState& proj);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static CClippedScreenRect ClipScreenRectFromVS(const CVector3f& p1, const CVector3f& p2,
#else
  // Guessed name; projects all four model-space corners independently.
  static CClippedScreenQuad ClipScreenQuadFromMS(const CVector3f& p1, const CVector3f& p2,
                                                 const CVector3f& p3, const CVector3f& p4,
#endif

                                                 ETexelFormat fmt);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static CClippedScreenRect ClipScreenRectFromMS(const CVector3f& p1, const CVector3f& p2,
#else
  // Guessed name; the view-space counterpart of ClipScreenQuadFromMS.
  static CClippedScreenQuad ClipScreenQuadFromVS(const CVector3f& p1, const CVector3f& p2,
                                                 const CVector3f& p3, const CVector3f& p4,
#endif

                                                 ETexelFormat fmt);
  static CVector2i ProjectPoint(const CVector3f& point);

#if RSTL_PRIME1
  static const GXRenderModeObj& GetRenderMode() { return mRenderModeObj; }
#endif

  static float GetDepthNear() { return mDepthNear; }
  static float GetDepthFar() { return mDepthFar; }

  static bool IsBeginSceneClearFb() { return mIsBeginSceneClearFb; }
  static void SetIsBeginSceneClearFb(bool);
  static void BeginScene();
  static void EndScene();
  static void SwapBuffers();
  static void SetTevOp(ERglTevStage stage, const CTevCombiners::CTevPass& pass);
#if !RSTL_PRIME1
  static void SetTevRegisterColor(int index, const CColor& color); // Guessed name
#endif

  static void StreamBegin(ERglPrimitive primitive);
  static void StreamColor(uint color);
  static void StreamColor(float r, float g, float b, float a);
  static void StreamColor(const CColor& color);
  static void StreamTexcoord(float u, float v);
  static void StreamTexcoord(const CVector2f& uv);
  static void StreamVertex(float x, float y, float z);
  static void StreamVertex(const CVector3f& vtx);
  static void StreamVertex(const float* vtx);
#if !RSTL_PRIME1
  // Guessed name. Delay each streamed vertex by eight instructions.
  static void DelayStreamVertex() {
    #ifdef __MWERKS__
    asm {
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
    }
    #endif
  }
#endif

  static void StreamNormal(const float* nrm);
  static void StreamEnd();
  static void Render2D(const CTexture& tex, int x, int y, int w, int h, const CColor& col);
#if !RSTL_PRIME1
  static void Render2D(const CTexture* tex, int x, int y, int w, int h, const CColor& col);
  // Names and signatures from the Prime demo map. Unreferenced and dead-stripped in Echoes retail.
#endif

  static bool BeginRender2D(const CTexture& tex);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static void DoRender2D(const CTexture& tex, int x, int y, int srcX, int srcY,
                         int srcW, int srcH, int w, int h, const CColor& col);
  static void EndRender2D(bool lightsWereOn);
#else
  static void EndRender2D(bool restoreLights);
  static void DoRender2D(const CTexture& tex, int x, int y, int texX, int texY, int texW, int texH,
                         int w, int h, const CColor& col);
#endif

  static void DrawPrimitive(ERglPrimitive primitive, const float* pos, const CVector3f& normal,
                            const CColor& col, int numVerts);

  static void VideoPreCallback(u32 retraceCount);
  static void VideoPostCallback(u32 retraceCount);

  static const CViewport& GetViewport() { return mViewport; }
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_GM8E_02
  static float GetPixelAspectRatio() { return mPixelAspectRatio; }
#endif
  static int GetViewportWidth() { return mViewport.mWidth; }
  static int GetViewportHeight() { return mViewport.mHeight; }
#endif

  static void GetViewport(int& left, int& top, int& width, int& height) {
    left = mViewport.mLeft;
    top = mViewport.mTop;
    width = mViewport.mWidth;
    height = mViewport.mHeight;
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static const CVector3f& GetViewPoint() { return mViewPoint; }
#else
  static float GetPixelAspectRatio() { return mPixelAspectRatio; }
  static const GXRenderModeObj& GetRenderMode() { return mRenderModeObj; }
  static bool Is50Hz() { return sIs50Hz; } // Guessed name.
  static CVector3f GetViewPoint() { return mViewMatrix.GetTranslation(); }
#endif

  static const CTransform4f& GetViewMatrix() { return mViewMatrix; }
#if !RSTL_PRIME1
  static const Mtx& GetCameraMtx() { return mCameraMtx; }
#endif

  static const CTransform4f& GetModelMatrix() { return mModelMatrix; }
#if !RSTL_PRIME1
  static uchar GetLightMask() { return mLightActive; }
#endif

  static void SetViewPointMatrix(const CTransform4f&);
  static void SetBrightness(float b) { mBrightness = b; }
#if RSTL_PRIME1
  static float GetBrightness() { return mBrightness; }
#endif

  static void SetOrtho(float left, float right, float top, float bottom, float znear, float zfar);

  static float GetSecondsMod900();
  static float GetFPS();
  static void SetExternalTimeProvider(CTimeProvider* provider);
  static void DisableAllLights();

  static void SetAmbientColor(const CColor&);
  static void SetFog(ERglFogMode mode, float startz, float endz, const CColor& color);

  static void SetModelMatrix(const CTransform4f& xf);
#if !RSTL_PRIME1
  static void SetUseNormalMatrix(bool enabled);      // Guessed name
  static void SetUseStreamVertexDelay(bool enabled); // Guessed name
#endif

  static void SetAlphaCompare(ERglAlphaFunc comp0, uchar ref0, ERglAlphaOp op, ERglAlphaFunc comp1,
                              uchar ref1);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static void SetDepthWriteMode(const bool test, ERglEnum comp, const bool write);
#else
  static void SetDepthWriteMode(bool test, ERglEnum comp, bool write);
#endif

  static void SetBlendMode(ERglBlendMode mode, ERglBlendFactor src, ERglBlendFactor dst,
                           ERglLogicOp op);
#if RSTL_PRIME1
  static ERglCullMode GetCullMode() { return mCullMode; }
#endif

  static void SetCullMode(ERglCullMode cullMode);
  static void SetTevStates(uchar);

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static void SetUseVideoFilter(const bool b);
#else
  static void SetUseVideoFilter(bool b);
#endif

  static bool GetDolphinLastFrameAbove() { return mLastFrameUsedAbove; }
  static bool GetUseVideoFilter();
  static int GetFrameCounter();
#if !RSTL_PRIME1
  static const CTransform4f& GetGXModelView() {
    return *reinterpret_cast< const CTransform4f* >(mGxModelView);
  }
  static const Mtx& GetGXModelViewInvXpose() { return mGxModelViewInvXpose; }
  static void* GetDolphinSpareBuffer() { return mpSpareBuffer; }
  static int GetSpareBufferSize() { return mSpareBufferSize; }
  static GXTexRegion* GetSpareTextureRegion() { return &mTexRegions[0]; }
#endif

  static void SetProgressiveMode(bool b);
#if !RSTL_PRIME1
  static void SetPAL60(bool enabled); // Guessed name; PAL60 (60 Hz) video-mode switch.
#endif

  static bool GetProgressiveMode();
  static bool CanSetProgressiveMode();
  static bool GetProgressiveDefault();

#if RSTL_PRIME1
  static void* GetDolphinSpareBuffer() { return mpSpareBuffer; }
  static void InvalidateSpareTexture();
  static int GetSpareBufferSize() { return mSpareBufferSize; }
#endif

  // Screen Position
  static void GetScreenPosition(int* stretch, int* xOffset, int* yOffset);
  static void SetScreenPosition(int stretch, int xOffset, int yOffset);
#if !RSTL_PRIME1
  static const CTevCombiners::CTevPass& EnvModulate() { return kEnvModulate; }
#endif

  static const CTevCombiners::CTevPass& kEnvPassthru;
  static CTevCombiners::CTevPass kEnvModulateConstColor;
  static CTevCombiners::CTevPass kEnvConstColor;
  static CTevCombiners::CTevPass kEnvModulate;
  static CTevCombiners::CTevPass kEnvDecal;
  static CTevCombiners::CTevPass kEnvBlend;
  static CTevCombiners::CTevPass kEnvReplace;
  static CTevCombiners::CTevPass kEnvModulateAlpha;
  static CTevCombiners::CTevPass kEnvModulateColor;
  static CTevCombiners::CTevPass kEnvModulateColorByAlpha;
#if RSTL_PRIME1
  static CRenderState sRenderState;
#endif

  static const GXTexMapID kSpareBufferTexMapID;

private:
#if !RSTL_PRIME1
  static void ConfigureVideo(bool initial, uchar progressive); // Guessed name
#endif

  static void UpdateVertexDataStream();
  static void ResetVertexDataStream(bool initial);
  static void FlushStream();
  static void FullRender();
#if !RSTL_PRIME1
  static void FullRenderWithVertexDelay(); // Guessed name
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static VecPtr vtxBuffer;
  static VecPtr nrmBuffer;
  static Vec2Ptr txtBuffer0;
  static Vec2Ptr txtBuffer1;
  static uint* clrBuffer;
#else
  static CRenderState mRenderState;
  static VecPtr mVertexBuffer;
  static VecPtr mNormalBuffer;
  static Vec2Ptr mTexCoordBuffer0;
  static Vec2Ptr mTexCoordBuffer1;
  static uint* mColorBuffer;
#endif

  static bool mJustReset;
  static ERglCullMode mCullMode;
  static int mNumLightsActive;
  static float mDepthNear;
  static VecPtr mpVtxBuffer;
  static VecPtr mpNrmBuffer;
  static Vec2Ptr mpTxtBuffer0;
  static Vec2Ptr mpTxtBuffer1;
  static uint* mpClrBuffer;
  static int mNumPrimitives;
  static int mFrameCounter;
  static float mFramesPerSecond;
  static float mLastFramesPerSecond;
  static int mNumBreakpointsWaiting;
  static int mFlippingState;
  static bool mLastFrameUsedAbove;
  static bool mInterruptLastFrameUsedAbove;
  static uchar mLightActive;
  static uchar mLightsWereOn;
  static void* mpFrameBuf1;
  static void* mpFrameBuf2;
  static void* mpCurrenFrameBuf;
  static int mSpareBufferSize;
  static void* mpSpareBuffer;
  static int mSpareBufferTexCacheSize;
  static GXTexRegionCallback mGXDefaultTexRegionCallback;
  static void* mpFifo;
  static GXFifoObj* mpFifoObj;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_GM8J_00
  static uint mFifoSize;
#endif
#else
  static uint mFifoSize;
#endif

  static uint mRenderTimings;
  static float mSecondsMod900;
  static CTimeProvider* mpExternalTimeProvider;
  static int mScreenStretch;
  static int mScreenPositionX;
  static int mScreenPositionY;

  static CVector3f kDefaultPositionVector;
  static CVector3f kDefaultDirectionVector;
  static CProjectionState mProj;
  static CTransform4f mViewMatrix;
  static CTransform4f mModelMatrix;
  static CColor mClearColor;
#if RSTL_PRIME1
  static CVector3f mViewPoint;
#endif

  static CViewport mViewport;
  static ELightType mLightTypes[8];
  static GXLightObj mLightObj[8];
  static GXTexRegion mTexRegions[GX_MAX_TEXMAP];
  static GXTexRegion mTexRegionsCI[GX_MAX_TEXMAP / 2];
  static GXRenderModeObj mRenderModeObj;
#if !RSTL_PRIME1
  static bool sIs50Hz; // Guessed name; shared video timing and splash-coordinate flag.
#endif

  static Mtx mGXViewPointMatrix;
#if RSTL_PRIME1
  static Mtx mGXModelMatrix;
#endif

  static Mtx mGxModelView;
#if !RSTL_PRIME1
  static Mtx mGxModelViewInvXpose;
#endif

  static Mtx mCameraMtx;

  // .sdata
  static bool mIsBeginSceneClearFb;
  static ERglEnum mDepthFunc;
  static ERglPrimitive mCurrentPrimitive;
  static float mDepthFar;
  static u32 mClearDepthValue; // = GX_MAX_Z24
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_GM8E_02
  static float mPixelAspectRatio;
#endif
#else
  static float mPixelAspectRatio;
  static bool mUseNormalMatrix;
#endif

  static bool mIsGXModelMatrixIdentity;
  static bool mFirstFrame;
  static bool mUseVideoFilter;
  static float mBrightness;
};

#if !RSTL_PRIME1
NESTED_CHECK_SIZEOF(CGraphics, CClippedScreenQuad, 0x3c)
#endif

#endif // _CGRAPHICS
