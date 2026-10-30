#ifndef _CELEMENTGEN
#define _CELEMENTGEN
#include "rstl/RstlVersions.h"

#include "types.h"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Graphics/CLight.hpp"
#include "Kyoto/Math/CAABox.hpp"

#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Particles/CParticleGen.hpp"
#include "Kyoto/Particles/IElement.hpp"

#include "Kyoto/TToken.hpp"
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_R3ME_00

namespace rstl {
RSTL_DECLARE_TRIVIALLY_DESTRUCTIBLE(CMatrix3f)
} // namespace rstl

#endif
#endif

class CGenDescription;
class CModVectorElement;

#if RSTL_PRIME1
#ifdef __MWERKS__
#pragma cpp_extensions on
#endif
#endif

class CElementGen : public CParticleGen {
public:
#if RSTL_PRIME1
  struct CParticleListItem {
    ushort mPartIdx;
    CVector3f mViewPoint;

    explicit CParticleListItem(short partIdx, const CVector3f& viewPoint)
    : mPartIdx(partIdx), mViewPoint(viewPoint) {}
  };

  struct CTexturedParticleListItem {
    ushort mTexMapIdx;
    ushort mPartIdx;
    CVector3f mViewPoint;

    explicit CTexturedParticleListItem(short texMapIdx, short partIdx, const CVector3f& viewPoint)
    : mTexMapIdx(texMapIdx), mPartIdx(partIdx), mViewPoint(viewPoint) {}
  };
#endif

  enum EModelOrientationType {
    kMOT_Normal,
    kMOT_One,
  };
  enum EOptionalSystemFlags {
    kOSF_None,
    kOSF_One,
    kOSF_Two,
#if !RSTL_PRIME1
    kOSF_DisableBounds = 4, // Guessed name.
#endif

  };
  enum LightType {
    kLT_None = 0,
    kLT_Custom = 1,
    kLT_Directional = 2,
    kLT_Spot = 3,
  };
  struct CParticle {
    int mEndFrame;
    CVector3f mPos;
    CVector3f mPrevPos;
    CVector3f mVel;
    int mStartFrame;
#if RSTL_PRIME1
    union {
      float mLineLengthOrSize;
      uint mLineLengthOrSizeBits;
    };
    union {
      float mLineWidthOrRota;
      uint mLineWidthOrRotaBits;
    };
#else
    float mLineLengthOrSize;
    float mLineWidthOrRota;
#endif

    CColor mColor;

    CParticle()
    : mPos(CVector3f::Zero())
    , mPrevPos(mPos)
    , mVel(mPrevPos)
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    , mColor(static_cast< u8 >(0xFF), 0x00, 0xFF, 0xFF) {}
#else
    , mColor(static_cast< uchar >(255), 0, 255, 255) {}
#endif

  };
  // Guessed type name, based on Prime; Echoes adds ADV9 and VAV1/VAV2/VAV3.
  struct CAdvancedValues {
    float mValues[
#if RSTL_PRIME1
        8
#else
        9
#endif
    ];
  };

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CElementGen();
#else
  // Guessed names for the sorting record and model-render state.
  struct CParticleListItem {
    ushort mPartIdx;
    CVector3f mViewPoint;
  };
  // Guessed name, correlated with Prime's multi-texture sorting record.
  struct CTexturedParticleListItem {
    ushort mTexMapIdx;
    ushort mPartIdx;
    CVector3f mViewPoint;
  };
  struct SModelRenderState {
    bool mModulateAlpha;
    bool mConstantUV;
    bool mConstantIndirectUV;
    SUVElementSet mUV;
    SUVElementSet mIndirectUV;

    SModelRenderState() : mConstantUV(true), mConstantIndirectUV(true) {
      mModulateAlpha = false;
      mUV.xMax = 1.f;
      mUV.yMax = 1.f;
      mUV.xMin = 0.f;
      mUV.yMin = 0.f;
      mIndirectUV.xMax = 1.f;
      mIndirectUV.yMax = 1.f;
      mIndirectUV.xMin = 0.f;
      mIndirectUV.yMin = 0.f;
    }
  };
#endif

