#ifndef _CPARTICLEELECTRIC
#define _CPARTICLEELECTRIC
#include "rstl/RstlVersions.h"

#include "Kyoto/Particles/CParticleGen.hpp"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/TToken.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/list.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"

class CElectricDescription;
class CElementGen;
class CParticleSwoosh;
struct SUVElementSet;

class CParticleElectric : public CParticleGen {
public:
  explicit CParticleElectric(TToken< CElectricDescription > desc);

  ~CParticleElectric() override;
  const bool Update(double) override;
  void Render() override;
  void SetOrientation(const CTransform4f& orientation) override;
  void SetTranslation(const CVector3f& translation) override;
  void SetGlobalOrientation(const CTransform4f& orientation) override;
  void SetGlobalTranslation(const CVector3f& translation) override;
  void SetGlobalScale(const CVector3f& scale) override;
  void SetLocalScale(const CVector3f& scale) override;
// TODO: verify that this Prime/Echoes difference is real
  void SetParticleEmission(RSTL_PRIME1_CONST bool emission) override;

  void SetModulationColor(const CColor& col) override;
#if !RSTL_PRIME1
  void SetGeneratorRate(float rate) override;
#endif

  const CTransform4f& GetOrientation() const override;
  const CVector3f& GetTranslation() const override;
  const CTransform4f& GetGlobalOrientation() const override;
  const CVector3f& GetGlobalTranslation() const override;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const CVector3f& GetGlobalScale() const override { return mGlobalScale; }
#else
  const CVector3f& GetGlobalScale() const override;
#endif

  bool GetParticleEmission() const override { return mEmitting; }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const CColor& GetModulationColor() const override { return mModuColor; }
#else
  const CColor& GetModulationColor() const override;
  float GetGeneratorRate() const override;
  int GetEmitterTime() const override;
  int GetSystemCount() const override;
#endif

  bool IsSystemDeletable() const override;
  rstl::optional_object< CAABox > GetBounds() const override;
  int GetParticleCount() const override;
  bool SystemHasLight() const override;
  CLight GetLight() const override;
  void DestroyParticles() override;
  uint Get4CharId() const override;
#if !RSTL_PRIME1
  int GetCumulativeParticleCount() const { return mCumulativeParticles; }
#endif

  class CLineManager {
    friend class CParticleElectric;
    rstl::vector< CVector3f, rstl::aligned_allocator > mVerts;
    float mWidth1;
    float mWidth2;
    float mWidth3;
    CColor mColor1;
    CColor mColor2;
    CColor mColor3;
    CAABox mBounds;

  public:
    CLineManager()
    : mWidth1(1.f)
    , mWidth2(2.f)
    , mWidth3(3.f)
    , mColor1(CColor::White())
    , mColor2(CColor::White())
    , mColor3(CColor::White())
    , mBounds(CAABox::MakeMaxInvertedBox()) {}
  };

  class CParticleElectricManager {
    friend class CParticleElectric;
    uint mIdx;
    uint mSLIF;
    uint mStartFrame;
    uint mEndFrame;
    int mGPSMIdx;
    int mEPSMIdx;
#if !RSTL_PRIME1
    CVector3f mInitialPos;
    CVector3f mInitialVel;
    CVector3f mFinalPos;
    CVector3f mFinalVel;
#endif

  public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    CParticleElectricManager(uint idx, uint slif, uint startFrame)
    : mIdx(idx)
    , mSLIF(slif)
    , mStartFrame(startFrame)
    , mEndFrame(startFrame + slif)
    , mGPSMIdx(-1)
    , mEPSMIdx(-1) {}
#else
    CParticleElectricManager(uint idx, uint slif, uint startFrame);
    const CVector3f& GetInitialPos() const { return mInitialPos; }
    const CVector3f& GetInitialVel() const { return mInitialVel; }
    const CVector3f& GetFinalPos() const { return mFinalPos; }
    const CVector3f& GetFinalVel() const { return mFinalVel; }
#endif

  };