  CElementGen(TToken< CGenDescription >, EModelOrientationType = kMOT_Normal,
              EOptionalSystemFlags = kOSF_One);
  // CParticleGen
  ~CElementGen() override;

  virtual const bool Update(double) override;
  virtual void Render() override;
  virtual void SetOrientation(const CTransform4f& orientation) override;
  virtual void SetTranslation(const CVector3f& translation) override;
  virtual void SetGlobalOrientation(const CTransform4f& orientation) override;
  virtual void SetGlobalTranslation(const CVector3f& translation) override;
  virtual void SetGlobalScale(const CVector3f& scale) override;
  virtual void SetLocalScale(const CVector3f& scale) override;
// TODO: verify that this Prime/Echoes difference is real
  virtual void SetParticleEmission(RSTL_PRIME1_CONST bool emission) override;

  virtual void SetModulationColor(const CColor& col) override;
  virtual void SetGeneratorRate(float rate) override;
#if !RSTL_PRIME1
  virtual const CVector3f& GetTranslation() const override { return mTranslation; }
  virtual const CVector3f& GetGlobalTranslation() const override { return mGlobalTranslation; }
#endif

  virtual const CTransform4f& GetOrientation() const override { return mOrientation; }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  virtual const CVector3f& GetTranslation() const override { return mTranslation; }
  virtual const CTransform4f& GetGlobalOrientation() const override;
  virtual const CVector3f& GetGlobalTranslation() const override;
#else
  virtual const CTransform4f& GetGlobalOrientation() const override { return mGlobalOrientation; }
#endif

  virtual const CVector3f& GetGlobalScale() const override { return mGlobalScale; }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  virtual float GetGeneratorRate() const override;
  virtual bool GetParticleEmission() const override;
#else
  virtual bool GetParticleEmission() const override { return mParticleEmission; }
#endif

  virtual const CColor& GetModulationColor() const override;
#if !RSTL_PRIME1
  float GetGeneratorRate() const override;
  int GetEmitterTime() const override { return mCurFrame; }

  int GetSystemCount() const override;
#endif

  virtual bool IsSystemDeletable() const override;
  virtual rstl::optional_object< CAABox > GetBounds() const override;
  virtual int GetParticleCount() const override { return mActiveParticleCount; }
  virtual bool SystemHasLight() const override;
  virtual CLight GetLight() const override;
  virtual void DestroyParticles() override;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  virtual uint Get4CharId() const override;
  int GetMaxParticles() const { return mMAXP; }
  void SetZTest(bool enabled) { mZTest = enabled; }
  rstl::vector< CParticle >& Particles() { return mParticles; }
  const rstl::vector< CParticle >& GetParticles() const { return mParticles; }
  int GetEmitterTime() const;
  int GetSystemCount();
#else
  virtual uint Get4CharId() const override { return 'PART'; }
#endif

  void EndLifetime();
  void ForceParticleCreation(int count);

  bool InternalUpdate(double dt);
  void UpdateLightParameters();
  void UpdateAdvanceAccessParameters(int particleIndex, int particleFrame);
  bool UpdateVelocitySource(int sourceIndex, int particleFrame, CParticle& particle
#if !RSTL_PRIME1
                            ,
                            const CVector3f& scaledTranslation
#endif
  );
  void UpdateExistingParticles();
  void CreateNewParticles(int count);
  void UpdatePSTranslationAndOrientation();
#if RSTL_PRIME1
  CElementGen* ConstructChildParticleSystem(TToken< CGenDescription >) const;
#else
  CParticleGen* ConstructChildParticleSystem(const CToken& description, uint type,
                                             ushort seed) const;
  static CParticleGen*
  ConstructChildParticleSystem(const CToken& description, uint type, short seed,
                               EOptionalSystemFlags flags, bool modelsUseLights, bool emission,
                               const CVector3f& translation, const CTransform4f& orientation,
                               const CVector3f& globalTranslation,
                               const CTransform4f& globalOrientation, const CVector3f& globalScale,
                               const CColor& modulationColor, const CVector3f& localScale);
#endif
  void UpdateChildParticleSystems(double dt);
  void RenderModels();
#if !RSTL_PRIME1
  // Guessed names; Echoes separates model setup, drawing and cleanup.
  void BeginModelRender(SModelRenderState& state);
  void BeginIndirectModelRender(SModelRenderState& state);
  void RenderModelParticle(SModelRenderState& state, const CColor& color,
                           const CParticle& particle);
  void RenderIndirectModelParticle(SModelRenderState& state, const CColor& color,
                                   const CParticle& particle);
  void EndModelRender(const SModelRenderState& state) const;
  void EndIndirectModelRender(const SModelRenderState& state);
#endif
  void RenderLines();
  void RenderParticlesIndirectTexture();
  static void RenderParticlesFlameThrower(CElementGen* const* gens, int count
#if !RSTL_PRIME1
                                          ,
                                          const CVector3f* globalTranslation,
                                          const CTransform4f* globalOrientation,
                                          const CVector3f* globalScale,
                                          const CTransform4f* localScale
#endif
  );
  void RenderParticles();
#if RSTL_PRIME1
  void RenderBasicParticlesRotNoTS(const CTransform4f& xf);
  void RenderBasicParticlesNoRotNoTS(const CTransform4f& xf);
  void RenderBasicParticlesRotTS(const CTransform4f& xf);
  void RenderBasicParticlesNoRotTS(const CTransform4f& xf);
#else
  // Guessed names, extending Prime's basic-render variants with modulation color.
  void RenderBasicParticlesNoRotTS(const CTransform4f& xf) const;
  void RenderBasicParticlesRotTS(const CTransform4f& xf) const;
  void RenderBasicParticlesNoRotNoTS(const CTransform4f& xf) const;
  void RenderBasicParticlesRotNoTS(const CTransform4f& xf) const;
  void RenderBasicParticlesNoRotTSModulated(const CTransform4f& xf) const;
  void RenderBasicParticlesRotTSModulated(const CTransform4f& xf) const;
  void RenderBasicParticlesNoRotNoTSModulated(const CTransform4f& xf) const;
  void RenderBasicParticlesRotNoTSModulated(const CTransform4f& xf) const;
#endif

  int GetParticleCountAll() const;
  int GetParticleCountAllInternal() const;
  void AccumulateBounds(const CVector3f& position, float size);
  void BuildParticleSystemBounds();

#if RSTL_PRIME1
  int GetNumActiveChildParticles() const;
  CParticleGen* GetActiveChildParticle(int index) const;
#endif

  int GetCumulativeParticleCount() const { return mCumulativeParticles; }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  bool IsIndirectTextured()
      const; // { return x28_loadedGenDesc->x54_x40_TEXR && x28_loadedGenDesc->x58_x44_TIND; }
  void SetExternalVar(int index, float val);
#else
  int GetMaxParticles() const { return mMAXP; }
  double GetCurrentTime() const { return mCurSeconds; } // Guessed name
  float GetTimeDeltaScale() const { return mTimeDeltaScale; } // Guessed name
  bool IsIndirectTextured() const;
#endif

  float GetExternalVar(int index) const;
#if !RSTL_PRIME1
  void SetExternalParam(uint index, float value);
  float GetGenerationRate();
  int GetNumSpawnedParticleSystems() const;
  CParticleGen* SpawnedParticleSystem(int index);
  CAdvancedValues* ParticleAdditionalData(uint index);
  const CAdvancedValues* GetParticleAdditionalData(uint index) const;
#endif

  static void Initialize();
  static void ShutDown();
#if RSTL_PRIME1
  static int GetParticleAliveCount() { return mParticleAliveCount; }
#endif

  void SetGlobalOrientAndTrans(const CTransform4f& xf);
  void SetLeaveLightsEnabledForModelRender(bool b) { mModelsUseLights = b; }

  static void SetSubtractBlend(bool subtract) { sSubtractBlend = subtract; }
#if RSTL_PRIME1
  static void SetMoveRedToAlphaBuffer(const bool move) { sMoveRedToAlphaBuffer = move; }
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static void SetGlobalSeed(const ushort seed) { sSeed = seed; }

private:
#else
public:
#endif