  void ForceParticleCreation(int count);
  void SetOverrideIPos(const CVector3f& vec) { mOverrideIPos = vec; }
  void SetOverrideIVel(const CVector3f& vec) { mOverrideIVel = vec; }
  void SetOverrideFPos(const CVector3f& vec) { mOverrideFPos = vec; }
  void SetOverrideFVel(const CVector3f& vec) { mOverrideFVel = vec; }
  static void SetGlobalSeed(ushort seed) { sSeed = seed; }
#if !RSTL_PRIME1
  static ushort GetGlobalSeed() { return sSeed; }
#endif

private:
  void SetupLineGXMaterial();
  void DrawLineStrip(const rstl::vector< CVector3f, rstl::aligned_allocator >& verts, float width,
                     const CColor& color);
#if !RSTL_PRIME1
  void DrawTexturedLineStrip(const rstl::vector< CVector3f, rstl::aligned_allocator >& verts,
                             float width, const CColor& color, const SUVElementSet& uvs);
#endif

  void RenderLines();
  void RenderSwooshes();
  void UpdateCachedTransform();
  void UpdateLine(int idx, int frame);
  void UpdateElectricalEffects();
  void CalculateFractal(int start, int end, float ampl, float ampd);
  void CalculatePoints();
  void CreateNewParticles(int count);
  void AddElectricalEffects();
  void BuildBounds();

  TLockedToken< CElectricDescription > mElecDesc;
  int mCurrentFrame;
  int mLIFE;
  double mCurTime;
  CVector3f mTranslation;
  CTransform4f mOrientation;
  CTransform4f mInvOrientation;
  CVector3f mGlobalTranslation;
  CTransform4f mGlobalOrientation;
  CVector3f mGlobalScale;
  CVector3f mLocalScale;
  CTransform4f mCachedXf;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  float x128_;
#else
  float x130_;
#endif

  float mRenderTime;
  CVector3f mMinBounds;
  CVector3f mMaxBounds;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  float x148_;
#else
  float x150_;
#endif

  CRandom16 mRandState;
  int mSSEG;
  int mSCNT;
#if RSTL_PRIME1
  int x158_;
#endif

  float mGenRem;
#if !RSTL_PRIME1
  float mGeneratorRate;
  int mCumulativeParticles;
#endif

  CAABox mSystemBounds;
  rstl::optional_object< CVector3f > mOverrideIPos;
  rstl::optional_object< CVector3f > mOverrideIVel;
  rstl::optional_object< CVector3f > mOverrideFPos;
  rstl::optional_object< CVector3f > mOverrideFVel;
  CColor mModuColor;
  rstl::reserved_vector< bool, 32 > mAllocated;
  rstl::reserved_vector< rstl::auto_ptr< CParticleSwoosh >, 32 > mSwooshGenerators;
  rstl::reserved_vector< rstl::auto_ptr< CLineManager >, 32 > mLineManagers;
  rstl::list< CParticleElectricManager > mElectricManagers;
  rstl::vector< rstl::auto_ptr< CElementGen > > mGPSMGenerators;
  rstl::vector< rstl::auto_ptr< CElementGen > > mEPSMGenerators;

  rstl::vector< CVector3f, rstl::aligned_allocator > mCalculatedVerts;
  rstl::vector< float > mFractalMags;
  rstl::vector< CVector3f > mFractalOffsets;
  bool mEmitting : 1;
  bool mHaveGPSM : 1;
  bool mHaveEPSM : 1;
  bool mHaveSSWH : 1;
  bool mHaveLWD : 1;
  bool mTransformDirty : 1;
#if !RSTL_PRIME1
  bool mTranslationDirty : 1;
  bool mOrientationDirty : 1;
  bool mGlobalScaleDirty : 1;
  bool mLocalScaleDirty : 1;
#endif

  static ushort sSeed;
};
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CHECK_SIZEOF(CParticleElectric, RSTL_VERSION >= RSTL_R3IJ ? 0x440 : 0x458)
NESTED_CHECK_SIZEOF(CParticleElectric, CLineManager, RSTL_VERSION >= RSTL_R3IJ ? 0x3c : 0x40)
NESTED_CHECK_SIZEOF(CParticleElectric, CParticleElectricManager, 0x18)
#else
CHECK_SIZEOF(CParticleElectric, 0x460)
NESTED_CHECK_SIZEOF(CParticleElectric, CLineManager, 0x40)
NESTED_CHECK_SIZEOF(CParticleElectric, CParticleElectricManager, 0x48)
#endif

#endif // _CPARTICLEELECTRIC