  TLockedToken< CGenDescription > mGenDesc;
  CGenDescription* mLoadedGenDesc;
  EModelOrientationType mOrientType;
  rstl::vector< CParticle > mParticles;
#if RSTL_PRIME1
  rstl::vector< CVector3f > x40;
#endif

  rstl::vector< CMatrix3f > mParentMatrices;
  rstl::vector< CAdvancedValues > mAdvValues;
  int mInternalStartFrame;
  int mCurFrame;
  double mCurSeconds;
  float mTimeDeltaScale;
  int mPrevFrame;
  bool mParticleEmission;
  float mGeneratorRemainder;
  int mMAXP;
  short mRandomSeed;
  float mGeneratorRate;
  float mExternalVars[16];
  CVector3f mTranslation;
  CVector3f mGlobalTranslation;
  CVector3f mPOFS;
  CVector3f mGlobalScale;
  CTransform4f mGlobalScaleTransform;
  CTransform4f mGlobalScaleTransformInverse;
  CVector3f mLocalScale;
  CTransform4f mLocalScaleTransform;
  CTransform4f mLocalScaleTransformInverse;
  CTransform4f mOrientation;
  CMatrix3f mOrientationInverse;
  CTransform4f mGlobalOrientation;
  uint mActiveParticleCount;
  uint mCumulativeParticles;
  uint mRecursiveParticleCount;
  int mPSLT;
  bool mTranslationDirty : 1;
  bool mLIT_ : 1;
  bool mAAPH : 1;
  bool mZBUF : 1;
  bool mZTest : 1;
  bool mORNT : 1;
  bool mMBLR : 1;
  bool mLINE : 1;
  bool mFXLL : 1;
  bool mWarmedUp : 1;
  bool mModelsUseLights : 1;
  bool mEnableOPTS : 1;
#if !RSTL_PRIME1
  // Guessed names, based on the bounds consumers and FXBR/FXBO properties.
  bool mEnableDynamicBounds : 1;
  bool mEnableFixedBounds : 1;
#endif

  bool mEnableADV : 1;
  int mMBSP;
  uchar mBackupLightActive;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_R3IJ
  bool mHasVMD[4];
#else
  // uchar x275_pad[3];
  union {
    struct {
      bool mHasVMD[4];
    };
    uint mVmdStates;
  };
#endif
#else
  union {
    bool mHasVMD[4];
    uint mVmdStates;
  };
#endif

  CRandom16 mRandState;
  CModVectorElement* mVELSources[4];
  rstl::vector< CParticleGen* > mActivePartChildren;
  int mCSSD;
  int mSISY;
  int mPISY;
  int mSSSD;
  CVector3f mSSPO;
  int mSESD;
  CVector3f mSEPO;
  float mUpdateTime; // Guessed name; elapsed update time measured by CStopwatch.
  float mRenderTime; // Guessed name; elapsed render time measured by CStopwatch.
  CVector3f mAabbMin;
  CVector3f mAabbMax;
  float mMaxSize;
  CAABox mSystemBounds;
  LightType mLightType;
  CColor mLCLR;
  float mLINT;
  CVector3f mLOFF;
  CVector3f mLDIR;
  EFalloffType mFalloffType;
  float mLFOR;
  float mLSLA;
  CColor mModuColor;

#if RSTL_PRIME1
  static double kTickTime;
#endif
  static bool sSubtractBlend;

  static ushort sSeed;
  static int mParticleAliveCount;
  static int mParticleSystemAliveCount;
#if !RSTL_PRIME1

public:
  static void SetGlobalSeed(const ushort seed) { sSeed = seed; }

  // Hypothesis: the sbss flag read by the G2ME01 CElementGen render paths.
#endif

  static bool sMoveRedToAlphaBuffer;
#if !RSTL_PRIME1
  static bool sEnableAlphaModulation; // Guessed name; distinct from sMoveRedToAlphaBuffer.
#endif

};
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_R3IJ
CHECK_SIZEOF(CElementGen, 0x328)
#else
CHECK_SIZEOF(CElementGen, 0x340)
#endif

#ifdef __MWERKS__
#pragma cpp_extensions reset
#endif
#else
CHECK_SIZEOF(CElementGen, 0x338)
#endif

#endif // _CELEMENTGEN
