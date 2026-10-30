#include "rstl/RstlVersions.h"
#include "Kyoto/Particles/CElementGen.hpp"
#if RSTL_PRIME1
#include "Prime/MetroidPrime/CGameDebug.hpp"
#endif

#include "Kyoto/Basics/CStopwatch.hpp"
#include "Kyoto/CRandom16.hpp"

#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CLight.hpp"

#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Graphics/CTevCombiners.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"

#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"

#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "Kyoto/Particles/CParticleElectric.hpp"
#include "Kyoto/Particles/CParticleGlobals.hpp"
#if !RSTL_PRIME1
#include "Kyoto/Particles/CParticleSpawnSystem.hpp"
#endif

#include "Kyoto/Particles/CParticleSwoosh.hpp"
#if !RSTL_PRIME1
#include "Kyoto/Particles/CSortedParticleSystem.hpp"
#endif

#include "Kyoto/Particles/CSpawnSystemKeyframeData.hpp"
#include "Kyoto/Particles/CWarp.hpp"
#include "Kyoto/Particles/IElement.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/algorithm.hpp"
#include "rstl/math.hpp"
#include "rstl/single_ptr.hpp"

#include "alloca.h"
#if RSTL_PRIME1
#include "math.h"
#endif

#include "dolphin/gx.h"

#if RSTL_PRIME1
#ifdef __MWERKS__
#define frsqrte(x) static_cast< float >(__frsqrte(x))
#else

extern "C" float frsqrte(float);

#endif
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
// NOTE: This is a hack, trying to figure out what causes the wack GX codegen in
// CElementGen::RenderParticles. The `const CVector3f&` binding seems to help.
#define GXPositionVector3f(in)                                                                     \
  {                                                                                                \
    const CVector3f& vec = (in);                                                                   \
    GXPosition3f32(vec[0], vec[1], vec[2]);                                                        \
  }
#else
// Guessed names for the TU-local scale default and initialization guard.
static const CVector3f skOneVector(1.f, 1.f, 1.f);
static const double skTickTime = 1.0 / 60.0;
#endif

#if !RSTL_PRIME1
ushort CElementGen::sSeed = 99;
#endif
int CElementGen::mParticleAliveCount;
int CElementGen::mParticleSystemAliveCount;
#if RSTL_PRIME1

bool CElementGen::sMoveRedToAlphaBuffer;
#else
bool CElementGen::sEnableAlphaModulation;
#endif

bool CElementGen::sSubtractBlend;
#if RSTL_PRIME1
double CElementGen::kTickTime = 1 / 60.0;
ushort CElementGen::sSeed = 99;
#endif

static bool sStaticListInitialized;

// Guessed name, correlated with Prime's back-to-front particle ordering.
struct CParticleListItemViewPointComp {
  bool operator()(const CElementGen::CParticleListItem& a,
                  const CElementGen::CParticleListItem& b) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    return a.mViewPoint.GetY() > b.mViewPoint.GetY() ? true : false;
#else
    if (a.mViewPoint.GetY() > b.mViewPoint.GetY()) {
      return true;
    }
    return false;
#endif
  }
};

// name?
// Guessed name, correlated with Prime's back-to-front batch ordering.
struct CTexturedParticleListItemViewPointComp {
  bool operator()(const CElementGen::CTexturedParticleListItem& a,
                  const CElementGen::CTexturedParticleListItem& b) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    return a.mViewPoint.GetY() > b.mViewPoint.GetY() ? true : false;
#else
    if (a.mViewPoint.GetY() > b.mViewPoint.GetY()) {
      return true;
    }
    return false;
#endif
  }
};

CElementGen::CElementGen(TToken< CGenDescription > description, EModelOrientationType orientation,
                         EOptionalSystemFlags flags)
: mGenDesc(description)
, mLoadedGenDesc(*mGenDesc)
, mOrientType(orientation)
, mInternalStartFrame(0)
, mCurFrame(0)
, mCurSeconds(0.0)
, mPrevFrame(-1)
, mParticleEmission(true)
, mGeneratorRemainder(0.f)
, mMAXP(0)
, mRandomSeed(sSeed)
, mGeneratorRate(1.f)
, mTranslation(CVector3f::Zero())
, mGlobalTranslation(CVector3f::Zero())
, mPOFS(CVector3f::Zero())
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
, mGlobalScale(1.f, 1.f, 1.f)
#else
, mGlobalScale(skOneVector)
#endif

, mGlobalScaleTransform(CTransform4f::Identity())
, mGlobalScaleTransformInverse(CTransform4f::Identity())
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
, mLocalScale(1.f, 1.f, 1.f)
#else
, mLocalScale(skOneVector)
#endif

, mLocalScaleTransform(CTransform4f::Identity())
, mLocalScaleTransformInverse(CTransform4f::Identity())
, mOrientation(CTransform4f::Identity())
, mOrientationInverse(CMatrix3f::Identity())
, mGlobalOrientation(CTransform4f::Identity())
, mActiveParticleCount(0)
, mCumulativeParticles(0)
, mRecursiveParticleCount(0)
, mTranslationDirty(false)
, mZTest(true)
, mORNT(false)
#if !RSTL_PRIME1
, mMBLR(false)
#endif

, mWarmedUp(false)
, mModelsUseLights(false)
, mEnableOPTS(flags & kOSF_Two)
#if !RSTL_PRIME1
, mEnableDynamicBounds((flags & kOSF_DisableBounds) == 0)
, mEnableFixedBounds(false)
#endif
, mEnableADV(false)
#if !RSTL_PRIME1
, mMBSP(1)
#endif
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
, mBackupLightActive(false)
#else
, mBackupLightActive(GX_LIGHT_NULL)
#endif
, mRandState(mRandomSeed)
, mSSPO(CVector3f::Zero())
, mSEPO(CVector3f::Zero())
, mUpdateTime(0.f)
, mRenderTime(0.f)
, mAabbMin(CVector3f::Zero())
, mAabbMax(CVector3f::Zero())
, mMaxSize(0.f)
, mSystemBounds(CAABox::MakeMaxInvertedBox())
, mLightType(kLT_None)
, mLCLR(CColor::White())
, mLINT(1.f)
, mLOFF(CVector3f::Zero())
, mLDIR(1.f, 0.f, 0.f)
, mFalloffType(kFT_Linear)
, mLFOR(1.f)
, mLSLA(45.f)
, mModuColor(0xFFFFFFFF) {
#if !RSTL_PRIME1
  SetDrawFlags(mLoadedGenDesc->mDFLG);
#endif
  CGlobalRandom gr(mRandState);
#if !RSTL_PRIME1
  CParticleGlobals::SParticleSystem system('PART', this);
#endif
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (CIntElement* seed = mLoadedGenDesc->mSEED) {
    int seedVal;
    seed->GetValue(mCurFrame, seedVal);
#else
  if (mLoadedGenDesc->mSEED) {
    int seedVal;
    mLoadedGenDesc->mSEED->GetValue(mCurFrame, seedVal);
#endif
    mRandomSeed = seedVal;
  }
  mRandState.SetSeed(mRandomSeed);
  ++mParticleSystemAliveCount;
  mLIT_ = mLoadedGenDesc->mLIT_;
  mAAPH = mLoadedGenDesc->mAAPH;
  mZBUF = mLoadedGenDesc->mZBUF;
  mORNT = mLoadedGenDesc->mORNT;
  mMBLR = mORNT ? false : mLoadedGenDesc->mMBLR;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (CIntElement* mbsp = mLoadedGenDesc->mMBSP) {
    mbsp->GetValue(mCurFrame, mMBSP);
#else
  if (mLoadedGenDesc->mMBSP && mMBLR) {
    mLoadedGenDesc->mMBSP->GetValue(mCurFrame, mMBSP);
#endif
  }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  int idx = 0;
  if (CModVectorElement* vel1 = mLoadedGenDesc->mVEL1) {
    mVELSources[idx] = vel1;
    mHasVMD[idx] = mLoadedGenDesc->mVMD1;
    ++idx;
#else
  int velocitySourceCount = 0;
  if (mLoadedGenDesc->mVEL1) {
    mVELSources[velocitySourceCount] = mLoadedGenDesc->mVEL1;
    mHasVMD[velocitySourceCount++] = mLoadedGenDesc->mVMD1;
#endif
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (CModVectorElement* vel2 = mLoadedGenDesc->mVEL2) {
    mVELSources[idx] = vel2;
    mHasVMD[idx] = mLoadedGenDesc->mVMD2;
    ++idx;
#else
  if (mLoadedGenDesc->mVEL2) {
    mVELSources[velocitySourceCount] = mLoadedGenDesc->mVEL2;
    mHasVMD[velocitySourceCount++] = mLoadedGenDesc->mVMD2;
#endif
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (CModVectorElement* vel3 = mLoadedGenDesc->mVEL3) {
    mVELSources[idx] = vel3;
    mHasVMD[idx] = mLoadedGenDesc->mVMD3;
    ++idx;
#else
  if (mLoadedGenDesc->mVEL3) {
    mVELSources[velocitySourceCount] = mLoadedGenDesc->mVEL3;
    mHasVMD[velocitySourceCount++] = mLoadedGenDesc->mVMD3;
#endif
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (CModVectorElement* vel4 = mLoadedGenDesc->mVEL4) {
    mVELSources[idx] = vel4;
    mHasVMD[idx] = mLoadedGenDesc->mVMD4;
    ++idx;
#else
  if (mLoadedGenDesc->mVEL4) {
    mVELSources[velocitySourceCount] = mLoadedGenDesc->mVEL4;
    mHasVMD[velocitySourceCount++] = mLoadedGenDesc->mVMD4;
#endif
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  for (; idx < 4; ++idx) {
    mVELSources[idx] = nullptr;
#else
  for (int i = velocitySourceCount; i < 4; ++i) {
    mVELSources[i] = nullptr;
#endif
  }

  if (mLoadedGenDesc->mADV1 || mLoadedGenDesc->mADV2 || mLoadedGenDesc->mADV3 ||
      mLoadedGenDesc->mADV4 || mLoadedGenDesc->mADV5 || mLoadedGenDesc->mADV6 ||
      mLoadedGenDesc->mADV7 || mLoadedGenDesc->mADV8
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
      || mLoadedGenDesc->mADV9
      || mLoadedGenDesc->mVAV1
      || mLoadedGenDesc->mVAV2
      || mLoadedGenDesc->mVAV3
#endif
  ) {
    mEnableADV = true;
  }

  for (int i = 0; i < 16; ++i) {
    mExternalVars[i] = 0.f;
  }

  mCSSD = 0;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (CIntElement* cssd = mLoadedGenDesc->mCSSD) {
    cssd->GetValue(0, mCSSD);
#else
  if (mLoadedGenDesc->mCSSD) {
    mLoadedGenDesc->mCSSD->GetValue(0, mCSSD);
#endif
  }

  mPISY = 16;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (CIntElement* pisy = mLoadedGenDesc->mPISY) {
    pisy->GetValue(0, mPISY);
#else
  if (mLoadedGenDesc->mPISY) {
    mLoadedGenDesc->mPISY->GetValue(0, mPISY);
#endif

    if (mPISY <= 0) {
      mPISY = 1;
    }
  }

  mSISY = 16;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (CIntElement* sisy = mLoadedGenDesc->mSISY) {
    sisy->GetValue(0, mSISY);
#else
  if (mLoadedGenDesc->mSISY) {
    mLoadedGenDesc->mSISY->GetValue(0, mSISY);
#endif
  }

  mSSSD = 0;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (CIntElement* sssd = mLoadedGenDesc->mSSSD) {
    sssd->GetValue(0, mSSSD);
#else
  if (mLoadedGenDesc->mSSSD) {
    mLoadedGenDesc->mSSSD->GetValue(0, mSSSD);
#endif
  }

  if (mLoadedGenDesc->mSSPO) {
    mLoadedGenDesc->mSSPO->GetValue(0, mSSPO);
    if (!mLoadedGenDesc->mSSPO->IsFastConstant()) {
      mTranslationDirty = true;
    }
  }

  mSESD = 0;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (CIntElement* sesd = mLoadedGenDesc->mSESD) {
    sesd->GetValue(0, mSESD);
#else
  if (mLoadedGenDesc->mSESD) {
    mLoadedGenDesc->mSESD->GetValue(0, mSESD);
#endif
  }

  if (mLoadedGenDesc->mSEPO) {
    mLoadedGenDesc->mSEPO->GetValue(0, mSEPO);
    if (!mLoadedGenDesc->mSEPO->IsFastConstant()) {
      mTranslationDirty = true;
    }
  }

  if (mLoadedGenDesc->mPOFS) {
    mLoadedGenDesc->mPOFS->GetValue(mCurFrame, mPOFS);
    if (!mLoadedGenDesc->mPOFS->IsFastConstant()) {
      mTranslationDirty = true;
    }
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (CIntElement* pslt = mLoadedGenDesc->mPSLT) {
    pslt->GetValue(0, mPSLT);
#else
  if (mLoadedGenDesc->mPSLT) {
    mLoadedGenDesc->mPSLT->GetValue(0, mPSLT);
#endif
  } else {
    mPSLT = 0x7FFFFF;
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (CIntElement* maxp = mLoadedGenDesc->mMAXP) {
    maxp->GetValue(mCurFrame, mMAXP);
#else
  if (mLoadedGenDesc->mMAXP) {
    mLoadedGenDesc->mMAXP->GetValue(mCurFrame, mMAXP);
#endif
  }
  int count = mMAXP;
  if (count > 256) {
    count = 256;
  }
  mParticles.reserve(count);

  if (mEnableADV) {
    mAdvValues.assign(count);
  }

  if (mOrientType == kMOT_One) {
    mParentMatrices.assign(mMAXP, CMatrix3f::Identity());
  }

  mLINE = mLoadedGenDesc->mLINE;
  mFXLL = mLoadedGenDesc->mFXLL;

  if (mLoadedGenDesc->mLTYP) {
    int ltypVal = 0;
    mLoadedGenDesc->mLTYP->GetValue(mCurFrame, ltypVal);
    switch (LightType(ltypVal)) {
    case kLT_None:
      mLightType = kLT_None;
      break;
    case kLT_Custom:
      mLightType = kLT_Custom;
      break;
    case kLT_Directional:
      mLightType = kLT_Directional;
      break;
    case kLT_Spot:
      mLightType = kLT_Spot;
      break;
    default:
      mLightType = kLT_None;
      break;
    }
    if (mLoadedGenDesc->mLFOT) {
      int lfotVal = 1;
      mLoadedGenDesc->mLFOT->GetValue(mCurFrame, lfotVal);
      switch (static_cast< EFalloffType >(lfotVal)) {
      case kFT_Constant:
        mFalloffType = kFT_Constant;
        break;
      case kFT_Linear:
        mFalloffType = kFT_Linear;
        break;
      case kFT_Quadratic:
        mFalloffType = kFT_Quadratic;
        break;
      default:
        mFalloffType = kFT_Linear;
        break;
      }
    }
#if !RSTL_PRIME1
    if (mLightType != kLT_None) {
      UpdateLightParameters();
    }
#endif
  }
#if !RSTL_PRIME1
  if (mEnableDynamicBounds) {
    mEnableFixedBounds = mLoadedGenDesc->mFXBR != nullptr;
    if (mEnableFixedBounds) {
      mEnableDynamicBounds = false;
    }
  }
#endif
}

CElementGen::~CElementGen() {
  --mParticleSystemAliveCount;
  mParticleAliveCount -= mParticles.size();
  for (AUTO(it, mActivePartChildren.begin()); it != mActivePartChildren.end(); ++it) {
    delete *it;
  }
}

void CElementGen::Initialize() {
  if (sStaticListInitialized) {
    return;
  }
  mParticleAliveCount = 0;
  mParticleSystemAliveCount = 0;
  sStaticListInitialized = true;
}

void CElementGen::ShutDown() { sStaticListInitialized = false; }

void CElementGen::SetTranslation(const CVector3f& translation) {
  mTranslation = translation;
  for (AUTO(it, mActivePartChildren.begin()); it != mActivePartChildren.end(); ++it) {
    CParticleGen* ch = *it;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    if (ch->Get4CharId() == 'ELSC') {
      ch->SetTranslation(translation + mSEPO);
    } else if (ch->Get4CharId() == 'SWHC') {
      ch->SetTranslation(translation + mSSPO);
    } else {
      ch->SetTranslation(translation);
#else
    switch (ch->Get4CharId()) {
    case 'ELSC':
      ch->SetTranslation(translation + mSEPO);
      break;
    case 'SWHC':
      ch->SetTranslation(translation + mSSPO);
      // The original also applies the unoffset translation to swooshes.
    default:
      ch->SetTranslation(translation);
      break;
#endif
    }
  }
}

void CElementGen::SetGlobalTranslation(const CVector3f& translation) {
  mGlobalTranslation = translation;
  for (AUTO(it, mActivePartChildren.begin()); it != mActivePartChildren.end(); ++it) {
    (*it)->SetGlobalTranslation(translation);
  }
}

void CElementGen::SetModulationColor(const CColor& color) {
  mModuColor = color;
  for (AUTO(it, mActivePartChildren.begin()); it != mActivePartChildren.end(); ++it) {
    (*it)->SetModulationColor(color);
  }
}

const CColor& CElementGen::GetModulationColor() const { return mModuColor; }

void CElementGen::SetGlobalScale(const CVector3f& scale) {
  mGlobalScale = scale;
#if !RSTL_PRIME1
  if (close_enough(mGlobalScale.GetX(), 0.f, 0.0001f)) {
    mGlobalScale.SetX(0.0001f * CMath::Sign(mGlobalScale.GetX()));
  }
  if (close_enough(mGlobalScale.GetY(), 0.f, 0.0001f)) {
    mGlobalScale.SetY(0.0001f * CMath::Sign(mGlobalScale.GetY()));
  }
  if (close_enough(mGlobalScale.GetZ(), 0.f, 0.0001f)) {
    mGlobalScale.SetZ(0.0001f * CMath::Sign(mGlobalScale.GetZ()));
  }
#endif

  mGlobalScaleTransform =
      CTransform4f::Scale(mGlobalScale.GetX(), mGlobalScale.GetY(), mGlobalScale.GetZ());
  mGlobalScaleTransformInverse = CTransform4f::Scale(
      1.f / mGlobalScale.GetX(), 1.f / mGlobalScale.GetY(), 1.f / mGlobalScale.GetZ());
  for (AUTO(it, mActivePartChildren.begin()); it != mActivePartChildren.end(); ++it) {
    (*it)->SetGlobalScale(scale);
  }
}

void CElementGen::SetLocalScale(const CVector3f& scale) {
  mLocalScale = scale;
#if !RSTL_PRIME1
  if (close_enough(mLocalScale.GetX(), 0.f, 0.0001f)) {
    mLocalScale.SetX(0.0001f * CMath::Sign(mLocalScale.GetX()));
  }
  if (close_enough(mLocalScale.GetY(), 0.f, 0.0001f)) {
    mLocalScale.SetY(0.0001f * CMath::Sign(mLocalScale.GetY()));
  }
  if (close_enough(mLocalScale.GetZ(), 0.f, 0.0001f)) {
    mLocalScale.SetZ(0.0001f * CMath::Sign(mLocalScale.GetZ()));
  }
#endif

  mLocalScaleTransform =
      CTransform4f::Scale(mLocalScale.GetX(), mLocalScale.GetY(), mLocalScale.GetZ());
  mLocalScaleTransformInverse = CTransform4f::Scale(
      1.f / mLocalScale.GetX(), 1.f / mLocalScale.GetY(), 1.f / mLocalScale.GetZ());
  for (AUTO(it, mActivePartChildren.begin()); it != mActivePartChildren.end(); ++it) {
    (*it)->SetLocalScale(scale);
  }
}

void CElementGen::SetOrientation(const CTransform4f& orientation) {
  mOrientation = orientation;
  mOrientationInverse = mOrientation.GetQuickInverse().BuildMatrix3f();
  for (AUTO(it, mActivePartChildren.begin()); it != mActivePartChildren.end(); ++it) {
    (*it)->SetOrientation(orientation);
  }
}

void CElementGen::SetGlobalOrientation(const CTransform4f& orientation) {
  mGlobalOrientation.SetRotation(orientation);
  for (AUTO(it, mActivePartChildren.begin()); it != mActivePartChildren.end(); ++it) {
    (*it)->SetGlobalOrientation(mGlobalOrientation);
  }
}

void CElementGen::SetGlobalOrientAndTrans(const CTransform4f& xf) {
  SetGlobalOrientation(xf);
  SetGlobalTranslation(xf.GetTranslation());
}

const bool CElementGen::Update(double dt) {
  CParticleGlobals::SParticleSystem thisSystem('PART', this);
  if (mLoadedGenDesc->mPSWT && !mWarmedUp) {
    int pswt = 0;
    mLoadedGenDesc->mPSWT->GetValue(mCurFrame, pswt);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    InternalUpdate(kTickTime * pswt);
#else
    InternalUpdate(skTickTime * pswt);
#endif
    mWarmedUp = true;
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1 && RSTL_VERSION >= RSTL_R3IJ
  const bool ret = InternalUpdate(dt);
  CParticleGlobals::SetCurrentParticleSystem(thisSystem.mPrev);
  return ret;
#else
  return InternalUpdate(dt);
#endif
}

bool CElementGen::InternalUpdate(double dt) {
  CStopwatch sw;
  CGlobalRandom gr(mRandState);
  int frameUpdateCount = 0;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  double t = mCurFrame * kTickTime;
  double dt1 = close_enough(dt, kTickTime) ? kTickTime : dt;
#else
  double t = mCurFrame * skTickTime;
  const double tolerance = skTickTime / 1000.0;
  double dt1;
  if (close_enough(dt, skTickTime, tolerance)) {
    dt1 = skTickTime;
  } else {
    dt1 = dt;
  }
#endif

  CParticleGlobals::SetEmitterTime(mCurFrame);

  if (mLoadedGenDesc->mPSTS) {
    float psts = 1.0f;
    mLoadedGenDesc->mPSTS->GetValue(mCurFrame, psts);
    dt1 *= psts;
    dt1 = rstl::max_val(0.0, dt1);
  }
  mCurSeconds += dt1;

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (mMBLR && dt > 0.0) {
    if (mLoadedGenDesc->mMBSP) {
      mLoadedGenDesc->mMBSP->GetValue(mCurFrame, mMBSP);
    }
  }
#else
  if (mMBLR && dt > 0.0 && mLoadedGenDesc->mMBSP) {
    mLoadedGenDesc->mMBSP->GetValue(mCurFrame, mMBSP);
  }
#endif

  while (t < mCurSeconds && !close_enough(t, mCurSeconds
#if !RSTL_PRIME1
                                          ,
                                          tolerance
#endif
                                          )) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    mAabbMin = CVector3f(FLT_MAX, FLT_MAX, FLT_MAX);
    mAabbMax = CVector3f(-FLT_MAX, -FLT_MAX, -FLT_MAX);
#else
    mAabbMin = CVector3f(3.4028235e38f, 3.4028235e38f, 3.4028235e38f);
    mAabbMax = CVector3f(-3.4028235e38f, -3.4028235e38f, -3.4028235e38f);
#endif
    mMaxSize = 0.0f;

    CParticleGlobals::SetEmitterTime(mCurFrame);
    UpdateExistingParticles();
    CParticleGlobals::SetParticleLifetime(mPSLT);

    if (mCurFrame < mPSLT && mParticleEmission) {
      float grte = 0.0f;
      if (mLoadedGenDesc->mGRTE->GetValue(mCurFrame, grte)) {
        mParticles.clear();
        return true;
      }
      grte = rstl::max_val(0.0f, grte * mGeneratorRate);
      mGeneratorRemainder += grte;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      int genCount = static_cast< int >(floor(mGeneratorRemainder));
#else
      const int genCount = static_cast< int >(static_cast< float >(floor(mGeneratorRemainder)));
#endif
      mGeneratorRemainder -= static_cast< float >(genCount);

      if (mLoadedGenDesc->mMAXP) {
        mLoadedGenDesc->mMAXP->GetValue(mCurFrame, mMAXP);
      }
      CreateNewParticles(genCount);
    }

    if (mTranslationDirty) {
      UpdatePSTranslationAndOrientation();
    }

    if (mLightType != kLT_None) {
      UpdateLightParameters();
    }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    UpdateChildParticleSystems(kTickTime);
#else
    UpdateChildParticleSystems(skTickTime);
#endif
    ++frameUpdateCount;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    t += kTickTime;
#else
    t += skTickTime;
#endif
    ++mCurFrame;
  }

  if (close_enough(t, mCurSeconds
#if !RSTL_PRIME1
                   ,
                   tolerance
#endif
                   )) {
    mCurSeconds = t;
    mTimeDeltaScale = 1.0f;
  } else {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    UpdateChildParticleSystems(dt1 - (double)frameUpdateCount * kTickTime);
    mTimeDeltaScale = 1.0f - static_cast< float >((t - mCurSeconds) / kTickTime);
#else
    UpdateChildParticleSystems(dt1 - frameUpdateCount * skTickTime);
    mTimeDeltaScale = 1.0f - static_cast< float >((t - mCurSeconds) / skTickTime);
#endif
  }
#if !RSTL_PRIME1
  if (mEnableFixedBounds) {
    float radius = 0.f;
    if (mLoadedGenDesc->mFXBR) {
      mLoadedGenDesc->mFXBR->GetValue(mCurFrame, radius);
    }
    CVector3f offset = CVector3f::Zero();
    if (mLoadedGenDesc->mFXBO) {
      mLoadedGenDesc->mFXBO->GetValue(mCurFrame, offset);
    }
    offset = mOrientation.Rotate(offset) +
             (mGlobalScaleTransformInverse * mLocalScaleTransformInverse) * mTranslation + mPOFS;
    AccumulateBounds(offset, 2.f * radius);
  }
#endif

  BuildParticleSystemBounds();
  mUpdateTime = sw.GetElapsedTime();
  return false;
}

void CElementGen::UpdateLightParameters() {
  mLoadedGenDesc->mLCLR->GetValue(mCurFrame, mLCLR);
  mLoadedGenDesc->mLINT->GetValue(mCurFrame, mLINT);
  if (mLightType != kLT_Directional) {
    mLoadedGenDesc->mLOFF->GetValue(mCurFrame, mLOFF);
    mLoadedGenDesc->mLFOR->GetValue(mCurFrame, mLFOR);
    if (mLightType == kLT_Spot) {
      mLoadedGenDesc->mLSLA->GetValue(mCurFrame, mLSLA);
    }
  }
  if (mLightType != kLT_Custom) {
    mLoadedGenDesc->mLDIR->GetValue(mCurFrame, mLDIR);
  }
}

void CElementGen::UpdateAdvanceAccessParameters(int particleIndex, int particleFrame) {
  float* params = mAdvValues[particleIndex].mValues;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CParticleGlobals::mParticleAccessParameters = params;
#else
  CParticleGlobals::SetParticleAccessParameters(params);
  if (mLoadedGenDesc->mVAV1) {
    mLoadedGenDesc->mVAV1->GetValue(particleFrame, *reinterpret_cast< CVector3f* >(params));
  }
#endif
  if (mLoadedGenDesc->mADV1) {
    mLoadedGenDesc->mADV1->GetValue(particleFrame, params[0]);
  }
  if (mLoadedGenDesc->mADV2) {
    mLoadedGenDesc->mADV2->GetValue(particleFrame, params[1]);
  }
  if (mLoadedGenDesc->mADV3) {
    mLoadedGenDesc->mADV3->GetValue(particleFrame, params[2]);
  }
#if !RSTL_PRIME1
  if (mLoadedGenDesc->mVAV2) {
    mLoadedGenDesc->mVAV2->GetValue(particleFrame, *reinterpret_cast< CVector3f* >(params + 3));
  }
#endif
  if (mLoadedGenDesc->mADV4) {
    mLoadedGenDesc->mADV4->GetValue(particleFrame, params[3]);
  }
  if (mLoadedGenDesc->mADV5) {
    mLoadedGenDesc->mADV5->GetValue(particleFrame, params[4]);
  }
  if (mLoadedGenDesc->mADV6) {
    mLoadedGenDesc->mADV6->GetValue(particleFrame, params[5]);
  }
#if !RSTL_PRIME1
  if (mLoadedGenDesc->mVAV3) {
    mLoadedGenDesc->mVAV3->GetValue(particleFrame, *reinterpret_cast< CVector3f* >(params + 6));
  }
#endif
  if (mLoadedGenDesc->mADV7) {
    mLoadedGenDesc->mADV7->GetValue(particleFrame, params[6]);
  }
  if (mLoadedGenDesc->mADV8) {
    mLoadedGenDesc->mADV8->GetValue(particleFrame, params[7]);
  }
#if !RSTL_PRIME1
  if (mLoadedGenDesc->mADV9) {
    mLoadedGenDesc->mADV9->GetValue(particleFrame, params[8]);
  }
#endif
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
bool CElementGen::UpdateVelocitySource(int velIdx, int frame, CParticle& particle) {
#else
bool CElementGen::UpdateVelocitySource(int velIdx, int frame, CParticle& particle,
                                       const CVector3f& scaledTranslation) {
#endif
  bool reset;
  if (mHasVMD[velIdx]) {
    CVector3f localVel = mOrientationInverse * particle.mVel;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    CVector3f localPos = mOrientationInverse * (particle.mPos - mTranslation);
#else
    CVector3f localPos = mOrientationInverse * (particle.mPos - scaledTranslation);
#endif
    reset = mVELSources[velIdx]->GetValue(frame, localVel, localPos);
    particle.mVel = mOrientation.Rotate(localVel);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    particle.mPos = mOrientation.Rotate(localPos) + mTranslation;
#else
    particle.mPos = mOrientation.Rotate(localPos) + scaledTranslation;
#endif
  } else {
    reset = mVELSources[velIdx]->GetValue(frame, particle.mVel, particle.mPos);
  }
  if (reset) {
    particle.mEndFrame = -1;
    return true;
  }
  return false;
}

void CElementGen::UpdateExistingParticles() {
  AUTO(it, mParticles.begin());
  mActiveParticleCount = 0;
  CParticleGlobals::SetEmitterTime(mCurFrame);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CParticleGlobals::mParticleAccessParameters = NULL;
#else
  CParticleGlobals::SetParticleAccessParameters(nullptr);
  const CVector3f scaledTranslation =
      (mGlobalScaleTransformInverse * mLocalScaleTransformInverse) * mTranslation;
#endif
  while (it != mParticles.end()) {
    if (it->mEndFrame < mCurFrame) {
      --mParticleAliveCount;
      if (it + 1 == mParticles.end()) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
        --mParticles.mCount;
#else
        mParticles.pop_back();
#endif
        break;
      }
      *it = mParticles.back();
      if (mOrientType == kMOT_One) {
        mParentMatrices[mActiveParticleCount] = mParentMatrices[mParticles.size() - 1];
      }
      if (mEnableADV) {
        mAdvValues[mActiveParticleCount] = mAdvValues[mParticles.size() - 1];
      }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      --mParticles.mCount;
      if (it != mParticles.end()) {
        if (it->mEndFrame < mCurFrame) {
          continue;
        }
      }
#else
      mParticles.pop_back();
      if (it != mParticles.end() && it->mEndFrame < mCurFrame) {
        continue;
      }
#endif
    }
    it->mPrevPos = it->mPos;
    it->mPos += it->mVel;
    int particleFrame = mCurFrame - it->mStartFrame;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    CParticleGlobals::mCurrentParticle = &*it;
#else
    CParticleGlobals::SetCurrentParticle(&*it);
#endif
    CParticleGlobals::SetParticleLifetime(it->mEndFrame - it->mStartFrame);
    CParticleGlobals::UpdateParticleLifetimeTweenValues(particleFrame);
    if (mEnableADV) {
      UpdateAdvanceAccessParameters(mActiveParticleCount, particleFrame);
    }
    ++mActiveParticleCount;
    if (mVELSources[0]) {
      UpdateVelocitySource(0, particleFrame, *it
#if !RSTL_PRIME1
                           ,
                           scaledTranslation
#endif
      );
      if (mVELSources[1]) {
        UpdateVelocitySource(1, particleFrame, *it
#if !RSTL_PRIME1
                             ,
                             scaledTranslation
#endif
        );
        if (mVELSources[2]) {
          UpdateVelocitySource(2, particleFrame, *it
#if !RSTL_PRIME1
                               ,
                               scaledTranslation
#endif
          );
          if (mVELSources[3]) {
            UpdateVelocitySource(3, particleFrame, *it
#if !RSTL_PRIME1
                                 ,
                                 scaledTranslation
#endif
            );
          }
        }
      }
    }
    if (mLINE) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      if (CRealElement* leng = mLoadedGenDesc->mLENG) {
        leng->GetValue(particleFrame, it->mLineLengthOrSize);
#else
      if (mLoadedGenDesc->mLENG) {
        mLoadedGenDesc->mLENG->GetValue(particleFrame, it->mLineLengthOrSize);
#endif
      }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      if (CRealElement* widt = mLoadedGenDesc->mWIDT) {
        widt->GetValue(particleFrame, it->mLineWidthOrRota);
#else
      if (mLoadedGenDesc->mWIDT) {
        mLoadedGenDesc->mWIDT->GetValue(particleFrame, it->mLineWidthOrRota);
#endif
      }
    } else {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      if (CRealElement* rota = mLoadedGenDesc->mROTA) {
        rota->GetValue(particleFrame, it->mLineWidthOrRota);
#else
      if (mLoadedGenDesc->mROTA) {
        mLoadedGenDesc->mROTA->GetValue(particleFrame, it->mLineWidthOrRota);
#endif
      }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      if (CRealElement* size = mLoadedGenDesc->mSIZE) {
        size->GetValue(particleFrame, it->mLineLengthOrSize);
#else
      if (mLoadedGenDesc->mSIZE) {
        mLoadedGenDesc->mSIZE->GetValue(particleFrame, it->mLineLengthOrSize);
#endif
      }
    }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    if (CColorElement* colr = mLoadedGenDesc->mCOLR) {
      colr->GetValue(particleFrame, it->mColor);
#else
    if (mLoadedGenDesc->mCOLR) {
      mLoadedGenDesc->mCOLR->GetValue(particleFrame, it->mColor);
#endif
    }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    AccumulateBounds(it->mPos, it->mLineLengthOrSize);
#else
    if (mEnableDynamicBounds) {
      AccumulateBounds(it->mPos, it->mLineLengthOrSize);
    }
#endif
    ++it;
  }

  if (mParticles.size() > 0) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    rstl::list< CWarp* >::iterator it = mModifiersList.begin();
    rstl::list< CWarp* >::iterator endIt = mModifiersList.end();
    while (it != endIt) {
#else
    for (rstl::list< CWarp* >::iterator it = mModifiersList.begin(), end = mModifiersList.end();
         it != end; ++it) {
#endif
      if ((*it)->UpdateWarp()) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
        CParticle* base = mParticles.data();
        (*it)->ModifyParticles(mParticles.size(), sizeof(CParticle), &base->mEndFrame,
                               &base->mPrevPos, &base->mPos, &base->mVel, &base->mColor,
                               &base->mLineLengthOrSize, &base->mLineWidthOrRota);
#else
        CParticle& first = mParticles.front();
        (*it)->ModifyParticles(mParticles.size(), sizeof(CParticle), &first.mEndFrame,
                               &first.mPrevPos, &first.mPos, &first.mVel, &first.mColor,
                               &first.mLineLengthOrSize, &first.mLineWidthOrRota);
#endif
      }
#if RSTL_PRIME1
      ++it;
#endif
    }
  }
}

void CElementGen::CreateNewParticles(int count) {
  if (!sStaticListInitialized) {
    Initialize();
  }
  int genCount = count;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (mParticles.size() >= mMAXP) {
    return;
#else
  if (mParticles.size() < mMAXP) {
    if (genCount + mParticles.size() > mMAXP) {
      genCount = mMAXP - mParticles.size();
#endif
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (genCount + mParticles.size() > mMAXP) {
    genCount = mMAXP - mParticles.size();
  }
#endif
  if (genCount + mParticleAliveCount > 0xa00) {
    genCount = 0xa00 - mParticleAliveCount;
  }

  CGlobalRandom gr(mRandState);
  mParticles.reserve(genCount + mParticles.size());

  if (mEnableADV && mAdvValues.capacity() < genCount + mParticles.size()) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    int newCap = mAdvValues.capacity() * 2;
    mAdvValues.reserve(rstl::min_val(mMAXP, newCap));
#else
    mAdvValues.reserve(rstl::min_val(mMAXP, (genCount + mAdvValues.capacity()) * 2));
#endif
    while (mAdvValues.size() < mAdvValues.capacity()) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      mAdvValues.push_back(CAdvancedValues());
#else
      mAdvValues.push_back_unsafe(CAdvancedValues());
#endif
    }
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CParticleGlobals::mParticleAccessParameters = NULL;
#else
  CParticleGlobals::SetParticleAccessParameters(nullptr);
  const CVector3f scaledTranslation =
      (mGlobalScaleTransformInverse * mLocalScaleTransformInverse) * mTranslation;
#endif

  for (int i = 0; i < genCount; ++i) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    // TODO: The x1c_vel values somehow get frsp'd, despite being
    // floats from CVector3f::Zero and being stored as floats in CParticle.
    mParticles.push_back(CParticle());

    int particleIndex = mParticles.size() - 1;
    ++mParticleAliveCount;
    ++mActiveParticleCount;
    ++mCumulativeParticles;
#else
    mParticles.push_back_unsafe(CParticle());
    const int particleIndex = mParticles.size() - 1;
#endif

    if (mOrientType == kMOT_One) {
      mParentMatrices[particleIndex] = mOrientation.BuildMatrix3f();
    }

    CParticle& particle = mParticles[particleIndex];
    particle.mStartFrame = mCurFrame;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    if (CIntElement* ltme = mLoadedGenDesc->mLTME) {
      ltme->GetValue(0, particle.mEndFrame);
    }

    CParticleGlobals::SetParticleLifetime(particle.mEndFrame);
#else
      CParticleGlobals::SetParticleLifetime(1);
#endif

    CParticleGlobals::UpdateParticleLifetimeTweenValues(0);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    CParticleGlobals::mCurrentParticle = &particle;
#else
      CParticleGlobals::SetCurrentParticle(&particle);
#endif

    if (mEnableADV) {
      UpdateAdvanceAccessParameters(particleIndex, 0);
    }
#if !RSTL_PRIME1
      if (mLoadedGenDesc->mLTME) {
        mLoadedGenDesc->mLTME->GetValue(0, particle.mEndFrame);
      }
#endif

    particle.mEndFrame += mCurFrame;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    if (CColorElement* colr = mLoadedGenDesc->mCOLR) {
      colr->GetValue(0, particle.mColor);
    } else {
      particle.mColor = CColor(0xFFFFFFFF);
    }

    if (CEmitterElement* emtr = mLoadedGenDesc->mEMTR) {
      emtr->GetValue(mCurFrame, particle.mPos, particle.mVel);
#else
      if (mLoadedGenDesc->mEMTR) {
        mLoadedGenDesc->mEMTR->GetValue(mCurFrame, particle.mPos, particle.mVel);
#endif

      particle.mPos =
          mOrientation.Rotate(particle.mPos) +
          (mGlobalScaleTransformInverse * mLocalScaleTransformInverse) * mTranslation + mPOFS;
      particle.mVel = mOrientation.Rotate(particle.mVel);
    } else {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      const CVector3f& compPos =
          (mGlobalScaleTransformInverse * mLocalScaleTransformInverse) * mTranslation;
      particle.mPos = compPos + mPOFS;
#else
        particle.mPos =
            (mGlobalScaleTransformInverse * mLocalScaleTransformInverse) * mTranslation + mPOFS;
#endif

      particle.mVel = CVector3f::Zero();
    }

    particle.mPrevPos = particle.mPos;
#if !RSTL_PRIME1
      if (mLoadedGenDesc->mCOLR) {
        mLoadedGenDesc->mCOLR->GetValue(0, particle.mColor);
      } else {
        particle.mColor = CColor(0xFFFFFFFF);
      }
#endif

    if (mLINE) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      if (CRealElement* leng = mLoadedGenDesc->mLENG) {
        leng->GetValue(0, particle.mLineLengthOrSize);
#else
        if (mLoadedGenDesc->mLENG) {
          mLoadedGenDesc->mLENG->GetValue(0, particle.mLineLengthOrSize);
#endif
      } else {
        particle.mLineLengthOrSize = 1.f;
      }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      if (CRealElement* widt = mLoadedGenDesc->mWIDT) {
        widt->GetValue(0, particle.mLineWidthOrRota);
#else
        if (mLoadedGenDesc->mWIDT) {
          mLoadedGenDesc->mWIDT->GetValue(0, particle.mLineWidthOrRota);
#endif
      } else {
        particle.mLineWidthOrRota = 1.f;
      }
    } else {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      if (CRealElement* rota = mLoadedGenDesc->mROTA) {
        rota->GetValue(0, particle.mLineWidthOrRota);
#else
        if (mLoadedGenDesc->mROTA) {
          mLoadedGenDesc->mROTA->GetValue(0, particle.mLineWidthOrRota);
#endif
      } else {
        particle.mLineWidthOrRota = 0.f;
      }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      if (CRealElement* size = mLoadedGenDesc->mSIZE) {
        size->GetValue(0, particle.mLineLengthOrSize);
#else
        if (mLoadedGenDesc->mSIZE) {
          mLoadedGenDesc->mSIZE->GetValue(0, particle.mLineLengthOrSize);
#endif
      } else {
        particle.mLineLengthOrSize = 0.1f;
      }
    }
#if !RSTL_PRIME1
      if (mLoadedGenDesc->mVMPC) {
        if (mVELSources[0]) {
          UpdateVelocitySource(0, 0, particle, scaledTranslation);
          if (mVELSources[1]) {
            UpdateVelocitySource(1, 0, particle, scaledTranslation);
            if (mVELSources[2]) {
              UpdateVelocitySource(2, 0, particle, scaledTranslation);
              if (mVELSources[3]) {
                UpdateVelocitySource(3, 0, particle, scaledTranslation);
              }
            }
          }
        }
      }
      ++mCumulativeParticles;
      if (particle.mEndFrame == -1) {
        mParticles.pop_back();
      } else {
        if (mEnableDynamicBounds) {
#endif

    AccumulateBounds(particle.mPos, particle.mLineLengthOrSize);
#if !RSTL_PRIME1
        }
        ++mParticleAliveCount;
        ++mActiveParticleCount;
      }
    }
#endif
  }
}

void CElementGen::UpdatePSTranslationAndOrientation() {
  CGlobalRandom gr(mRandState);
  if (mPSLT >= mCurFrame) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    if (CVectorElement* pofs = mLoadedGenDesc->mPOFS) {
      pofs->GetValue(mCurFrame, mPOFS);
#else
    if (mLoadedGenDesc->mPOFS) {
      mLoadedGenDesc->mPOFS->GetValue(mCurFrame, mPOFS);
#endif
    }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    if (CVectorElement* sspo = mLoadedGenDesc->mSSPO) {
      sspo->GetValue(mCurFrame, mSSPO);
#else
    if (mLoadedGenDesc->mSSPO) {
      mLoadedGenDesc->mSSPO->GetValue(mCurFrame, mSSPO);
#endif
    }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    if (CVectorElement* sepo = mLoadedGenDesc->mSEPO) {
      sepo->GetValue(mCurFrame, mSEPO);
#else
    if (mLoadedGenDesc->mSEPO) {
      mLoadedGenDesc->mSEPO->GetValue(mCurFrame, mSEPO);
#endif
    }
  }
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CElementGen* CElementGen::ConstructChildParticleSystem(TToken< CGenDescription > desc) const {
  CElementGen* child = rs_new CElementGen(desc, kMOT_Normal, mEnableOPTS ? kOSF_Two : kOSF_One);
  child->mModelsUseLights = mModelsUseLights;
  child->SetGlobalTranslation(mGlobalTranslation);
  child->SetGlobalOrientation(mGlobalOrientation);
  child->SetGlobalScale(mGlobalScale);
  child->SetLocalScale(mLocalScale);
  child->SetTranslation(mTranslation);
  child->SetOrientation(mOrientation);
  child->SetParticleEmission(mParticleEmission);
  child->SetModulationColor(mModuColor);
#else
CParticleGen* CElementGen::ConstructChildParticleSystem(
    const CToken& description, uint type, short seed, EOptionalSystemFlags flags,
    bool modelsUseLights, bool emission, const CVector3f& translation,
    const CTransform4f& orientation, const CVector3f& globalTranslation,
    const CTransform4f& globalOrientation, const CVector3f& globalScale,
    const CColor& modulationColor, const CVector3f& localScale) {
  CParticleGen* child;
  const bool optsEnabled = (flags & kOSF_Two) != 0;
  switch (type) {
  case 'PART': {
    const short backupSeed = sSeed;
    if (seed != 0) {
      sSeed = seed;
    }
    TLockedToken< CGenDescription > particleDescription(description);
    const bool descOpts = particleDescription->mOPTS;
    if (optsEnabled && descOpts) {
      return nullptr;
    }
    CElementGen* particles = rs_new CElementGen(particleDescription, kMOT_Normal, flags);
    particles->SetLeaveLightsEnabledForModelRender(modelsUseLights);
    child = particles;
    sSeed = backupSeed;
    break;
  }
  case 'SWHC': {
    TToken< CSwooshDescription > swooshDescription(description);
    child = rs_new CParticleSwoosh(swooshDescription, 0);
    break;
  }
  case 'ELSC': {
    const short backupSeed = CParticleElectric::GetGlobalSeed();
    if (seed != 0) {
      CParticleElectric::SetGlobalSeed(seed);
    }
    TToken< CElectricDescription > electricDescription(description);
    child = rs_new CParticleElectric(electricDescription);
    CParticleElectric::SetGlobalSeed(backupSeed);
    break;
  }
  case 'SPSC': {
    const short backupSeed = CParticleSpawnSystem::GetGlobalSeed();
    if (seed != 0) {
      CParticleSpawnSystem::SetGlobalSeed(seed);
    }
    TToken< CSpawnSystemDescription > spawnDescription(description);
    child = rs_new CParticleSpawnSystem(spawnDescription, flags, modelsUseLights);
    CParticleSpawnSystem::SetGlobalSeed(backupSeed);
    break;
  }
  case 'SRSC': {
    const short backupSeed = CSortedParticleSystem::GetGlobalSeed();
    if (seed != 0) {
      CSortedParticleSystem::SetGlobalSeed(seed);
    }
    TToken< CSortedParticleSystemDescription > spawnDescription(description);
    child = rs_new CSortedParticleSystem(spawnDescription, flags, modelsUseLights);
    CSortedParticleSystem::SetGlobalSeed(backupSeed);
    break;
  }
  default:
    return nullptr;
  }

  if (child) {
    if (child->Get4CharId() == 'SPSC') {
      CParticleSpawnSystem* spawnSystem = static_cast< CParticleSpawnSystem* >(child);
      spawnSystem->ForceSetTranslation(translation);
      spawnSystem->ForceSetOrientation(orientation);
      spawnSystem->ForceSetGlobalTranslation(globalTranslation);
      spawnSystem->ForceSetGlobalOrientation(globalOrientation);
    } else {
      child->SetGlobalTranslation(globalTranslation);
      child->SetGlobalOrientation(globalOrientation);
      child->SetTranslation(translation);
      child->SetOrientation(orientation);
    }
    child->SetGlobalScale(globalScale);
    child->SetLocalScale(localScale);
    child->SetParticleEmission(emission);
    child->SetModulationColor(modulationColor);
  }
#endif

  return child;
}

#if !RSTL_PRIME1
CParticleGen* CElementGen::ConstructChildParticleSystem(const CToken& description, uint type,
                                                        ushort seed) const {
  const int boundsFlags = mEnableDynamicBounds ? kOSF_One : kOSF_DisableBounds;
  const int optsFlags = mEnableOPTS ? kOSF_Two : kOSF_One;
  const EOptionalSystemFlags flags = EOptionalSystemFlags(optsFlags | boundsFlags);
  return ConstructChildParticleSystem(
      description, type, seed, flags, mModelsUseLights, mParticleEmission, mTranslation,
      mOrientation, mGlobalTranslation, mGlobalOrientation, mGlobalScale, mModuColor, mLocalScale);
}
#endif

void CElementGen::UpdateChildParticleSystems(double dt) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (close_enough(dt, 0.0, 1e-7)) {
    return;
  }
  CGlobalRandom gr(mRandState);
#else
  if (!close_enough(dt, 0.0, 1e-7)) {
    CGlobalRandom gr(mRandState);
#endif

  // ICTS - child particle systems spawned at CSSD frame
  if (mLoadedGenDesc->mICTS && mPrevFrame != mCurFrame && mCurFrame == mCSSD) {
    int ncsyVal = 1;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    if (CIntElement* ncsy = mLoadedGenDesc->mNCSY) {
      ncsy->GetValue(mCurFrame, ncsyVal);
#else
    if (mLoadedGenDesc->mNCSY) {
      mLoadedGenDesc->mNCSY->GetValue(mCurFrame, ncsyVal);
#endif
    }
    mActivePartChildren.reserve(ncsyVal + mActivePartChildren.size());
    for (int i = 0; i < ncsyVal; ++i) {
      TLockedToken< CGenDescription > icsToken = mLoadedGenDesc->mICTS->GetToken();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      bool descOPTS = icsToken->mOPTS;
#else
      const bool descOPTS = icsToken->mOPTS;
#endif
      if (mEnableOPTS && descOPTS) {
        break;
      }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      mActivePartChildren.push_back(ConstructChildParticleSystem(icsToken));
#else
      mActivePartChildren.push_back_unsafe(ConstructChildParticleSystem(icsToken, 'PART', sSeed));
#endif
    }
  }

  // IITS - child particle systems spawned periodically
  if (mLoadedGenDesc->mIITS && mPrevFrame != mCurFrame && mCurFrame < mPSLT &&
      mParticleEmission == true && mCurFrame >= mSISY && (mCurFrame - mSISY) % mPISY == 0) {
    TLockedToken< CGenDescription > iitsToken = mLoadedGenDesc->mIITS->GetToken();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    bool iitsOPTS = iitsToken->mOPTS;
#else
    const bool iitsOPTS = iitsToken->mOPTS;
#endif
    if (!(mEnableOPTS && iitsOPTS)) {
      mActivePartChildren.reserve(mActivePartChildren.size() + 1);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      mActivePartChildren.push_back(ConstructChildParticleSystem(iitsToken));
#else
      mActivePartChildren.push_back_unsafe(ConstructChildParticleSystem(iitsToken, 'PART', sSeed));
#endif
    }
  }

  // KSSM - spawn system keyframe data
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (mLoadedGenDesc->mKSSM.get() != nullptr && mPrevFrame != mCurFrame && mCurFrame < mPSLT) {
    const ushort backupSeed = sSeed;
#else
    if (mLoadedGenDesc->mKSSM && mPrevFrame != mCurFrame && mCurFrame < mPSLT) {
#endif
    rstl::vector< CSpawnSystemKeyframeData::CSpawnSystemKeyframeInfo >& spawns =
        mLoadedGenDesc->mKSSM->GetSpawnedSystemsAtFrame(mCurFrame);
#if !RSTL_PRIME1
      if (spawns.size() != 0) {
        const ushort backupSeed = sSeed;
#endif

    mActivePartChildren.reserve(spawns.size() + mActivePartChildren.size());
    for (int i = 0; i < spawns.size(); ++i) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      TLockedToken< CGenDescription > kssmToken = *spawns[i].GetToken();
      bool kssmOPTS = kssmToken->mOPTS;
      if (mEnableOPTS && kssmOPTS) {
        continue;
#else
          const CSpawnSystemKeyframeData::CSpawnSystemKeyframeInfo& info = spawns[i];
          const ushort seed = backupSeed + i + mCurFrame;
          CParticleGen* child =
              ConstructChildParticleSystem(*info.GetToken(), info.GetType(), seed);
          if (child) {
            mActivePartChildren.push_back_unsafe(child);
#endif
      }
#if RSTL_PRIME1
      sSeed = backupSeed + i;
      mActivePartChildren.push_back(ConstructChildParticleSystem(kssmToken));
#endif
    }
    sSeed = backupSeed;
  }
#if !RSTL_PRIME1
    }
#endif

  // IDTS - child particle systems spawned at death
  if (mCurFrame == mPSLT && mPrevFrame != mCurFrame && mLoadedGenDesc->mIDTS) {
    int ndsyVal = 1;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    if (CIntElement* ndsy = mLoadedGenDesc->mNDSY) {
      ndsy->GetValue(0, ndsyVal);
#else
    if (mLoadedGenDesc->mNDSY) {
      mLoadedGenDesc->mNDSY->GetValue(0, ndsyVal);
#endif
    }
    mActivePartChildren.reserve(ndsyVal + mActivePartChildren.size());
    for (int i = 0; i < ndsyVal; ++i) {
      TLockedToken< CGenDescription > idtsToken = mLoadedGenDesc->mIDTS->GetToken();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      bool idtsOPTS = idtsToken->mOPTS;
#else
      const bool idtsOPTS = idtsToken->mOPTS;
#endif
      if (mEnableOPTS && idtsOPTS) {
        break;
      }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      mActivePartChildren.push_back(ConstructChildParticleSystem(idtsToken));
#else
      mActivePartChildren.push_back_unsafe(ConstructChildParticleSystem(idtsToken, 'PART', sSeed));
#endif
    }
  }

  // SSWH - swoosh child particle system
  if (mLoadedGenDesc->mSSWH && mPrevFrame != mCurFrame && mCurFrame == mSSSD) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    CParticleSwoosh* swoosh = rs_new CParticleSwoosh(*mLoadedGenDesc->mSSWH, 0);
#else
      CParticleGen* swoosh = rs_new CParticleSwoosh(*mLoadedGenDesc->mSSWH, 0);
#endif

    swoosh->SetGlobalTranslation(mGlobalTranslation);
    swoosh->SetGlobalScale(mGlobalScale);
    swoosh->SetLocalScale(mLocalScale);
    swoosh->SetTranslation(mTranslation + mSSPO);
    swoosh->SetOrientation(mOrientation);
    const bool emission = mParticleEmission;
    swoosh->SetParticleEmission(emission);
    mActivePartChildren.reserve(mActivePartChildren.size() + 1);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    mActivePartChildren.push_back(swoosh);
#else
      mActivePartChildren.push_back_unsafe(swoosh);
#endif
  }

  // SELC - electric child particle system
  if (mLoadedGenDesc->mSELC && mPrevFrame != mCurFrame && mCurFrame == mSESD) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    CParticleElectric* electric = rs_new CParticleElectric(*mLoadedGenDesc->mSELC);
#else
      CParticleGen* electric = rs_new CParticleElectric(*mLoadedGenDesc->mSELC);
#endif

    electric->SetGlobalTranslation(mGlobalTranslation);
    electric->SetGlobalScale(mGlobalScale);
    electric->SetLocalScale(mLocalScale);
    electric->SetTranslation(mTranslation + mSEPO);
    electric->SetOrientation(mOrientation);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    electric->SetParticleEmission(mParticleEmission);
#else
      const bool emission = mParticleEmission;
      electric->SetParticleEmission(emission);
#endif

    mActivePartChildren.reserve(mActivePartChildren.size() + 1);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    mActivePartChildren.push_back(electric);
#else
      mActivePartChildren.push_back_unsafe(electric);
#endif
  }

  // Update all child particle systems
  AUTO(it, mActivePartChildren.begin());
  while (it != mActivePartChildren.end()) {
    CParticleGen* child = *it;
    child->Update(dt);
    if (child->IsSystemDeletable() == true) {
      delete child;
      it = mActivePartChildren.erase(it);
    } else {
      ++it;
    }
  }

  mPrevFrame = mCurFrame;
#if !RSTL_PRIME1
  }
#endif
}

void CElementGen::SetParticleEmission(bool emission) {
  mParticleEmission = emission;
  for (AUTO(it, mActivePartChildren.begin()); it != mActivePartChildren.end(); ++it) {
    (*it)->SetParticleEmission(emission);
  }
}

void CElementGen::ForceParticleCreation(int count) {
  CParticleGlobals::SParticleSystem thisSystem('PART', this);
  CParticleGlobals::SetEmitterTime(mCurFrame);
  CreateNewParticles(count);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1 && RSTL_VERSION >= RSTL_R3IJ
  CParticleGlobals::SetCurrentParticleSystem(thisSystem.mPrev);
#endif
}

void CElementGen::EndLifetime() {
  mPSLT = 0;
  for (AUTO(it, mActivePartChildren.begin()); it != mActivePartChildren.end(); ++it) {
    CParticleGen* child = *it;
    if (child->Get4CharId() == 'PART') {
      static_cast< CElementGen* >(child)->EndLifetime();
    } else {
      child->SetParticleEmission(false);
    }
  }
}

void CElementGen::DestroyParticles() {
  mParticleAliveCount -= mParticles.size();

  mParticles.clear();
#if RSTL_PRIME1
  if (mOrientType == kMOT_One) {
    mParentMatrices.clear();
  }
#endif
  for (AUTO(it, mActivePartChildren.begin()); it != mActivePartChildren.end(); ++it) {
    (*it)->DestroyParticles();
  }
#if !RSTL_PRIME1
  mActiveParticleCount = 0;
  mRecursiveParticleCount = GetParticleCountAllInternal();
#endif
}

bool CElementGen::IsSystemDeletable() const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  for (AUTO(it, mActivePartChildren.begin()); it != mActivePartChildren.end(); ++it) {
#else
  rstl::vector< CParticleGen* >::const_iterator end = mActivePartChildren.end();
  for (rstl::vector< CParticleGen* >::const_iterator it = mActivePartChildren.begin(); it != end;
       ++it) {
#endif

    if (!(*it)->IsSystemDeletable()) {
      return false;
    }
  }
  if (mPSLT < mCurFrame && static_cast< int >(mActiveParticleCount) == 0) {
    return true;
  }
  return false;
}

void CElementGen::Render() {
#if RSTL_PRIME1
  if (!DEBUG_TOOLS_GET_BOOL(kDO_RenderParticles, true)) {
    return;
  }
#endif
  CStopwatch timer;
  mBackupLightActive = CGraphics::GetLightMask();
  CGraphics::DisableAllLights();
  for (AUTO(it, mActivePartChildren.begin()); it != mActivePartChildren.end(); ++it) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    (*it)->Render();
#else
    CParticleGen* child = *it;
    if (child->ShouldDraw()) {
      child->Render();
    }
#endif
  }

  CParticleGlobals::SParticleSystem sys('PART', this);
  if (mParticles.size() > 0
#if !RSTL_PRIME1
      && ShouldDraw()
#endif
  ) {
    if (mLoadedGenDesc->mPMDL || mLoadedGenDesc->mPMUS) {
      RenderModels();
    }
    if (mLINE) {
      RenderLines();
    } else {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      RenderParticles();
#else
      bool zeroSize = false;
      if (mLoadedGenDesc->mSIZE && mLoadedGenDesc->mSIZE->IsConstant()) {
        CGlobalRandom gr(mRandState);
        float size = 1.f;
        mLoadedGenDesc->mSIZE->GetValue(0, size);
        if (size == 0.f) {
          size = 1.f;
          mLoadedGenDesc->mSIZE->GetValue(1, size);
          if (size == 0.f) {
            zeroSize = true;
          }
        }
      }
      if (!zeroSize && mLoadedGenDesc->mRDOP) {
        if (IsIndirectTextured()) {
          RenderParticlesIndirectTexture();
        } else {
          RenderParticles();
        }
      }
#endif
    }
  }
  mRenderTime = timer.GetElapsedTime();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1 && RSTL_VERSION >= RSTL_R3IJ
  CParticleGlobals::SetCurrentParticleSystem(sys.mPrev);
#endif
}

#if !RSTL_PRIME1
void CElementGen::RenderBasicParticlesNoRotTS(const CTransform4f& xf) const {
  for (int i = 0; i < mParticles.size(); ++i) {
    const CParticle& particle = mParticles[i];
    const CVector3f viewPos = xf * particle.mPos;
    const float halfSize = 0.5f * particle.mLineLengthOrSize;
    float x = viewPos.GetX() + halfSize;
    const float y = viewPos.GetY();
    float z = viewPos.GetZ() + halfSize;
    const uint color = particle.mColor.GetColor_u32();

    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0202);

    x -= particle.mLineLengthOrSize;
    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0002);

    z -= particle.mLineLengthOrSize;
    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0000);

    x += particle.mLineLengthOrSize;
    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0200);
  }
}

void CElementGen::RenderBasicParticlesRotTS(const CTransform4f& xf) const {
  for (int i = 0; i < mParticles.size(); ++i) {
    const CParticle& particle = mParticles[i];
    const CVector3f viewPos = xf * particle.mPos;
    const float x = viewPos.GetX();
    const float y = viewPos.GetY();
    const float z = viewPos.GetZ();
    const uint color = particle.mColor.GetColor_u32();
    const float halfSize = 0.5f * particle.mLineLengthOrSize;
    const float theta = particle.mLineWidthOrRota * (M_PIF / 180.f);
    const float sinT = CMath::FastSinR(theta) * halfSize;
    const float cosT = CMath::FastCosR(theta) * halfSize;
    const float sinPlusCos = sinT + cosT;
    const float sinMinusCos = sinT - cosT;

    GXPosition3f32(x + sinPlusCos, y, z - sinMinusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0202);

    GXPosition3f32(x + sinMinusCos, y, z + sinPlusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0002);

    GXPosition3f32(x - sinPlusCos, y, z + sinMinusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0000);

    GXPosition3f32(x - sinMinusCos, y, z - sinPlusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0200);
  }
}

void CElementGen::RenderBasicParticlesNoRotNoTS(const CTransform4f& xf) const {
  for (int i = 0; i < mParticles.size(); ++i) {
    const CParticle& particle = mParticles[i];
    const CVector3f viewPos =
        xf * ((particle.mPos - particle.mPrevPos) * mTimeDeltaScale + particle.mPrevPos);
    const float halfSize = 0.5f * particle.mLineLengthOrSize;
    float x = viewPos.GetX() + halfSize;
    const float y = viewPos.GetY();
    float z = viewPos.GetZ() + halfSize;
    const uint color = particle.mColor.GetColor_u32();

    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0202);

    x -= particle.mLineLengthOrSize;
    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0002);

    z -= particle.mLineLengthOrSize;
    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0000);

    x += particle.mLineLengthOrSize;
    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0200);
  }
}

void CElementGen::RenderBasicParticlesRotNoTS(const CTransform4f& xf) const {
  for (int i = 0; i < mParticles.size(); ++i) {
    const CParticle& particle = mParticles[i];
    const CVector3f viewPos =
        xf * ((particle.mPos - particle.mPrevPos) * mTimeDeltaScale + particle.mPrevPos);
    const float x = viewPos.GetX();
    const float y = viewPos.GetY();
    const float z = viewPos.GetZ();
    const uint color = particle.mColor.GetColor_u32();
    const float halfSize = particle.mLineLengthOrSize * 0.5f;
    const float theta = particle.mLineWidthOrRota * (M_PIF / 180.f);
    const float sinT = CMath::FastSinR(theta) * halfSize;
    const float cosT = CMath::FastCosR(theta) * halfSize;
    const float sinPlusCos = sinT + cosT;
    const float sinMinusCos = sinT - cosT;

    GXPosition3f32(x + sinPlusCos, y, z - sinMinusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0202);

    GXPosition3f32(x + sinMinusCos, y, z + sinPlusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0002);

    GXPosition3f32(x - sinPlusCos, y, z + sinMinusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0000);

    GXPosition3f32(x - sinMinusCos, y, z - sinPlusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0200);
  }
}
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CElementGen::RenderBasicParticlesNoRotTS(const CTransform4f& xf) {
#else
void CElementGen::RenderBasicParticlesNoRotTSModulated(const CTransform4f& xf) const {
#endif
  for (int i = 0; i < mParticles.size(); ++i) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    CParticle& particle = mParticles[i];
    CVector3f viewPos = xf * particle.mPos;
    float halfSize = 0.5f * particle.mLineLengthOrSize;
    uint color = particle.mColor.GetColor_u32();
    float x = viewPos.GetX() + halfSize;
    float y = viewPos.GetY();
#else
    const CParticle& particle = mParticles[i];
    const CVector3f viewPos = xf * particle.mPos;
    const uint color = CColor::Modulate(mModuColor, particle.mColor).GetColor_u32();
    const float halfSize = 0.5f * particle.mLineLengthOrSize;
    float x = viewPos.GetX() + halfSize;
    const float y = viewPos.GetY();
#endif
    float z = viewPos.GetZ() + halfSize;

    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0202);

    x -= particle.mLineLengthOrSize;
    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0002);

    z -= particle.mLineLengthOrSize;
    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0000);

    x += particle.mLineLengthOrSize;
    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0200);
  }
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CElementGen::RenderBasicParticlesRotTS(const CTransform4f& xf) {
#else
void CElementGen::RenderBasicParticlesRotTSModulated(const CTransform4f& xf) const {
#endif
  for (int i = 0; i < mParticles.size(); ++i) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    CParticle& particle = mParticles[i];
    CVector3f viewPos = xf * particle.mPos;
    float halfSize = 0.5f * particle.mLineLengthOrSize;
    float theta = particle.mLineWidthOrRota * (M_PIF / 180.f);
    uint color = particle.mColor.GetColor_u32();
    float sinT = CMath::FastSinR(theta) * halfSize;
    float cosT = CMath::FastCosR(theta) * halfSize;
    float sinPlusCos = sinT + cosT;
    float sinMinusCos = sinT - cosT;

    float x = viewPos.GetX();
    float y = viewPos.GetY();
    float z = viewPos.GetZ();
#else
    const CParticle& particle = mParticles[i];
    const CVector3f viewPos = xf * particle.mPos;
    const uint color = CColor::Modulate(mModuColor, particle.mColor).GetColor_u32();
    const float halfSize = 0.5f * particle.mLineLengthOrSize;
    const float theta = particle.mLineWidthOrRota * (M_PIF / 180.f);
    const float sinT = CMath::FastSinR(theta) * halfSize;
    const float cosT = CMath::FastCosR(theta) * halfSize;
    const float sinPlusCos = sinT + cosT;
    const float sinMinusCos = sinT - cosT;
    const float x = viewPos.GetX();
    const float y = viewPos.GetY();
    const float z = viewPos.GetZ();
#endif

    GXPosition3f32(x + sinPlusCos, y, z - sinMinusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0202);

    GXPosition3f32(x + sinMinusCos, y, z + sinPlusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0002);

    GXPosition3f32(x - sinPlusCos, y, z + sinMinusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0000);

    GXPosition3f32(x - sinMinusCos, y, z - sinPlusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0200);
  }
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CElementGen::RenderBasicParticlesNoRotNoTS(const CTransform4f& xf) {
#else
void CElementGen::RenderBasicParticlesNoRotNoTSModulated(const CTransform4f& xf) const {
#endif
  for (int i = 0; i < mParticles.size(); ++i) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    CParticle& particle = mParticles[i];
    CVector3f viewPos =
#else
    const CParticle& particle = mParticles[i];
    const CVector3f viewPos =
#endif
        xf * ((particle.mPos - particle.mPrevPos) * mTimeDeltaScale + particle.mPrevPos);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    float halfSize = 0.5f * particle.mLineLengthOrSize;
    uint color = particle.mColor.GetColor_u32();
#else
    const uint color = CColor::Modulate(mModuColor, particle.mColor).GetColor_u32();
    const float halfSize = 0.5f * particle.mLineLengthOrSize;
#endif
    float x = viewPos.GetX() + halfSize;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    float y = viewPos.GetY();
#else
    const float y = viewPos.GetY();
#endif
    float z = viewPos.GetZ() + halfSize;

    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0202);

    x -= particle.mLineLengthOrSize;
    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0002);

    z -= particle.mLineLengthOrSize;
    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0000);

    x += particle.mLineLengthOrSize;
    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0200);
  }
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CElementGen::RenderBasicParticlesRotNoTS(const CTransform4f& xf) {
#else
void CElementGen::RenderBasicParticlesRotNoTSModulated(const CTransform4f& xf) const {
#endif
  for (int i = 0; i < mParticles.size(); ++i) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    CParticle& particle = mParticles[i];
    CVector3f viewPos =
#else
    const CParticle& particle = mParticles[i];
    const CVector3f viewPos =
#endif
        xf * ((particle.mPos - particle.mPrevPos) * mTimeDeltaScale + particle.mPrevPos);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    float halfSize = 0.5f * particle.mLineLengthOrSize;
    float theta = particle.mLineWidthOrRota * (M_PIF / 180.f);
    uint color = particle.mColor.GetColor_u32();
    float sinT = CMath::FastSinR(theta) * halfSize;
    float cosT = CMath::FastCosR(theta) * halfSize;
    float sinPlusCos = sinT + cosT;
    float sinMinusCos = sinT - cosT;
    float x = viewPos.GetX();
    float y = viewPos.GetY();
    float z = viewPos.GetZ();
#else
    const uint color = CColor::Modulate(mModuColor, particle.mColor).GetColor_u32();
    const float halfSize = 0.5f * particle.mLineLengthOrSize;
    const float theta = particle.mLineWidthOrRota * (M_PIF / 180.f);
    const float sinT = CMath::FastSinR(theta) * halfSize;
    const float cosT = CMath::FastCosR(theta) * halfSize;
    const float sinPlusCos = sinT + cosT;
    const float sinMinusCos = sinT - cosT;
    const float x = viewPos.GetX();
    const float y = viewPos.GetY();
    const float z = viewPos.GetZ();
#endif

    GXPosition3f32(x + sinPlusCos, y, z - sinMinusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0202);

    GXPosition3f32(x + sinMinusCos, y, z + sinPlusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0002);

    GXPosition3f32(x - sinPlusCos, y, z + sinMinusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0000);

    GXPosition3f32(x - sinMinusCos, y, z - sinPlusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0200);
  }
}

#if !RSTL_PRIME1
// Guessed name; shared by the ordinary and batched particle renderers.
static CTevCombiners::CTevPass sParticleModulatePass(
    CTevCombiners::ColorPass(CTevCombiners::kCS_Zero, CTevCombiners::kCS_RasterColor,
                             CTevCombiners::kCS_TextureColor, CTevCombiners::kCS_Zero),
    CTevCombiners::AlphaPass(CTevCombiners::kAS_Zero, CTevCombiners::kAS_RasterAlpha,
                             CTevCombiners::kAS_TextureAlpha, CTevCombiners::kAS_Zero));

// Guessed name
static inline void SetParticleTevScale(CTevCombiners::ETevScale scale) {
  if (scale == CTevCombiners::kTS_Scale1) {
    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
  } else {
    sParticleModulatePass.SetColorScale(scale);
    CGraphics::SetTevOp(kTS_Stage0, sParticleModulatePass);
  }
}
#endif

void CElementGen::RenderParticles() {
#if !RSTL_PRIME1
  bool noRota;
  bool constUVs;
#endif
  CGlobalRandom gr(mRandState);
#if RSTL_PRIME1
  if (IsIndirectTextured()) {
    RenderParticlesIndirectTexture();
    return;
  }

  if (mLoadedGenDesc->mSIZE != nullptr && mLoadedGenDesc->mSIZE->IsConstant()) {
    float sizeVal = 1.f;
    mLoadedGenDesc->mSIZE->GetValue(0, sizeVal);
    if (sizeVal == 0.f) {
      sizeVal = 1.f;
      mLoadedGenDesc->mSIZE->GetValue(1, sizeVal);
      if (sizeVal == 0.f) {
        return;
      }
    }
  }
#endif

  CGraphics::SetCullMode(kCM_None);

  bool hasModuColor = mModuColor.GetColor_u32() != 0xFFFFFFFF;

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CTransform4f systemViewCopy(CGraphics::GetViewMatrix());
#else
  CTransform4f systemViewCopy = CGraphics::GetViewMatrix();
#endif

  CTransform4f systemModelMatrix(systemViewCopy);
  systemModelMatrix.SetTranslation(CVector3f::Zero());
  CTransform4f systemCameraCopy(systemModelMatrix.GetQuickInverse() * mGlobalOrientation);
  systemModelMatrix = CTransform4f::Translate(mGlobalTranslation) * mGlobalScaleTransform *
                      systemModelMatrix * mLocalScaleTransform;

  if (mORNT) {
    CGraphics::SetModelMatrix(systemModelMatrix * systemCameraCopy);
  } else {
    CGraphics::SetModelMatrix(systemModelMatrix);
  }

  CGraphics::SetAlphaCompare(kAF_Greater, 0, kAO_And, kAF_Always, 0);

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  bool constUVs = true;
#else
  constUVs = true;
#endif

  SUVElementSet uvs;
  uvs.xMin = 0.f;
  uvs.xMax = 1.f;
  uvs.yMin = 0.f;
  uvs.yMax = 1.f;

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  bool noRota = mLoadedGenDesc->mROTA == nullptr;
#else
  noRota = mLoadedGenDesc->mROTA == nullptr;
#endif

  if (mLoadedGenDesc->mROTA != nullptr && mLoadedGenDesc->mROTA->IsConstant()) {
    float value = 1.f;
    mLoadedGenDesc->mROTA->GetValue(0, value);
    if (0.f == value) {
      value = 1.f;
      mLoadedGenDesc->mROTA->GetValue(1, value);
      if (0.f == value) {
        noRota = true;
      }
    }
  }

  if (mLoadedGenDesc->mTEXR != nullptr) {
    int partFrame = mCurFrame - mParticles[0].mStartFrame;
    TToken< CTexture > tex = mLoadedGenDesc->mTEXR->GetValueTexture(partFrame);
    tex->Load(GX_TEXMAP0, CTexture::kCM_Repeat);

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);

    if (hasModuColor) {
      static CTevCombiners::CTevPass ModulatePreviousTEVWithRasColor(
          CTevCombiners::ColorPass(CTevCombiners::kCS_Zero, CTevCombiners::kCS_PreviousColor,
                                   CTevCombiners::kCS_RasterColor, CTevCombiners::kCS_Zero),
          CTevCombiners::AlphaPass(CTevCombiners::kAS_Zero, CTevCombiners::kAS_PreviousAlpha,
                                   CTevCombiners::kAS_RasterAlpha, CTevCombiners::kAS_Zero));
      CGraphics::SetTevOp(kTS_Stage1, ModulatePreviousTEVWithRasColor);
    } else {
      CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
    }
#else
    if (mLoadedGenDesc->mXTAD) {
      int value = 1;
      CTevCombiners::ETevScale scale = CTevCombiners::kTS_Scale1;
      mLoadedGenDesc->mXTAD->GetValue(mCurFrame, value);
      switch (value) {
      case 2:
        scale = CTevCombiners::kTS_Scale2;
        break;
      case 4:
        scale = CTevCombiners::kTS_Scale4;
        break;
      }
      if (scale == CTevCombiners::kTS_Scale1) {
        CGraphics::SetTevOp(kTS_Stage0, CGraphics::EnvModulate());
      } else {
        SetParticleTevScale(scale);
      }
    } else {
      CGraphics::SetTevOp(kTS_Stage0, CGraphics::EnvModulate());
    }
#endif

    mLoadedGenDesc->mTEXR->GetValueUV(mCurFrame - mParticles[0].mStartFrame, uvs);
    constUVs = mLoadedGenDesc->mTEXR->HasConstantUV();
  } else {
    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvPassthru);
#if RSTL_PRIME1
    CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
#endif
  }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static const GXVtxDescList sVtxDescList[] = {
#else
  static const GXVtxDescList skVtxDescList[] = {
#endif
      {GX_VA_POS, GX_DIRECT},
      {GX_VA_CLR0, GX_DIRECT},
      {GX_VA_TEX0, GX_DIRECT},
      {GX_VA_NULL, GX_NONE},
  };
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CGX::SetVtxDescv(sVtxDescList);
#else
  CGX::SetVtxDescv(skVtxDescList);
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  GXTevStageID nextStage;
  if (hasModuColor) {
    CGX::SetNumChans(2);
    nextStage = GX_TEVSTAGE2;
    CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR1A1);
    CGX::SetChanAmbColor(CGX::Channel1, CColor(0.f, 0.f, 0.f, 1.f).GetGXColor());
    CGX::SetChanMatColor(CGX::Channel1, mModuColor.GetGXColor());
    CGX::SetChanCtrl(CGX::Channel1, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE,
                     GX_AF_NONE);
  } else {
    CGX::SetNumChans(1);
    nextStage = GX_TEVSTAGE1;
  }
  bool modulateAlpha = sMoveRedToAlphaBuffer;
#else
  GXTevStageID nextStage = GX_TEVSTAGE1;
  bool modulateAlpha = sEnableAlphaModulation;
  if (sMoveRedToAlphaBuffer) {
    GXSetAlphaUpdate(GX_FALSE);
  }
#endif

  if (sSubtractBlend) {
    CGraphics::SetDepthWriteMode(mZTest, kE_LEqual, false);
    CGX::SetBlendMode(GX_BM_SUBTRACT, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
    if (modulateAlpha) {
      CGX::SetTevColorIn(nextStage, GX_CC_ZERO, GX_CC_CPREV, GX_CC_APREV, GX_CC_ZERO);
      CGX::SetTevAlphaIn(nextStage, GX_CA_ZERO, GX_CA_TEXA, GX_CA_APREV, GX_CA_ZERO);
      CGX::SetStandardTevColorAlphaOp(nextStage);
      CGX::SetTevOrder(nextStage, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
      GXSetTevSwapMode(nextStage, GX_TEV_SWAP0, GX_TEV_SWAP1);
      nextStage = static_cast< GXTevStageID >(nextStage + 1);
    }
  } else if (modulateAlpha) {
    CGraphics::SetDepthWriteMode(mZTest, kE_LEqual, false);
    CGraphics::SetBlendMode(kBM_Blend, kBF_One, kBF_One, kLO_Clear);
    CGX::SetTevColorIn(nextStage, GX_CC_ZERO, GX_CC_CPREV, GX_CC_APREV, GX_CC_ZERO);
    CGX::SetTevAlphaIn(nextStage, GX_CA_ZERO, GX_CA_TEXA, GX_CA_APREV, GX_CA_ZERO);
    CGX::SetStandardTevColorAlphaOp(nextStage);
    CGX::SetTevOrder(nextStage, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevSwapMode(nextStage, GX_TEV_SWAP0, GX_TEV_SWAP1);
    nextStage = static_cast< GXTevStageID >(nextStage + 1);
  } else if (mAAPH) {
    CGraphics::SetDepthWriteMode(mZTest, kE_LEqual, false);
    CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_One, kLO_Clear);
  } else {
    CGraphics::SetDepthWriteMode(mZTest, kE_LEqual, mZBUF);
    CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
  }

  CGX::SetNumTevStages(static_cast< uchar >(nextStage));
  CGX::SetNumTexGens(1);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
#if !RSTL_PRIME1
  CGX::SetNumChans(1);
  CGX::SetChanAmbColor(CGX::Channel0, CColor::ToGX(0u));
#endif
  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);

  GXSetVtxAttrFmt(GX_VTXFMT6, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
  GXSetVtxAttrFmt(GX_VTXFMT6, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
  if (constUVs) {
    GXSetVtxAttrFmt(GX_VTXFMT6, GX_VA_TEX0, GX_TEX_ST, GX_S8, 1);
  } else {
    GXSetVtxAttrFmt(GX_VTXFMT6, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
  }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  int particleCount = mParticles.size();
  int mbspVal = rstl::max_val(1, mMBSP);
#else
  int mbspVal = 1;
  int particleCount = static_cast< int >(mParticles.size());
  if (mbspVal < mMBSP) {
    mbspVal = mMBSP;
  }
#endif

  if (!mMBLR) {
    CGX::Begin(GX_QUADS, GX_VTXFMT6, static_cast< ushort >(particleCount * 4));
  } else {
    CGX::Begin(GX_QUADS, GX_VTXFMT6, static_cast< ushort >(mbspVal * (particleCount * 4)));
  }

  bool noTS = !close_enough(mTimeDeltaScale, 1.f, 0.00001f);
#if !RSTL_PRIME1
  CParticleListItem* sortItems = nullptr;
  rstl::single_ptr< CParticleListItem > heapItems;
#endif

  bool SORT = mLoadedGenDesc->mSORT;
#if RSTL_PRIME1
  CParticleListItem* sortItems = nullptr;
#endif

  if (SORT) {
#if !RSTL_PRIME1
    if (particleCount <= 128)
#endif
    {
      sortItems =
          static_cast< CParticleListItem* >(alloca(particleCount * sizeof(CParticleListItem)));
    }
#if !RSTL_PRIME1
    else {
      sortItems = static_cast< CParticleListItem* >(
          CMemory::Alloc(particleCount * sizeof(CParticleListItem)));
      heapItems = sortItems;
    }
#endif
    for (int i = 0; i < particleCount; ++i) {
      CParticle& particle = mParticles[i];
      sortItems[i].mViewPoint =
          systemCameraCopy *
          ((particle.mPos - particle.mPrevPos) * mTimeDeltaScale + particle.mPrevPos);
      sortItems[i].mPartIdx = static_cast< ushort >(i);
    }
    rstl::sort(sortItems, sortItems + particleCount, CParticleListItemViewPointComp());
  }

  CParticleGlobals::SetEmitterTime(mCurFrame);

  if (!mMBLR) {
    if (!SORT && constUVs && !mORNT) {
#if !RSTL_PRIME1
      if (hasModuColor) {
        if (noRota) {
          if (noTS) {
            RenderBasicParticlesNoRotNoTSModulated(systemCameraCopy);
          } else {
            RenderBasicParticlesNoRotTSModulated(systemCameraCopy);
          }
        } else {
          if (noTS) {
            RenderBasicParticlesRotNoTSModulated(systemCameraCopy);
          } else {
            RenderBasicParticlesRotTSModulated(systemCameraCopy);
          }
        }
      } else
#endif
      {
        if (noRota) {
          if (noTS) {
            RenderBasicParticlesNoRotNoTS(systemCameraCopy);
          } else {
            RenderBasicParticlesNoRotTS(systemCameraCopy);
          }
        } else {
          if (noTS) {
            RenderBasicParticlesRotNoTS(systemCameraCopy);
          } else {
            RenderBasicParticlesRotTS(systemCameraCopy);
          }
        }
      }
    } else if (!mORNT) {
      for (int i = 0; i < particleCount; ++i) {
        CParticle& particle = SORT ? mParticles[sortItems[i].mPartIdx] : mParticles[i];
        CVector3f viewPoint =
            systemCameraCopy *
            ((particle.mPos - particle.mPrevPos) * mTimeDeltaScale + particle.mPrevPos);

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
        uint color = particle.mColor.GetColor_u32();
#else
        uint color = CColor::Modulate(particle.mColor, mModuColor).GetColor_u32();
#endif

        if (!constUVs) {
          int partFrame = mCurFrame - particle.mStartFrame - 1;
          CParticleGlobals::SetParticleLifetime(particle.mEndFrame - particle.mStartFrame);
          CParticleGlobals::UpdateParticleLifetimeTweenValues(partFrame);
          mLoadedGenDesc->mTEXR->GetValueUV(partFrame, uvs);

          if (noRota) {
            float x = (particle.mLineLengthOrSize * 0.5f) + viewPoint.GetX();
            float y = viewPoint.GetY();
            float z = (particle.mLineLengthOrSize * 0.5f) + viewPoint.GetZ();

            GXPosition3f32(x, y, z);
            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMax, uvs.yMax);

            x -= particle.mLineLengthOrSize;
            GXPosition3f32(x, y, z);
            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMin, uvs.yMax);

            z -= particle.mLineLengthOrSize;
            GXPosition3f32(x, y, z);
            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMin, uvs.yMin);

            x += particle.mLineLengthOrSize;
            GXPosition3f32(x, y, z);
            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMax, uvs.yMin);
          } else {
            float halfSize = 0.5f * particle.mLineLengthOrSize;
            float theta = particle.mLineWidthOrRota * (M_PIF / 180.f);
            float sinT = CMath::FastSinR(theta) * halfSize;
            float cosT = CMath::FastCosR(theta) * halfSize;

            float sinPlusCos = sinT + cosT;
            float cosMinusSin = cosT - sinT;
            float sinMinusCos = sinT - cosT;
            float negSinPlusCos = -sinT + cosT;
            float negCosMinusSin = -cosT - sinT;

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
            GXPosition3f32(sinPlusCos + viewPoint.GetX(), viewPoint.GetY(),
                           cosMinusSin + viewPoint.GetZ());
#else
            float vpX = viewPoint.GetX();
            float vpY = viewPoint.GetY();
            float vpZ = viewPoint.GetZ();

            GXPosition3f32(sinPlusCos + vpX, vpY, cosMinusSin + vpZ);
#endif

            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMax, uvs.yMax);

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
            GXPosition3f32(sinMinusCos + viewPoint.GetX(), viewPoint.GetY(),
                           sinPlusCos + viewPoint.GetZ());
#else
            GXPosition3f32(sinMinusCos + vpX, vpY, sinPlusCos + vpZ);
#endif

            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMin, uvs.yMax);

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
            GXPosition3f32(viewPoint.GetX() - sinPlusCos, viewPoint.GetY(),
                           viewPoint.GetZ() - cosMinusSin);
#else
            GXPosition3f32(vpX - sinPlusCos, vpY, vpZ - cosMinusSin);
#endif

            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMin, uvs.yMin);

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
            GXPosition3f32(negSinPlusCos + viewPoint.GetX(), viewPoint.GetY(),
                           negCosMinusSin + viewPoint.GetZ());
#else
            GXPosition3f32(negSinPlusCos + vpX, vpY, negCosMinusSin + vpZ);
#endif

            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMax, uvs.yMin);
          }
        } else if (noRota) {
          float x = (particle.mLineLengthOrSize * 0.5f) + viewPoint.GetX();
          float y = viewPoint.GetY();
          float z = (particle.mLineLengthOrSize * 0.5f) + viewPoint.GetZ();

          GXPosition3f32(x, y, z);
          GXColor1u32(color);
          GXTexCoord1s16(0x0202);

          x -= particle.mLineLengthOrSize;
          GXPosition3f32(x, y, z);
          GXColor1u32(color);
          GXTexCoord1s16(0x0002);

          z -= particle.mLineLengthOrSize;
          GXPosition3f32(x, y, z);
          GXColor1u32(color);
          GXTexCoord1s16(0x0000);

          x += particle.mLineLengthOrSize;
          GXPosition3f32(x, y, z);
          GXColor1u32(color);
          GXTexCoord1s16(0x0200);
        } else {
          float halfSize = 0.5f * particle.mLineLengthOrSize;
          float theta = particle.mLineWidthOrRota * (M_PIF / 180.f);
          float sinT = CMath::FastSinR(theta) * halfSize;
          float cosT = CMath::FastCosR(theta) * halfSize;

          float sinPlusCos = sinT + cosT;
          float sinMinusCos = sinT - cosT;

          float vpX = viewPoint.GetX();
          float vpY = viewPoint.GetY();
          float vpZ = viewPoint.GetZ();

          GXPosition3f32(vpX + sinPlusCos, vpY, vpZ - sinMinusCos);
          GXColor1u32(color);
          GXTexCoord1s16(0x0202);

          GXPosition3f32(vpX + sinMinusCos, vpY, vpZ + sinPlusCos);
          GXColor1u32(color);
          GXTexCoord1s16(0x0002);

          GXPosition3f32(vpX - sinPlusCos, vpY, vpZ + sinMinusCos);
          GXColor1u32(color);
          GXTexCoord1s16(0x0000);

          GXPosition3f32(vpX - sinMinusCos, vpY, vpZ - sinPlusCos);
          GXColor1u32(color);
          GXTexCoord1s16(0x0200);
        }
      }
    } else {
      // ORNT
// TODO: verify that this Prime/Echoes difference is real
      RSTL_PRIME1_CONST CVector3f fwd = systemViewCopy.GetForward();
      RSTL_PRIME1_CONST CVector3f origin = systemViewCopy.GetTranslation();

      float canBeNormEps = FLT_EPSILON;
#if RSTL_PRIME1
      CVector3f dir = CVector3f::Zero();
      CVector3f right = CVector3f::Zero();
#endif

      for (int i = 0; i < particleCount; ++i) {
        CParticle* particle = SORT ? &mParticles[sortItems[i].mPartIdx] : &mParticles[i];

#if !RSTL_PRIME1
        uint color = CColor::Modulate(particle->mColor, mModuColor).GetColor_u32();
        CVector3f dir = CVector3f::Zero();
#endif

        CVector3f pos =
            (particle->mPos - particle->mPrevPos) * mTimeDeltaScale + particle->mPrevPos;
#if RSTL_PRIME1
        uint color = particle->mColor.GetColor_u32();
#endif

        float rota = mLoadedGenDesc->mROTA == nullptr ? 1.f : particle->mLineWidthOrRota;
        float velMagSq = particle->mVel.MagSquared();

        if (velMagSq > canBeNormEps) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
          dir = particle->mVel * frsqrte(velMagSq);
#else
          dir = particle->mVel * CMath::FastInvSqrtF(velMagSq);
#endif
        } else {
          const CVector3f& dv = particle->mPos - particle->mPrevPos;
          float dMagSq = dv.MagSquared();
          if (dMagSq > canBeNormEps) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
            CVector3f tmp = dv;
            dir = tmp * frsqrte(dMagSq);
#else
            dir = dv * CMath::FastInvSqrtF(dMagSq);
#endif
          } else {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
            dir = CVector3f(0.f, 0.f, 1.f);
#else
            dir = CVector3f::Up();
#endif
          }
        }

        CVector3f fore = dir * particle->mLineLengthOrSize;

#if !RSTL_PRIME1
        CVector3f right;
#endif

        if (mLoadedGenDesc->mRSOP) {
          right = CVector3f::Cross(dir, fwd);
          float crMagSq = right.MagSquared();

          if (crMagSq > canBeNormEps) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
            float invMag = frsqrte(crMagSq);
#else
            float invMag = CMath::FastInvSqrtF(crMagSq);
#endif

            float width = particle->mLineLengthOrSize * rota;
            right *= width * invMag;
          } else {
            right = CVector3f::Cross(dir, (origin - particle->mPos).AsNormalized());
            float cr2MagSq = right.MagSquared();

            if (cr2MagSq > canBeNormEps) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
              float invMag = frsqrte(cr2MagSq);
#else
              float invMag = CMath::FastInvSqrtF(cr2MagSq);
#endif

              float width = particle->mLineLengthOrSize * rota;
              right *= width * invMag;
            }
          }
        } else {
          right = CVector3f::Cross(fore, fwd) * rota;
        }

#if !RSTL_PRIME1
        const CVector3f& vertex = pos;
#endif

        if (!constUVs) {
          int partFrame = mCurFrame - particle->mStartFrame - 1;
          CParticleGlobals::SetParticleLifetime(particle->mEndFrame - particle->mStartFrame);
          CParticleGlobals::UpdateParticleLifetimeTweenValues(partFrame);
          mLoadedGenDesc->mTEXR->GetValueUV(partFrame, uvs);

          pos += right * 0.5f;
          pos += fore * 0.5f;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
          GXPositionVector3f(pos);
#else
          GXPosition3f32(vertex.GetX(), vertex.GetY(), vertex.GetZ());
#endif

          GXColor1u32(color);
          GXTexCoord2f32(uvs.xMax, uvs.yMax);

          pos -= fore;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
          GXPositionVector3f(pos);
#else
          GXPosition3f32(vertex.GetX(), vertex.GetY(), vertex.GetZ());
#endif

          GXColor1u32(color);
          GXTexCoord2f32(uvs.xMin, uvs.yMax);

          pos -= right;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
          GXPositionVector3f(pos);
#else
          GXPosition3f32(vertex.GetX(), vertex.GetY(), vertex.GetZ());
#endif

          GXColor1u32(color);
          GXTexCoord2f32(uvs.xMin, uvs.yMin);

          pos += fore;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
          GXPositionVector3f(pos);
#else
          GXPosition3f32(vertex.GetX(), vertex.GetY(), vertex.GetZ());
#endif

          GXColor1u32(color);
          GXTexCoord2f32(uvs.xMax, uvs.yMin);
        } else {
          pos += right * 0.5f;
          pos += fore * 0.5f;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
          GXPositionVector3f(pos);
#else
          GXPosition3f32(vertex.GetX(), vertex.GetY(), vertex.GetZ());
#endif

          GXColor1u32(color);
          GXTexCoord1s16(0x0202);

          pos -= fore;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
          GXPositionVector3f(pos);
#else
          GXPosition3f32(vertex.GetX(), vertex.GetY(), vertex.GetZ());
#endif

          GXColor1u32(color);
          GXTexCoord1s16(0x0002);

          pos -= right;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
          GXPositionVector3f(pos);
#else
          GXPosition3f32(vertex.GetX(), vertex.GetY(), vertex.GetZ());
#endif

          GXColor1u32(color);
          GXTexCoord1s16(0x0000);

          pos += fore;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
          GXPositionVector3f(pos);
#else
          GXPosition3f32(vertex.GetX(), vertex.GetY(), vertex.GetZ());
#endif

          GXColor1u32(color);
          GXTexCoord1s16(0x0200);
        }
      }
    }
  } else {
    // MBLR
    float mbspFac = 1.f / static_cast< float >(mbspVal);
    for (int i = 0; i < particleCount; ++i) {
      CParticle* particle = &mParticles[i];

      CVector3f mbspVec = (particle->mPos - particle->mPrevPos) * mbspFac;
      CVector3f vec = (particle->mPos - particle->mPrevPos) * mTimeDeltaScale + particle->mPrevPos;

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      CColor col = particle->mColor;
#else
      CColor col = CColor::Modulate(particle->mColor, mModuColor);
#endif

      col.SetAlpha(col.GetAlpha() * mbspFac);
      uint color = col.GetColor_u32();

      if (!constUVs) {
        int partFrame = mCurFrame - particle->mStartFrame - 1;
        CParticleGlobals::SetParticleLifetime(particle->mEndFrame - particle->mStartFrame);
        CParticleGlobals::UpdateParticleLifetimeTweenValues(partFrame);
        mLoadedGenDesc->mTEXR->GetValueUV(partFrame, uvs);

        if (noRota) {
          for (int j = 0; j < mbspVal; ++j) {
            vec += mbspVec;
            CVector3f viewPoint = systemCameraCopy * vec;
            float x = (0.5f * particle->mLineLengthOrSize) + viewPoint.GetX();
            float y = viewPoint.GetY();
            float z = (0.5f * particle->mLineLengthOrSize) + viewPoint.GetZ();

            GXPosition3f32(x, y, z);
            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMax, uvs.yMax);

            x -= particle->mLineLengthOrSize;
            GXPosition3f32(x, y, z);
            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMin, uvs.yMax);

            z -= particle->mLineLengthOrSize;
            GXPosition3f32(x, y, z);
            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMin, uvs.yMin);

            x += particle->mLineLengthOrSize;
            GXPosition3f32(x, y, z);
            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMax, uvs.yMin);
          }
        } else {
          for (int j = 0; j < mbspVal; ++j) {
            vec += mbspVec;
            CVector3f viewPoint = systemCameraCopy * vec;
            float halfSize = 0.5f * particle->mLineLengthOrSize;
            float theta = particle->mLineWidthOrRota * (M_PIF / 180.f);

            float sinT = CMath::FastSinR(theta) * halfSize;
            float cosT = CMath::FastCosR(theta) * halfSize;

            float sinPlusCos = sinT + cosT;
            float negSinPlusCos = -sinT + cosT;
            float negCosMinusSin = -cosT - sinT;
            float sinMinusCos = sinT - cosT;
            float cosMinusSin = cosT - sinT;

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
            GXPosition3f32(sinPlusCos + viewPoint.GetX(), viewPoint.GetY(),
                           cosMinusSin + viewPoint.GetZ());
#else
            float vpX = viewPoint.GetX();
            float vpY = viewPoint.GetY();
            float vpZ = viewPoint.GetZ();

            GXPosition3f32(sinPlusCos + vpX, vpY, cosMinusSin + vpZ);
#endif

            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMax, uvs.yMax);

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
            GXPosition3f32(sinMinusCos + viewPoint.GetX(), viewPoint.GetY(),
                           sinPlusCos + viewPoint.GetZ());
#else
            GXPosition3f32(sinMinusCos + vpX, vpY, sinPlusCos + vpZ);
#endif

            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMin, uvs.yMax);

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
            GXPosition3f32(viewPoint.GetX() - sinPlusCos, viewPoint.GetY(),
                           viewPoint.GetZ() - cosMinusSin);
#else
            GXPosition3f32(vpX - sinPlusCos, vpY, vpZ - cosMinusSin);
#endif

            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMin, uvs.yMin);

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
            GXPosition3f32(negSinPlusCos + viewPoint.GetX(), viewPoint.GetY(),
                           negCosMinusSin + viewPoint.GetZ());
#else
            GXPosition3f32(negSinPlusCos + vpX, vpY, negCosMinusSin + vpZ);
#endif

            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMax, uvs.yMin);
          }
        }
      } else if (noRota) {
        for (int j = 0; j < mbspVal; ++j) {
          vec += mbspVec;
          CVector3f viewPoint = systemCameraCopy * vec;
          float x = (0.5f * particle->mLineLengthOrSize) + viewPoint.GetX();
          float y = viewPoint.GetY();
          float z = (0.5f * particle->mLineLengthOrSize) + viewPoint.GetZ();

          GXPosition3f32(x, y, z);
          GXColor1u32(color);
          GXTexCoord1s16(0x0202);

          x -= particle->mLineLengthOrSize;
          GXPosition3f32(x, y, z);
          GXColor1u32(color);
          GXTexCoord1s16(0x0002);

          z -= particle->mLineLengthOrSize;
          GXPosition3f32(x, y, z);
          GXColor1u32(color);
          GXTexCoord1s16(0x0000);

          x += particle->mLineLengthOrSize;
          GXPosition3f32(x, y, z);
          GXColor1u32(color);
          GXTexCoord1s16(0x0200);
        }
      } else {
        for (int j = 0; j < mbspVal; ++j) {
          vec += mbspVec;
          CVector3f viewPoint = systemCameraCopy * vec;
          float halfSize = 0.5f * particle->mLineLengthOrSize;
          float theta = particle->mLineWidthOrRota * (M_PIF / 180.f);

          float sinT = CMath::FastSinR(theta) * halfSize;
          float cosT = CMath::FastCosR(theta) * halfSize;
          float sinPlusCos = sinT + cosT;
          float sinMinusCos = sinT - cosT;

          float vpX = viewPoint.GetX();
          float vpY = viewPoint.GetY();
          float vpZ = viewPoint.GetZ();

          GXPosition3f32(vpX + sinPlusCos, vpY, vpZ - sinMinusCos);
          GXColor1u32(color);
          GXTexCoord1s16(0x0202);

          GXPosition3f32(vpX + sinMinusCos, vpY, vpZ + sinPlusCos);
          GXColor1u32(color);
          GXTexCoord1s16(0x0002);

          GXPosition3f32(vpX - sinPlusCos, vpY, vpZ + sinMinusCos);
          GXColor1u32(color);
          GXTexCoord1s16(0x0000);

          GXPosition3f32(vpX - sinMinusCos, vpY, vpZ - sinPlusCos);
          GXColor1u32(color);
          GXTexCoord1s16(0x0200);
        }
      }
    }
  }

  CGX::End();
  if (modulateAlpha) {
    GXSetTevSwapMode(static_cast< GXTevStageID >(nextStage - 1), GX_TEV_SWAP0, GX_TEV_SWAP0);
  }
#if !RSTL_PRIME1
  if (sMoveRedToAlphaBuffer) {
    GXSetAlphaUpdate(GX_TRUE);
  }
#endif
  CGraphics::SetCullMode(kCM_Front);
  CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_And, kAF_Always, 0);
}

void CElementGen::RenderParticlesFlameThrower(CElementGen* const* gens, int count
#if !RSTL_PRIME1
                                              ,
                                              const CVector3f* globalTranslation,
                                              const CTransform4f* globalOrientation,
                                              const CVector3f* globalScale,
                                              const CTransform4f* localScale
#endif
) {
  CTransform4f viewCopy(CGraphics::GetViewMatrix());
  CTransform4f systemModelMatrix(viewCopy);
  systemModelMatrix.SetTranslation(CVector3f::Zero());
  CTransform4f systemCameraCopy(systemModelMatrix.GetQuickInverse());
#if !RSTL_PRIME1
  if (globalOrientation) {
    systemCameraCopy *= *globalOrientation;
  }
  systemModelMatrix =
      (globalTranslation ? CTransform4f::Translate(*globalTranslation) : CTransform4f::Identity()) *
      (globalScale ? CTransform4f::Scale(*globalScale) : CTransform4f::Identity()) *
      systemModelMatrix * (localScale ? *localScale : CTransform4f::Identity());
  const CTransform4f orientedModelMatrix(systemModelMatrix * systemCameraCopy);
#endif

  CGraphics::SetModelMatrix(systemModelMatrix);
  CGraphics::SetCullMode(kCM_None);
  CGraphics::SetDepthWriteMode(true, kE_LEqual, false);
  CGraphics::SetAlphaCompare(kAF_Greater, 0, kAO_And, kAF_Always, 0);
  CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
  CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const bool modulateAlpha = sMoveRedToAlphaBuffer;
  SUVElementSet uvs;
#else
  const bool modulateAlpha = sEnableAlphaModulation;
#endif

  for (int i = 0; i < count; ++i) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    const TLockedToken< CTexture >& tex = gens[i]->mLoadedGenDesc->mTEXR->GetValueTexture(0);
    (*tex)->Load(static_cast< GXTexMapID >(i), CTexture::kCM_Repeat);
#else
    gens[i]->mLoadedGenDesc->mTEXR->GetValueTexture(0)->Load(static_cast< GXTexMapID >(i),
                                                             CTexture::kCM_Repeat);
#endif
  }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static const GXVtxDescList sVtxDescList[] = {
#else
  static const GXVtxDescList skVtxDescList[] = {
#endif
      {GX_VA_POS, GX_DIRECT},
      {GX_VA_CLR0, GX_DIRECT},
      {GX_VA_TEX0, GX_DIRECT},
      {GX_VA_NULL, GX_NONE},
  };
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CGX::SetVtxDescv(sVtxDescList);

  int activeCount;
#else
  CGX::SetVtxDescv(skVtxDescList);
#endif
  int nStages = 1;

  CGX::SetNumChans(1);

  GXTevStageID stage = GX_TEVSTAGE0;
  if (modulateAlpha) {
    CGraphics::SetBlendMode(kBM_Blend, kBF_One, kBF_One, kLO_Clear);
    stage = GX_TEVSTAGE1;
    CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_CPREV, GX_CC_APREV, GX_CC_ZERO);
    CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_TEXA, GX_CA_APREV, GX_CA_ZERO);
    CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE1);
    GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP1);
    nStages = 2;
  }
#if !RSTL_PRIME1
  if (sMoveRedToAlphaBuffer) {
    GXSetAlphaUpdate(GX_FALSE);
  }
#endif
  CGX::SetNumTevStages(nStages);
  CGX::SetNumTexGens(1);
  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);

  GXSetVtxAttrFmt(GX_VTXFMT6, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
  GXSetVtxAttrFmt(GX_VTXFMT6, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
  GXSetVtxAttrFmt(GX_VTXFMT6, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

  int totalParticles = 0;
#if !RSTL_PRIME1
  CTevCombiners::ETevScale scales[8];
  rstl::reserved_vector< CColor, 8 > colors;
#endif

  for (int i = 0; i < count; ++i) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    totalParticles += gens[i]->GetParticleCount();
#else
    CElementGen* gen = gens[i];
    totalParticles += gen->GetParticleCount();
    const CGenDescription* desc = gen->mLoadedGenDesc;
    if (desc->mXTAD) {
      int scale = 1;
      desc->mXTAD->GetValue(gen->GetEmitterTime(), scale);
      switch (scale) {
      case 2:
        scales[i] = CTevCombiners::kTS_Scale2;
        break;
      case 4:
        scales[i] = CTevCombiners::kTS_Scale4;
        break;
      default:
        scales[i] = CTevCombiners::kTS_Scale1;
        break;
      }
    } else {
      scales[i] = CTevCombiners::kTS_Scale1;
    }
    colors.push_back(gen->GetModulationColor());
#endif
  }

  CTexturedParticleListItem* sortItems = static_cast< CTexturedParticleListItem* >(
      alloca(totalParticles * sizeof(CTexturedParticleListItem)));
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  activeCount = 0;
#else
  int activeCount = 0;
#endif

  for (int i = 0; i < count; ++i) {
    CElementGen* gen = gens[i];
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    int numParts = gen->GetParticleCount();
    float timeDelta = gen->mTimeDeltaScale;
#else
    const int numParts = gen->GetParticleCount();
    const float timeDelta = gen->GetTimeDeltaScale();
#endif
    for (int j = 0; j < numParts; ++j) {
      CParticle& particle = gen->mParticles[static_cast< uint >(j)];
      if (particle.mEndFrame == -1) {
        continue;
      }
#if RSTL_PRIME1
      // ?
      sortItems[activeCount].mViewPoint = systemCameraCopy * particle.mPos;

      // TODO: this should be the constructor
#endif
      sortItems[activeCount].mViewPoint =
          systemCameraCopy * ((particle.mPos - particle.mPrevPos) * timeDelta + particle.mPrevPos);
      sortItems[activeCount].mTexMapIdx = i;
      sortItems[activeCount].mPartIdx = j;
      ++activeCount;
    }
  }

  rstl::sort(sortItems, sortItems + activeCount, CTexturedParticleListItemViewPointComp());

  ushort lastMap = 0xFFFF;
  CElementGen* gen = nullptr;
  CGenDescription* genDesc = nullptr;
  int emitterTime = 0;
#if !RSTL_PRIME1
  bool packedUV = false;
  int lastScale = -1;
  bool oriented = false;
  CColor modulationColor = CColor::White();
  const CVector3f cameraPosition = viewCopy.GetTranslation();
  const CVector3f cameraForward = viewCopy.GetForward();
  SUVElementSet uvs;
#endif

#if RSTL_PRIME1
  float cosMinusSin;
  float sinPlusCos;
#endif

  for (int i = 0; i < activeCount; ++i) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    CTexturedParticleListItem* readPtr = &sortItems[i];
    ushort map = readPtr->mTexMapIdx;
#else
    const CTexturedParticleListItem& item = sortItems[i];
    const ushort map = item.mTexMapIdx;
#endif

    if (lastMap != map) {
      gen = gens[map];
      emitterTime = gen->GetEmitterTime();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      CGenDescription* desc = gens[map]->mLoadedGenDesc;
      genDesc = desc;
#else
      genDesc = gens[map]->mLoadedGenDesc;
#endif
      if (!modulateAlpha) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
        if (desc->mAAPH) {
#else
        if (genDesc->mAAPH) {
#endif
          CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_One, kLO_Clear);
        } else {
          CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
        }
      } else {
        CGX::SetTevOrder(stage, GX_TEXCOORD0, static_cast< GXTexMapID >(map), GX_COLOR_NULL);
      }

      CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, static_cast< GXTexMapID >(map), GX_COLOR0A0);
      lastMap = map;
#if !RSTL_PRIME1
      if (lastScale != scales[map]) {
        lastScale = scales[map];
        if (scales[map] == CTevCombiners::kTS_Scale1) {
          CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
        } else {
          SetParticleTevScale(scales[map]);
        }
      }
      if (genDesc->mORNT && !oriented) {
        oriented = true;
        CGraphics::SetModelMatrix(orientedModelMatrix);
      } else if (oriented) {
        oriented = false;
        CGraphics::SetModelMatrix(systemModelMatrix);
      }
      modulationColor = colors[map];
#endif
    }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    CParticle& particle = gen->mParticles[readPtr->mPartIdx];

    int elapsed = emitterTime - particle.mStartFrame - 1;
#else
    CParticle& particle = gen->mParticles[item.mPartIdx];
    const int elapsed = emitterTime - particle.mStartFrame - 1;
#endif

    CParticleGlobals::SetParticleLifetime(particle.mEndFrame - particle.mStartFrame);
    CParticleGlobals::UpdateParticleLifetimeTweenValues(elapsed);

#if !RSTL_PRIME1
    if (!oriented) {
      if (packedUV) {
        packedUV = false;
        GXSetVtxAttrFmt(GX_VTXFMT6, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
      }
#endif

    genDesc->mTEXR->GetValueUV(elapsed, uvs);
#if !RSTL_PRIME1
      const uint color = CColor::Modulate(particle.mColor, modulationColor).GetColor_u32();
      CGX::Begin(GX_QUADS, GX_VTXFMT6, 4);
      const float halfSize = 0.5f * particle.mLineLengthOrSize;
      const float theta = particle.mLineWidthOrRota * (M_PIF / 180.f);
      const float sinT = halfSize * CMath::FastSinR(theta);
      const float cosT = halfSize * CMath::FastCosR(theta);
      const CVector3f& viewPoint = item.mViewPoint;
#endif

#if !RSTL_PRIME1
      GXPosition3f32(viewPoint.GetX() + (sinT + cosT), viewPoint.GetY(),
                     viewPoint.GetZ() + (cosT - sinT));
      GXColor1u32(color);
      GXTexCoord2f32(uvs.xMax, uvs.yMax);
      GXPosition3f32(viewPoint.GetX() + (sinT - cosT), viewPoint.GetY(),
                     viewPoint.GetZ() + (cosT + sinT));
      GXColor1u32(color);
      GXTexCoord2f32(uvs.xMin, uvs.yMax);
      GXPosition3f32(viewPoint.GetX() - (sinT + cosT), viewPoint.GetY(),
                     viewPoint.GetZ() - (cosT - sinT));
      GXColor1u32(color);
      GXTexCoord2f32(uvs.xMin, uvs.yMin);
      GXPosition3f32(viewPoint.GetX() + (-sinT + cosT), viewPoint.GetY(),
                     viewPoint.GetZ() + (-cosT - sinT));
      GXColor1u32(color);
      GXTexCoord2f32(uvs.xMax, uvs.yMin);
      CGX::End();
    } else {
      const uint color = CColor::Modulate(particle.mColor, modulationColor).GetColor_u32();
      const bool constantUV = genDesc->mTEXR && genDesc->mTEXR->HasConstantUV();
      if (constantUV && !packedUV) {
        packedUV = true;
        GXSetVtxAttrFmt(GX_VTXFMT6, GX_VA_TEX0, GX_TEX_ST, GX_S8, 1);
      }
#endif

    CGX::Begin(GX_QUADS, GX_VTXFMT6, 4);

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    float halfSize = 0.5f * particle.mLineLengthOrSize;
    float theta = particle.mLineWidthOrRota * (M_PIF / 180.f);
    uint color = particle.mColor.GetColor_u32();
    float sinT = halfSize * CMath::FastSinR(theta);
    float cosT = halfSize * CMath::FastCosR(theta);
#else
      CVector3f direction = CVector3f::Zero();
      const CVector3f travel = particle.mPos - particle.mPrevPos;
      CVector3f position = travel * gen->mTimeDeltaScale + particle.mPrevPos;
      const float widthFactor = !gen->mLoadedGenDesc->mROTA ? 1.f : particle.mLineWidthOrRota;
      const CVector3f& velocity = particle.mVel;
      float magnitudeSquared = velocity.MagSquared();
      if (magnitudeSquared > FLT_EPSILON) {
        direction = CMath::FastInvSqrtF(magnitudeSquared) * velocity;
      } else {
        const CVector3f& delta = particle.mPos - particle.mPrevPos;
        magnitudeSquared = delta.MagSquared();
        if (magnitudeSquared > FLT_EPSILON) {
          direction = CMath::FastInvSqrtF(magnitudeSquared) * delta;
        } else {
          direction = CVector3f::Up();
        }
      }
      const CVector3f length = particle.mLineLengthOrSize * direction;
      CVector3f width;
      if (gen->mLoadedGenDesc->mRSOP) {
        width = CVector3f::Cross(direction, cameraForward);
        magnitudeSquared = width.MagSquared();
        if (magnitudeSquared > FLT_EPSILON) {
          width *= particle.mLineLengthOrSize * widthFactor * CMath::FastInvSqrtF(magnitudeSquared);
        } else {
          width = CVector3f::Cross(direction, (cameraPosition - particle.mPos).AsNormalized());
          magnitudeSquared = width.MagSquared();
          if (magnitudeSquared > FLT_EPSILON) {
            width *=
                particle.mLineLengthOrSize * widthFactor * CMath::FastInvSqrtF(magnitudeSquared);
          }
        }
      } else {
        width = widthFactor * CVector3f::Cross(length, cameraForward);
      }
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    sinPlusCos = sinT + cosT;
    cosMinusSin = cosT - sinT;
    float sinMinusCos = sinT - cosT;
    float negSinPlusCos = -sinT + cosT;
    float negCos = -cosT;

    GXPosition3f32(sinPlusCos + readPtr->mViewPoint.GetX(), readPtr->mViewPoint.GetY(),
                   cosMinusSin + readPtr->mViewPoint.GetZ());
#else
      const CVector3f& vertex = position;
      if (!constantUV) {
        const int frame = gen->GetEmitterTime() - particle.mStartFrame - 1;
        CParticleGlobals::SetParticleLifetime(particle.mEndFrame - particle.mStartFrame);
        CParticleGlobals::UpdateParticleLifetimeTweenValues(frame);
        gen->mLoadedGenDesc->mTEXR->GetValueUV(frame, uvs);
        position += 0.5f * width;
        position += 0.5f * length;
        GXPosition3f32(vertex.GetX(), vertex.GetY(), vertex.GetZ());
#endif

    GXColor1u32(color);
    GXTexCoord2f32(uvs.xMax, uvs.yMax);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    GXPosition3f32(sinMinusCos + readPtr->mViewPoint.GetX(), readPtr->mViewPoint.GetY(),
                   sinPlusCos + readPtr->mViewPoint.GetZ());
#else
        position -= length;
        GXPosition3f32(vertex.GetX(), vertex.GetY(), vertex.GetZ());
#endif

    GXColor1u32(color);
    GXTexCoord2f32(uvs.xMin, uvs.yMax);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    GXPosition3f32(readPtr->mViewPoint.GetX() - sinPlusCos, readPtr->mViewPoint.GetY(),
                   readPtr->mViewPoint.GetZ() - cosMinusSin);
#else
        position -= width;
        GXPosition3f32(vertex.GetX(), vertex.GetY(), vertex.GetZ());
#endif

    GXColor1u32(color);
    GXTexCoord2f32(uvs.xMin, uvs.yMin);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    GXPosition3f32(negSinPlusCos + readPtr->mViewPoint.GetX(), readPtr->mViewPoint.GetY(),
                   (negCos - sinT) + readPtr->mViewPoint.GetZ());
#else
        position += length;
        GXPosition3f32(vertex.GetX(), vertex.GetY(), vertex.GetZ());
#endif

    GXColor1u32(color);
    GXTexCoord2f32(uvs.xMax, uvs.yMin);
#if !RSTL_PRIME1
      } else {
        position += 0.5f * width;
        position += 0.5f * length;
        GXPosition3f32(vertex.GetX(), vertex.GetY(), vertex.GetZ());
        GXColor1u32(color);
        GXTexCoord1s16(0x0202);
        position -= length;
        GXPosition3f32(vertex.GetX(), vertex.GetY(), vertex.GetZ());
        GXColor1u32(color);
        GXTexCoord1s16(0x0002);
        position -= width;
        GXPosition3f32(vertex.GetX(), vertex.GetY(), vertex.GetZ());
        GXColor1u32(color);
        GXTexCoord1s16(0x0000);
        position += length;
        GXPosition3f32(vertex.GetX(), vertex.GetY(), vertex.GetZ());
        GXColor1u32(color);
        GXTexCoord1s16(0x0200);
      }
#endif

    CGX::End();
  }
#if !RSTL_PRIME1
  }
#endif

  if (modulateAlpha) {
    GXSetTevSwapMode(stage, GX_TEV_SWAP0, GX_TEV_SWAP0);
  }
#if !RSTL_PRIME1
  if (sMoveRedToAlphaBuffer) {
    GXSetAlphaUpdate(GX_TRUE);
  }
#endif
  CGraphics::SetCullMode(kCM_Front);
  CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_And, kAF_Always, 0);
}

void CElementGen::RenderParticlesIndirectTexture() {
#if !RSTL_PRIME1
  CGlobalRandom gr(mRandState);
  if (sMoveRedToAlphaBuffer) {
    GXSetAlphaUpdate(GX_FALSE);
  }
#endif

  CTransform4f systemViewCopy(CGraphics::GetViewMatrix());
  systemViewCopy.SetTranslation(CVector3f::Zero());
  CTransform4f systemCameraCopy(systemViewCopy.GetQuickInverse() * mGlobalOrientation);
  systemViewCopy = CTransform4f::Translate(mGlobalTranslation) * mGlobalScaleTransform *
                   systemViewCopy * mLocalScaleTransform;
  CGraphics::SetModelMatrix(systemViewCopy);

  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);

  if (mAAPH) {
    CGX::SetZMode(true, GX_LEQUAL, false);
    CGX::SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_CLEAR);
  } else {
    CGX::SetZMode(true, GX_LEQUAL, mZBUF);
    CGX::SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
  }

  SUVElementSet uvs;
  uvs.xMin = 0.f;
  uvs.xMax = 1.f;
  uvs.yMin = 0.f;
  uvs.yMax = 1.f;
  SUVElementSet uvsInd;
  uvsInd.xMin = 0.f;
  uvsInd.xMax = 1.f;
  uvsInd.yMin = 0.f;
  uvsInd.yMax = 1.f;

  TToken< CTexture > texToken =
      mLoadedGenDesc->mTEXR->GetValueTexture(mCurFrame - mParticles[0].mStartFrame);
  texToken->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
  CTexture* cachedTex = *texToken;

  bool constTexr = mLoadedGenDesc->mTEXR->HasConstantTexture();
  mLoadedGenDesc->mTEXR->GetValueUV(mCurFrame - mParticles[0].mStartFrame, uvs);
  bool constUVs = mLoadedGenDesc->mTEXR->HasConstantUV();

  TToken< CTexture > indTexToken =
      mLoadedGenDesc->mTIND->GetValueTexture(mCurFrame - mParticles[0].mStartFrame);
  indTexToken->Load(GX_TEXMAP2, CTexture::kCM_Repeat);
  CTexture* cachedIndTex = *indTexToken;

  bool constIndTexr = mLoadedGenDesc->mTIND->HasConstantTexture();
  bool constIndUVs = mLoadedGenDesc->mTIND->HasConstantUV();
  mLoadedGenDesc->mTIND->GetValueUV(mCurFrame - mParticles[0].mStartFrame, uvsInd);

  CGX::SetNumTexGens(3);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX1, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX2, GX_IDENTITY, false, GX_PTIDENTITY);

  float indMtx[2][3] = {
      {0.5f, 0.f, 0.f},
      {0.f, 0.5f, 0.f},
  };
  GXSetIndTexMtx(GX_ITM_0, indMtx, 1);
  GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD2, GX_TEXMAP2);

  CGX::SetTevIndirect(GX_TEVSTAGE1, GX_INDTEXSTAGE0, GX_ITF_8, GX_ITB_STU, GX_ITM_0, GX_ITW_OFF,
                      GX_ITW_OFF, false, false, GX_ITBA_OFF);
  CGX::SetNumIndStages(1);

  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
  CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, CGraphics::kSpareBufferTexMapID, GX_COLOR0A0);
  CGX::SetNumTevStages(2);

  CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_8_8);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE1, GX_TEV_KASEL_8_8);

  if (!mLoadedGenDesc->mCIND) {
    CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_ONE, GX_CC_ZERO);
    CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXC, GX_CC_RASC, GX_CC_CPREV);
  } else {
    CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_RASC, GX_CC_ZERO);
    CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXC, GX_CC_CPREV, GX_CC_ZERO);
  }

  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_ZERO);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE1);
  CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_APREV, GX_CA_RASA, GX_CA_ZERO);

  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
  CGX::SetNumChans(1);
  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);

  CParticleListItem* sortItems = nullptr;
  int particleCount = mParticles.size();
  bool SORT = mLoadedGenDesc->mSORT;

  if (SORT) {
    sortItems =
        static_cast< CParticleListItem* >(alloca(particleCount * sizeof(CParticleListItem)));
    for (int i = 0; i < particleCount; ++i) {
      CParticle& particle = mParticles[i];
      sortItems[i].mViewPoint =
          systemCameraCopy *
          ((particle.mPos - particle.mPrevPos) * mTimeDeltaScale + particle.mPrevPos);
      sortItems[i].mPartIdx = static_cast< ushort >(i);
    }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1 && RSTL_VERSION >= RSTL_R3IJ
    rstl::sort(sortItems, sortItems + particleCount, CParticleListItemViewPointComp());
#else
    static CParticleListItemViewPointComp sComp;
    rstl::sort(sortItems, sortItems + particleCount, sComp);
#endif
  }

  static const GXVtxDescList skIndVtxDescList[] = {
      {GX_VA_POS, GX_DIRECT},  {GX_VA_CLR0, GX_DIRECT}, {GX_VA_TEX0, GX_DIRECT},
      {GX_VA_TEX1, GX_DIRECT}, {GX_VA_TEX2, GX_DIRECT}, {GX_VA_NULL, GX_NONE},
  };
  CGX::SetVtxDescv(skIndVtxDescList);

  for (int i = 0; i < particleCount; ++i) {
    CParticle* particle = SORT ? &mParticles[sortItems[i].mPartIdx] : &mParticles[i];
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1 && RSTL_VERSION >= RSTL_R3IJ
    const CVector3f viewPoint =
        systemCameraCopy *
        ((particle->mPos - particle->mPrevPos) * mTimeDeltaScale + particle->mPrevPos);
#else
#if RSTL_PRIME1
    const CVector3f& viewPoint =
#else
    const CVector3f viewPoint =
#endif
        systemCameraCopy *
        ((particle->mPos - particle->mPrevPos) * mTimeDeltaScale + particle->mPrevPos);

    float vpX = viewPoint.GetX();
    float vpY = viewPoint.GetY();
    float vpZ = viewPoint.GetZ();
#endif

    if (!constTexr) {
      TToken< CTexture > tex =
          mLoadedGenDesc->mTEXR->GetValueTexture(mCurFrame - particle->mStartFrame);
      if (*tex != cachedTex) {
        tex->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
        cachedTex = *tex;
      }
    }

    if (!constIndTexr) {
      TToken< CTexture > tex =
          mLoadedGenDesc->mTIND->GetValueTexture(mCurFrame - particle->mStartFrame);
      if (*tex != cachedIndTex) {
        tex->Load(GX_TEXMAP2, CTexture::kCM_Repeat);
        cachedIndTex = *tex;
      }
    }

    if (!constUVs) {
      mLoadedGenDesc->mTEXR->GetValueUV(mCurFrame - mParticles[i].mStartFrame, uvs);
    }

    if (!constIndUVs) {
      mLoadedGenDesc->mTIND->GetValueUV(mCurFrame - mParticles[i].mStartFrame, uvsInd);
    }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    float size = particle->mLineLengthOrSize * 0.5f;
#else
    float sinPlusCos;
    float cosMinusSin;
    const float halfSize = 0.5f * particle->mLineLengthOrSize;
    const float theta = particle->mLineWidthOrRota * (M_PIF / 180.f);
    const float sinT = halfSize * CMath::FastSinR(theta);
    const float cosT = halfSize * CMath::FastCosR(theta);
    cosMinusSin = cosT - sinT;
    sinPlusCos = sinT + cosT;
    const float sinMinusCos = sinT - cosT;
    const float negSinPlusCos = -sinT + cosT;
    const float negCosMinusSin = -cosT - sinT;
    CGraphics::CClippedScreenQuad clipRect = CGraphics::ClipScreenQuadFromMS(
        CVector3f(sinPlusCos + vpX, vpY, cosMinusSin + vpZ),
        CVector3f(sinMinusCos + vpX, vpY, sinPlusCos + vpZ),
        CVector3f(vpX - sinPlusCos, vpY, vpZ - cosMinusSin),
        CVector3f(negSinPlusCos + vpX, vpY, negCosMinusSin + vpZ), kTF_RGB565);
    if (!clipRect.IsValid()) {
      continue;
    }
    const int indScale = mLoadedGenDesc->mINDM ? 1 : 0;
    void* const dest = CGraphics::GetDolphinSpareBuffer();
    GXSetTexCopySrc(static_cast< u16 >(clipRect.GetX()), static_cast< u16 >(clipRect.GetY()),
                    static_cast< u16 >(clipRect.GetWidth()),
                    static_cast< u16 >(clipRect.GetHeight()));
    GXSetTexCopyDst(clipRect.GetTexWidth() >> indScale, clipRect.GetHeight() >> indScale,
                    GX_TF_RGB565, indScale ? GX_TRUE : GX_FALSE);
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_R3IJ
    CGraphics::CClippedScreenRect clipRect = CGraphics::ClipScreenRectFromMS(
        CVector3f(viewPoint.GetX() - size, viewPoint.GetY(), viewPoint.GetZ() - size),
        CVector3f(size + viewPoint.GetX(), viewPoint.GetY(), size + viewPoint.GetZ()), kTF_RGB565);

    if (clipRect.IsValid()) {
      void* const dest = CGraphics::GetDolphinSpareBuffer();
      GXSetTexCopySrc(static_cast< u16 >(clipRect.GetX()), static_cast< u16 >(clipRect.GetY()),
                      static_cast< u16 >(clipRect.GetWidth()),
                      static_cast< u16 >(clipRect.GetHeight()));
      GXSetTexCopyDst(static_cast< u16 >(clipRect.GetTexWidth()),
                      static_cast< u16 >(clipRect.GetHeight()), GX_TF_RGB565, GX_FALSE);

      int bufSize = CGraphics::GetSpareBufferSize();
      size_t texBufSize = GXGetTexBufferSize(clipRect.GetTexWidth(), clipRect.GetHeight(),
                                             GX_TF_RGB565, GX_FALSE, 0);
      if (texBufSize <= bufSize) {
        const bool useVideoFilter = CGraphics::GetUseVideoFilter();
        CGraphics::SetUseVideoFilter(false);
        GXCopyTex(dest, GX_FALSE);
        CGraphics::SetUseVideoFilter(useVideoFilter);
        GXPixModeSync();

        CGraphics::LoadDolphinSpareTexture(clipRect.GetTexWidth(), clipRect.GetHeight(),
                                           GX_TF_RGB565, NULL, CGraphics::kSpareBufferTexMapID);

        uint color = particle->mColor.GetColor_u32();
        CGX::Begin(GX_QUADS, GX_VTXFMT0, 4);

        GXPosition3f32(size + viewPoint.GetX(), viewPoint.GetY(), size + viewPoint.GetZ());
        GXColor1u32(color);
        GXTexCoord2f32(uvs.xMax, uvs.yMax);
        GXTexCoord2f32(clipRect.GetMaxU(), clipRect.GetMinV());
        GXTexCoord2f32(uvsInd.xMax, uvsInd.yMax);

        GXPosition3f32(viewPoint.GetX() - size, viewPoint.GetY(), size + viewPoint.GetZ());
        GXColor1u32(color);
        GXTexCoord2f32(uvs.xMin, uvs.yMax);
        GXTexCoord2f32(clipRect.GetMinU(), clipRect.GetMinV());
        GXTexCoord2f32(uvsInd.xMin, uvsInd.yMax);

        GXPosition3f32(viewPoint.GetX() - size, viewPoint.GetY(), viewPoint.GetZ() - size);
        GXColor1u32(color);
        GXTexCoord2f32(uvs.xMin, uvs.yMin);
        GXTexCoord2f32(clipRect.GetMinU(), clipRect.GetMaxV());
        GXTexCoord2f32(uvsInd.xMin, uvsInd.yMin);

        GXPosition3f32(size + viewPoint.GetX(), viewPoint.GetY(), viewPoint.GetZ() - size);
        GXColor1u32(color);
        GXTexCoord2f32(uvs.xMax, uvs.yMin);
        GXTexCoord2f32(clipRect.GetMaxU(), clipRect.GetMaxV());
        GXTexCoord2f32(uvsInd.xMax, uvsInd.yMin);

        CGX::End();
#else
    CGraphics::CClippedScreenRect clipRect = CGraphics::ClipScreenRectFromMS(
        CVector3f(vpX - size, vpY, vpZ - size), CVector3f(size + vpX, vpY, size + vpZ), kTF_RGB565);
    const int x = clipRect.GetX();
    const int y = clipRect.GetY();
    const int width = clipRect.GetWidth();
    const int height = clipRect.GetHeight();
    const int texWidth = clipRect.GetTexWidth();
    float minU = clipRect.GetMinU();
    float maxU = clipRect.GetMaxU();
    float minV = clipRect.GetMinV();
    float maxV = clipRect.GetMaxV();

    if (clipRect.IsValid()) {
      void* const dest = CGraphics::GetDolphinSpareBuffer();
      GXSetTexCopySrc(x, y, width, height);
      GXSetTexCopyDst(texWidth, height, GX_TF_RGB565, GX_FALSE);

      int bufSize = CGraphics::GetSpareBufferSize();
      size_t texBufSize = GXGetTexBufferSize(texWidth, height, GX_TF_RGB565, GX_FALSE, 0);
      if (texBufSize <= bufSize) {
        const bool useVideoFilter = CGraphics::GetUseVideoFilter();
        CGraphics::SetUseVideoFilter(false);
        GXCopyTex(dest, GX_FALSE);
        CGraphics::SetUseVideoFilter(useVideoFilter);
        GXPixModeSync();

        CGraphics::LoadDolphinSpareTexture(texWidth, height, GX_TF_RGB565, NULL,
                                           CGraphics::kSpareBufferTexMapID);

        uint color = particle->mColor.GetColor_u32();
        CGX::Begin(GX_QUADS, GX_VTXFMT0, 4);

        GXPosition3f32(size + vpX, vpY, size + vpZ);
        GXColor1u32(color);
        GXTexCoord2f32(uvs.xMax, uvs.yMax);
        GXTexCoord2f32(maxU, minV);
        GXTexCoord2f32(uvsInd.xMax, uvsInd.yMax);

        GXPosition3f32(vpX - size, vpY, size + vpZ);
        GXColor1u32(color);
        GXTexCoord2f32(uvs.xMin, uvs.yMax);
        GXTexCoord2f32(minU, minV);
        GXTexCoord2f32(uvsInd.xMin, uvsInd.yMax);

        GXPosition3f32(vpX - size, vpY, vpZ - size);
        GXColor1u32(color);
        GXTexCoord2f32(uvs.xMin, uvs.yMin);
        GXTexCoord2f32(minU, maxV);
        GXTexCoord2f32(uvsInd.xMin, uvsInd.yMin);

        GXPosition3f32(size + vpX, vpY, vpZ - size);
        GXColor1u32(color);
        GXTexCoord2f32(uvs.xMax, uvs.yMin);
        GXTexCoord2f32(maxU, maxV);
        GXTexCoord2f32(uvsInd.xMax, uvsInd.yMin);

        CGX::End();
#endif
#else
    int bufSize = CGraphics::GetSpareBufferSize();
    if (GXGetTexBufferSize(clipRect.GetTexWidth() >> indScale, clipRect.GetHeight() >> indScale,
                           GX_TF_RGB565, GX_FALSE, 0) > bufSize) {
      continue;
#endif
      }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    }
#else
    bool useVideoFilter = CGraphics::GetUseVideoFilter();
    CGraphics::SetUseVideoFilter(false);
    GXCopyTex(dest, GX_FALSE);
    CGraphics::SetUseVideoFilter(useVideoFilter);
    GXPixModeSync();

    CGraphics::LoadDolphinSpareTexture(clipRect.GetTexWidth() >> indScale,
                                       clipRect.GetHeight() >> indScale, GX_TF_RGB565, NULL,
                                       CGraphics::kSpareBufferTexMapID);

    uint color = particle->mColor.GetColor_u32();
    CGX::Begin(GX_QUADS, GX_VTXFMT0, 4);

    GXPosition3f32(sinPlusCos + vpX, vpY, cosMinusSin + vpZ);
    GXColor1u32(color);
    GXTexCoord2f32(uvs.xMax, uvs.yMax);
    GXTexCoord2f32(clipRect.GetTexCoord(0).GetX(), clipRect.GetTexCoord(0).GetY());
    GXTexCoord2f32(uvsInd.xMax, uvsInd.yMax);

    GXPosition3f32(sinMinusCos + vpX, vpY, sinPlusCos + vpZ);
    GXColor1u32(color);
    GXTexCoord2f32(uvs.xMin, uvs.yMax);
    GXTexCoord2f32(clipRect.GetTexCoord(1).GetX(), clipRect.GetTexCoord(1).GetY());
    GXTexCoord2f32(uvsInd.xMin, uvsInd.yMax);

    GXPosition3f32(vpX - sinPlusCos, vpY, vpZ - cosMinusSin);
    GXColor1u32(color);
    GXTexCoord2f32(uvs.xMin, uvs.yMin);
    GXTexCoord2f32(clipRect.GetTexCoord(2).GetX(), clipRect.GetTexCoord(2).GetY());
    GXTexCoord2f32(uvsInd.xMin, uvsInd.yMin);

    GXPosition3f32(negSinPlusCos + vpX, vpY, negCosMinusSin + vpZ);
    GXColor1u32(color);
    GXTexCoord2f32(uvs.xMax, uvs.yMin);
    GXTexCoord2f32(clipRect.GetTexCoord(3).GetX(), clipRect.GetTexCoord(3).GetY());
    GXTexCoord2f32(uvsInd.xMax, uvsInd.yMin);

    CGX::End();
#endif
  }

  CGX::SetNumIndStages(0);
  CGX::SetTevDirect(GX_TEVSTAGE1);
#if !RSTL_PRIME1
  if (sMoveRedToAlphaBuffer) {
    GXSetAlphaUpdate(GX_TRUE);
  }
#endif
}

void CElementGen::RenderLines() {
#if RSTL_PRIME1
  bool hasModuColor = mModuColor.GetColor_u32() != 0xFFFFFFFF;
#endif

  CGlobalRandom gr(mRandState);
#if !RSTL_PRIME1
  const bool hasModuColor = mModuColor.GetColor_u32() != 0xffffffff;
#endif

  CTransform4f systemViewPointMatrix(CGraphics::GetViewMatrix());
  systemViewPointMatrix.SetTranslation(CVector3f::Zero());
  CTransform4f systemCameraMatrix(systemViewPointMatrix.GetQuickInverse() * mGlobalOrientation);
  systemViewPointMatrix = CTransform4f::Translate(mGlobalTranslation) * mGlobalScaleTransform *
                          systemViewPointMatrix * mLocalScaleTransform;
  CGraphics::SetModelMatrix(systemViewPointMatrix);

  CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_And, kAF_Always, 0);

  if (mAAPH) {
    CGraphics::SetDepthWriteMode(true, kE_LEqual, false);
    CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_One, kLO_Clear);
  } else {
    CGraphics::SetDepthWriteMode(true, kE_LEqual, true);
    CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
  }

  bool constUVs = true;
  SUVElementSet uvs;
  uvs.xMin = 0.f;
  uvs.xMax = 1.f;
  uvs.yMin = 0.f;
  uvs.yMax = 1.f;

  const bool widtConst = mLoadedGenDesc->mWIDT != nullptr && mLoadedGenDesc->mWIDT->IsConstant();

  if (mLoadedGenDesc->mTEXR != nullptr) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    int partFrame = mCurFrame - mParticles[0].mStartFrame;
    TToken< CTexture > tex = mLoadedGenDesc->mTEXR->GetValueTexture(partFrame);
#else
    TToken< CTexture > tex =
        mLoadedGenDesc->mTEXR->GetValueTexture(mCurFrame - mParticles[0].mStartFrame);
#endif

    tex->Load(GX_TEXMAP0, CTexture::kCM_Repeat);

    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);

    if (hasModuColor) {
      static CTevCombiners::CTevPass ModulatePreviousTEVWithRasColor(
          CTevCombiners::ColorPass(CTevCombiners::kCS_Zero, CTevCombiners::kCS_PreviousColor,
                                   CTevCombiners::kCS_RasterColor, CTevCombiners::kCS_Zero),
          CTevCombiners::AlphaPass(CTevCombiners::kAS_Zero, CTevCombiners::kAS_PreviousAlpha,
                                   CTevCombiners::kAS_RasterAlpha, CTevCombiners::kAS_Zero));
      CGraphics::SetTevOp(kTS_Stage1, ModulatePreviousTEVWithRasColor);
    } else {
      CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
    }

    mLoadedGenDesc->mTEXR->GetValueUV(mCurFrame - mParticles[0].mStartFrame, uvs);
    constUVs = mLoadedGenDesc->mTEXR->HasConstantUV();
  } else {
    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvPassthru);
    CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
  }

  static const GXVtxDescList skDescList[] = {
      {GX_VA_POS, GX_DIRECT},
      {GX_VA_CLR0, GX_DIRECT},
      {GX_VA_TEX0, GX_DIRECT},
      {GX_VA_NULL, GX_NONE},
  };
  CGX::SetVtxDescv(skDescList);

  if (hasModuColor) {
    CGX::SetNumChans(2);
    CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR1A1);
    CGX::SetChanMatColor(CGX::Channel1, mModuColor.GetGXColor());
    CGX::SetChanCtrl(CGX::Channel1, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE,
                     GX_AF_NONE);
  } else {
    CGX::SetNumChans(1);
  }

  CGX::SetNumTexGens(1);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);

  if (widtConst) {
    float lineWidth = 1.f;
    mLoadedGenDesc->mWIDT->GetValue(0, lineWidth);
    if (lineWidth < 0.f) {
      lineWidth = 0.f;
    } else if (lineWidth > 42.5f) {
      lineWidth = 42.5f;
    }
    CGX::SetLineWidth(CCast::ToUint8(6.f * lineWidth), GX_TO_ZERO);
    CGX::Begin(GX_LINES, GX_VTXFMT0, static_cast< ushort >(mParticles.size() * 2));
  }

  for (int i = 0; i < mParticles.size(); ++i) {
    CParticle& particle = mParticles[i];

    if (!constUVs) {
      mLoadedGenDesc->mTEXR->GetValueUV(mCurFrame - particle.mStartFrame, uvs);
    }

    CVector3f direction = particle.mPos - particle.mPrevPos;

    if (mFXLL) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      float magnitude = direction.Magnitude();
#else
      const float magnitude = direction.Magnitude();
#endif
      if (magnitude > 0.f) {
        direction *= 1.f / magnitude;
      }
    }

    const CVector3f p1 = systemCameraMatrix * particle.mPos;
    const CVector3f p2 =
        systemCameraMatrix * (particle.mPos + particle.mLineLengthOrSize * direction);

    if (widtConst) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      uint color = particle.mColor.GetColor_u32();
#else
      const uint color = particle.mColor.GetColor_u32();
#endif

      GXPosition3f32(p1[0], p1[1], p1[2]);
      GXColor1u32(color);
      GXTexCoord2f32(uvs.xMin, uvs.yMin);
      GXPosition3f32(p2[0], p2[1], p2[2]);
      GXColor1u32(color);
      GXTexCoord2f32(uvs.xMax, uvs.yMax);
    } else {
      float lineWidth = particle.mLineWidthOrRota;
      if (lineWidth < 0.f) {
        lineWidth = 0.f;
      } else if (lineWidth > 42.5f) {
        lineWidth = 42.5f;
      }
      CGX::SetLineWidth(CCast::ToUint8(6.f * lineWidth), GX_TO_ZERO);
      CGX::Begin(GX_LINES, GX_VTXFMT0, 2);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      uint color = particle.mColor.GetColor_u32();
#else
      const uint color = particle.mColor.GetColor_u32();
#endif

      GXPosition3f32(p1[0], p1[1], p1[2]);
      GXColor1u32(color);
      GXTexCoord2f32(uvs.xMin, uvs.yMin);
      GXPosition3f32(p2[0], p2[1], p2[2]);
      GXColor1u32(color);
      GXTexCoord2f32(uvs.xMax, uvs.yMax);
      CGX::End();
    }
  }

  if (widtConst) {
    CGX::End();
  }

  CGraphics::SetLineWidth(1.f, kTO_Zero);
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CElementGen::RenderModels() {
  CParticleGlobals::mParticleAccessParameters = NULL;

  if (mModelsUseLights) {
    CGraphics::SetLightState(mBackupLightActive);
  } else {
    CGraphics::SetAmbientColor(CColor::White());
  }

  bool moveRedToAlphaBuffer = false;
  SUVElementSet uvs;
  CGlobalRandom gr(mRandState);
  uvs.xMin = 0.f;
  uvs.xMax = 1.f;
  uvs.yMin = 0.f;
  uvs.yMax = 1.f;

  if (mLoadedGenDesc->mPMUS) {
    bool enableMRTA = false;
    if (sMoveRedToAlphaBuffer && mLoadedGenDesc->mPMAB && mLoadedGenDesc->mTEXR != NULL) {
      enableMRTA = true;
    }
    moveRedToAlphaBuffer = enableMRTA;

    if (mLoadedGenDesc->mPMAB) {
      CGraphics::SetDepthWriteMode(true, kE_LEqual, false);
      if (enableMRTA) {
        CGraphics::SetBlendMode(kBM_Blend, kBF_One, kBF_One, kLO_Clear);
      } else {
        CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_One, kLO_Clear);
        CGraphics::SetAlphaCompare(kAF_Greater, 0, kAO_And, kAF_Always, 0);
      }
    } else {
      CGraphics::SetDepthWriteMode(true, kE_LEqual, true);

      CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
      CGraphics::SetAlphaCompare(kAF_Greater, 0, kAO_And, kAF_Always, 0);
    }

    CGraphics::SetCullMode(kCM_None);

    if (mLoadedGenDesc->mTEXR != NULL) {
      int partFrame = mCurFrame - mParticles[0].mStartFrame;
      TToken< CTexture > tex(mLoadedGenDesc->mTEXR->GetValueTexture(partFrame));
      tex.GetT()->Load(GX_TEXMAP0, CTexture::kCM_Repeat);

      CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
      if (enableMRTA) {
        CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_CPREV, GX_CC_APREV, GX_CC_ZERO);
        CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_TEXA, GX_CA_APREV, GX_CA_ZERO);
        CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE1);
        CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
        GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP1);
        CGX::SetNumTevStages(2);

        static const GXVtxDescList skDescList[] = {
            {GX_VA_POS, GX_DIRECT},
            {GX_VA_CLR0, GX_DIRECT},
            {GX_VA_TEX0, GX_DIRECT},
            {GX_VA_NULL, GX_NONE},
        };
        CGX::SetVtxDescv(skDescList);
        CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                         GX_AF_NONE);
        CGX::SetNumChans(1);
        CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false,
                            GX_PTIDENTITY);
        CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
        CGX::SetNumTexGens(1);
      } else {
        CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
      }
      mLoadedGenDesc->mTEXR->GetValueUV(mCurFrame - mParticles[0].mStartFrame, uvs);
      mLoadedGenDesc->mTEXR->HasConstantUV();
    } else {
      CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvPassthru);
      CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
    }
  }

  CVector3f pmopVec(0.f, 0.f, 0.f);

  CTransform4f orient(CTransform4f::Identity());
  if (!mLoadedGenDesc->mPMOO) {
    orient = mOrientation;
  }
  orient = orient * mGlobalOrientation;

  bool pmrtConst = false;
  if (mLoadedGenDesc->mPMRT != NULL && mLoadedGenDesc->mPMRT->IsFastConstant()) {
    pmrtConst = true;
  }

  CVector3f trans =
      (mGlobalScaleTransformInverse * mLocalScaleTransformInverse) * mGlobalTranslation;

  CTransform4f rot(CTransform4f::Identity());
  if (pmrtConst) {
    CVector3f pmrtVal(0.f, 0.f, 0.f);
    mLoadedGenDesc->mPMRT->GetValue(mCurFrame, pmrtVal);
    rot = CTransform4f::RotateZ(CRelAngle::FromDegrees(pmrtVal.GetZ()));
    rot.RotateLocalY(CRelAngle::FromDegrees(pmrtVal.GetY()));
    rot.RotateLocalX(CRelAngle::FromDegrees(pmrtVal.GetX()));
  }
  rot = orient * rot;

  CParticleGlobals::SetEmitterTime(mCurFrame);
  CColor col = mModuColor;

  for (int i = 0; i < mParticles.size(); ++i) {
    CParticle& particle = mParticles[i];
    if (particle.mEndFrame != -1) {
      int lifetime = particle.mEndFrame - particle.mStartFrame;
      int partFrame = mCurFrame - particle.mStartFrame - 1;
      CParticleGlobals::SetParticleLifetime(lifetime);
      CParticleGlobals::UpdateParticleLifetimeTweenValues(partFrame);
      CParticleGlobals::mCurrentParticle = &particle;

      if (mEnableADV) {
        CParticleGlobals::mParticleAccessParameters = mAdvValues[i].mValues;
      }

      if (mLoadedGenDesc->mPMOP != NULL) {
        mLoadedGenDesc->mPMOP->GetValue(partFrame, pmopVec);
      }
      CTransform4f partTrans = CTransform4f::Translate(
          trans + ((particle.mPos - particle.mPrevPos) * mTimeDeltaScale + particle.mPrevPos));

      if (mOrientType == kMOT_One) {
        CTransform4f partRot(mParentMatrices[i], CVector3f::Zero());
        partTrans.AddTranslation((orient * partRot) * pmopVec);
        partTrans *= partRot;
      } else {
        partTrans.AddTranslation(orient * pmopVec);
      }

      if (pmrtConst) {
        partTrans *= rot;
      } else if (mLoadedGenDesc->mPMRT != NULL) {
        CVector3f pmrtVal(0.f, 0.f, 0.f);
        mLoadedGenDesc->mPMRT->GetValue(partFrame, pmrtVal);
        rot = CTransform4f::RotateZ(CRelAngle::FromDegrees(pmrtVal.GetZ()));
        rot.RotateLocalY(CRelAngle::FromDegrees(pmrtVal.GetY()));
        rot.RotateLocalX(CRelAngle::FromDegrees(pmrtVal.GetX()));
        partTrans *= (orient * rot);
      } else {
        partTrans *= rot;
      }

      if (mLoadedGenDesc->mPMSC != NULL) {
        CVector3f pmscVal(0.f, 0.f, 0.f);
        mLoadedGenDesc->mPMSC->GetValue(partFrame, pmscVal);
        partTrans *= CTransform4f::Scale(pmscVal.GetX(), pmscVal.GetY(), pmscVal.GetZ());
      }

      if (mLoadedGenDesc->mPMCL != NULL) {
        mLoadedGenDesc->mPMCL->GetValue(partFrame, col);
        col = CColor::Modulate(col, mModuColor);
      }

      CGraphics::SetModelMatrix(mGlobalScaleTransform * partTrans * mLocalScaleTransform);

      if (mLoadedGenDesc->mPMUS) {
        if (moveRedToAlphaBuffer) {
          CGX::Begin(GX_QUADS, GX_VTXFMT0, 4);
          GXPosition3f32(0.5f, 0.f, 0.5f);
          u32 c = col.GetColor_u32();
          GXColor1u32(c);

          GXTexCoord2f32(uvs.xMax, uvs.yMax);
          GXPosition3f32(-0.5f, 0.f, 0.5f);
          GXColor1u32(c);

          GXTexCoord2f32(uvs.xMin, uvs.yMax);
          GXPosition3f32(-0.5f, 0.f, -0.5f);
          GXColor1u32(c);

          GXTexCoord2f32(uvs.xMin, uvs.yMin);
          GXPosition3f32(0.5f, 0.f, -0.5f);
          GXColor1u32(c);

          GXTexCoord2f32(uvs.xMax, uvs.yMin);
          CGX::End();
        } else {
          CGraphics::StreamBegin(kP_Quads);
          CGraphics::StreamColor(col.GetColor_u32());

          CGraphics::StreamTexcoord(uvs.xMax, uvs.yMax);
          CGraphics::StreamVertex(0.5f, 0.f, 0.5f);
          CGraphics::StreamTexcoord(uvs.xMin, uvs.yMax);
          CGraphics::StreamVertex(-0.5f, 0.f, 0.5f);
          CGraphics::StreamTexcoord(uvs.xMin, uvs.yMin);
          CGraphics::StreamVertex(-0.5f, 0.f, -0.5f);
          CGraphics::StreamTexcoord(uvs.xMax, uvs.yMin);
          CGraphics::StreamVertex(0.5f, 0.f, -0.5f);
          CGraphics::StreamEnd();
        }
      } else {
        CModel* model = mLoadedGenDesc->mPMDL->GetObject();
        if (sSubtractBlend) {
          model->Draw(CModelFlags::AlphaBlended(0.5f).DepthCompareUpdate(true, false));
        } else if (mLoadedGenDesc->mPMAB) {
          model->Draw(CModelFlags::Additive(col).DepthCompareUpdate(true, false));
        } else if (1.f == col.GetAlpha()) {
          model->Draw(CModelFlags::Normal());
        } else {
          model->Draw(CModelFlags::AlphaBlended(col.GetAlpha()).DepthCompareUpdate(true, false));
        }
      }
    }
  }

  if (mModelsUseLights) {
    CGraphics::DisableAllLights();
  }

  CGraphics::SetCullMode(kCM_Front);
  CTevCombiners::ResetStates();
  if (moveRedToAlphaBuffer) {
    GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
  }
  CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_And, kAF_Always, 0);
}
#else
void CElementGen::BeginModelRender(SModelRenderState& state) {
  if (sMoveRedToAlphaBuffer) {
    GXSetAlphaUpdate(GX_FALSE);
  }

  if (mModelsUseLights) {
    CGraphics::SetLightState(mBackupLightActive);
  } else {
    CGraphics::SetAmbientColor(CColor::White());
  }
  if (!mLoadedGenDesc->mPMUS) {
    return;
  }

  state.mModulateAlpha =
      sEnableAlphaModulation && mLoadedGenDesc->mPMAB && mLoadedGenDesc->mTEXR != nullptr;

  if (mLoadedGenDesc->mPMAB) {
    CGraphics::SetDepthWriteMode(true, kE_LEqual, false);
    if (state.mModulateAlpha) {
      CGraphics::SetBlendMode(kBM_Blend, kBF_One, kBF_One, kLO_Clear);
    } else {
      CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_One, kLO_Clear);
      CGraphics::SetAlphaCompare(kAF_Greater, 0, kAO_And, kAF_Always, 0);
    }
  } else {
    CGraphics::SetDepthWriteMode(true, kE_LEqual, mZBUF);

    CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
    CGraphics::SetAlphaCompare(kAF_Greater, 0, kAO_And, kAF_Always, 0);
  }

  CGraphics::SetCullMode(kCM_None);

  if (mLoadedGenDesc->mTEXR != nullptr) {
    TToken< CTexture > texture =
        mLoadedGenDesc->mTEXR->GetValueTexture(mCurFrame - mParticles[0].mStartFrame);
    texture->Load(GX_TEXMAP0, CTexture::kCM_Repeat);

    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
    if (state.mModulateAlpha) {
      CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_CPREV, GX_CC_APREV, GX_CC_ZERO);
      CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_TEXA, GX_CA_APREV, GX_CA_ZERO);
      CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE1);
      CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
      GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP1);
      CGX::SetNumTevStages(2);

      static const GXVtxDescList skDescList[] = {
          {GX_VA_POS, GX_DIRECT},
          {GX_VA_CLR0, GX_DIRECT},
          {GX_VA_TEX0, GX_DIRECT},
          {GX_VA_NULL, GX_NONE},
      };
      CGX::SetVtxDescv(skDescList);
      CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                       GX_AF_NONE);
      CGX::SetNumChans(1);
      CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false,
                          GX_PTIDENTITY);
      CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
      CGX::SetNumTexGens(1);
    } else {
      CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
    }
    mLoadedGenDesc->mTEXR->GetValueUV(mCurFrame - mParticles[0].mStartFrame, state.mUV);
    state.mConstantUV = mLoadedGenDesc->mTEXR->HasConstantUV();
  } else {
    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvPassthru);
    CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
  }
}

void CElementGen::BeginIndirectModelRender(SModelRenderState& state) {
  if (sMoveRedToAlphaBuffer) {
    GXSetAlphaUpdate(GX_FALSE);
  }
  CGraphics::SetCullMode(kCM_None);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
  if (mAAPH) {
    CGX::SetZMode(true, GX_LEQUAL, false);
    CGX::SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_CLEAR);
  } else {
    CGX::SetZMode(true, GX_LEQUAL, mZBUF);
    CGX::SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
  }

  TToken< CTexture > texture =
      mLoadedGenDesc->mTEXR->GetValueTexture(mCurFrame - mParticles[0].mStartFrame);
  texture->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
  mLoadedGenDesc->mTEXR->GetValueUV(mCurFrame - mParticles[0].mStartFrame, state.mUV);
  state.mConstantUV = mLoadedGenDesc->mTEXR->HasConstantUV();
  TToken< CTexture > indirectTexture =
      mLoadedGenDesc->mTIND->GetValueTexture(mCurFrame - mParticles[0].mStartFrame);
  indirectTexture->Load(GX_TEXMAP2, CTexture::kCM_Repeat);
  state.mConstantIndirectUV = mLoadedGenDesc->mTIND->HasConstantUV();
  mLoadedGenDesc->mTIND->GetValueUV(mCurFrame - mParticles[0].mStartFrame, state.mIndirectUV);

  CGX::SetNumTexGens(3);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX1, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX2, GX_IDENTITY, false, GX_PTIDENTITY);
  float indirectMatrix[2][3] = {{0.5f, 0.f, 0.f}, {0.f, 0.5f, 0.f}};
  GXSetIndTexMtx(GX_ITM_0, indirectMatrix, 1);
  GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD2, GX_TEXMAP2);
  CGX::SetTevIndirect(GX_TEVSTAGE1, GX_INDTEXSTAGE0, GX_ITF_8, GX_ITB_STU, GX_ITM_0, GX_ITW_OFF,
                      GX_ITW_OFF, false, false, GX_ITBA_OFF);
  CGX::SetNumIndStages(1);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
  CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, CGraphics::kSpareBufferTexMapID, GX_COLOR0A0);
  CGX::SetNumTevStages(2);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_8_8);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE1, GX_TEV_KASEL_8_8);
  if (!mLoadedGenDesc->mCIND) {
    CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_ONE, GX_CC_ZERO);
    CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXC, GX_CC_RASC, GX_CC_CPREV);
  } else {
    CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_RASC, GX_CC_ZERO);
    CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXC, GX_CC_CPREV, GX_CC_ZERO);
  }
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_ZERO);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE1);
  CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_APREV, GX_CA_RASA, GX_CA_ZERO);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
  CGX::SetNumChans(1);
  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
  static const GXVtxDescList skDescList[] = {
      {GX_VA_POS, GX_DIRECT},  {GX_VA_CLR0, GX_DIRECT}, {GX_VA_TEX0, GX_DIRECT},
      {GX_VA_TEX1, GX_DIRECT}, {GX_VA_TEX2, GX_DIRECT}, {GX_VA_NULL, GX_NONE},
  };
  CGX::SetVtxDescv(skDescList);
}

void CElementGen::RenderModelParticle(SModelRenderState& state, const CColor& color,
                                      const CParticle& particle) {
  if (mLoadedGenDesc->mPMUS) {
    SUVElementSet& uvs = state.mUV;
    if (!state.mConstantUV) {
      mLoadedGenDesc->mTEXR->GetValueUV(mCurFrame - particle.mStartFrame - 1, uvs);
    }
    if (state.mModulateAlpha) {
      CGX::Begin(GX_QUADS, GX_VTXFMT0, 4);
      GXPosition3f32(0.5f, 0.f, 0.5f);
      const uint packedColor = color.GetColor_u32();
      GXColor1u32(packedColor);

      GXTexCoord2f32(uvs.xMax, uvs.yMax);
      GXPosition3f32(-0.5f, 0.f, 0.5f);
      GXColor1u32(packedColor);

      GXTexCoord2f32(uvs.xMin, uvs.yMax);
      GXPosition3f32(-0.5f, 0.f, -0.5f);
      GXColor1u32(packedColor);

      GXTexCoord2f32(uvs.xMin, uvs.yMin);
      GXPosition3f32(0.5f, 0.f, -0.5f);
      GXColor1u32(packedColor);

      GXTexCoord2f32(uvs.xMax, uvs.yMin);
      CGX::End();
    } else {
      CGraphics::StreamBegin(kP_Quads);
      CGraphics::StreamColor(color.GetColor_u32());

      CGraphics::StreamTexcoord(uvs.xMax, uvs.yMax);
      CGraphics::StreamVertex(0.5f, 0.f, 0.5f);
      CGraphics::StreamTexcoord(uvs.xMin, uvs.yMax);
      CGraphics::StreamVertex(-0.5f, 0.f, 0.5f);
      CGraphics::StreamTexcoord(uvs.xMin, uvs.yMin);
      CGraphics::StreamVertex(-0.5f, 0.f, -0.5f);
      CGraphics::StreamTexcoord(uvs.xMax, uvs.yMin);
      CGraphics::StreamVertex(0.5f, 0.f, -0.5f);
      CGraphics::StreamEnd();
    }
  } else {
    CModel* model = mLoadedGenDesc->mPMDL->GetObject();
    if (sSubtractBlend) {
      model->Draw(CModelFlags::AlphaBlended(0.5f).DepthCompareUpdate(true, false));
    } else if (mLoadedGenDesc->mPMAB) {
      model->Draw(CModelFlags::Additive(color).DepthCompareUpdate(true, false));
    } else if (color.GetAlpha() == 1.f) {
      model->Draw(CModelFlags(
          CModelFlags::kT_One, 0,
          CModelFlags::EFlags(CModelFlags::kF_DepthCompare | CModelFlags::kF_DepthUpdate), color));
    } else {
      model->Draw(CModelFlags(CModelFlags::kT_Blend, 0,
                              CModelFlags::EFlags(CModelFlags::kF_DepthCompare |
                                                  CModelFlags::kF_DepthUpdate),
                              color)
                      .DepthCompareUpdate(true, false));
    }
  }
}

void CElementGen::RenderIndirectModelParticle(SModelRenderState& state, const CColor& color,
                                              const CParticle& particle) {
  SUVElementSet& uvs = state.mUV;
  if (!state.mConstantUV) {
    mLoadedGenDesc->mTEXR->GetValueUV(mCurFrame - particle.mStartFrame, uvs);
  }
  SUVElementSet& indirectUVs = state.mIndirectUV;
  if (!state.mConstantIndirectUV) {
    mLoadedGenDesc->mTIND->GetValueUV(mCurFrame - particle.mStartFrame, indirectUVs);
  }

  CGraphics::CClippedScreenQuad clip = CGraphics::ClipScreenQuadFromMS(
      CVector3f(0.5f, 0.f, 0.5f), CVector3f(-0.5f, 0.f, 0.5f), CVector3f(-0.5f, 0.f, -0.5f),
      CVector3f(0.5f, 0.f, -0.5f), kTF_RGB565);
  if (!clip.IsValid()) {
    return;
  }
  const int halfSize = mLoadedGenDesc->mINDM ? 1 : 0;
  void* const dest = CGraphics::GetDolphinSpareBuffer();
  GXSetTexCopySrc(clip.GetX(), clip.GetY(), clip.GetWidth(), clip.GetHeight());
  GXSetTexCopyDst(clip.GetTexWidth() >> halfSize, clip.GetHeight() >> halfSize, GX_TF_RGB565,
                  halfSize != 0 ? GX_TRUE : GX_FALSE);
  int bufferSize = CGraphics::GetSpareBufferSize();
  if (GXGetTexBufferSize(clip.GetTexWidth() >> halfSize, clip.GetHeight() >> halfSize, GX_TF_RGB565,
                         GX_FALSE, 0) > bufferSize) {
    return;
  }
  bool useVideoFilter = CGraphics::GetUseVideoFilter();
  CGraphics::SetUseVideoFilter(false);
  GXCopyTex(dest, GX_FALSE);
  CGraphics::SetUseVideoFilter(useVideoFilter);
  GXPixModeSync();
  CGraphics::LoadDolphinSpareTexture(clip.GetTexWidth() >> halfSize, clip.GetHeight() >> halfSize,
                                     GX_TF_RGB565, nullptr, CGraphics::kSpareBufferTexMapID);

  const uint packedColor = color.GetColor_u32();
  CGX::Begin(GX_QUADS, GX_VTXFMT0, 4);
  GXPosition3f32(0.5f, 0.f, 0.5f);
  GXColor1u32(packedColor);
  GXTexCoord2f32(uvs.xMax, uvs.yMax);
  GXTexCoord2f32(clip.GetTexCoord(0).GetX(), clip.GetTexCoord(0).GetY());
  GXTexCoord2f32(indirectUVs.xMax, indirectUVs.yMax);
  GXPosition3f32(-0.5f, 0.f, 0.5f);
  GXColor1u32(packedColor);
  GXTexCoord2f32(uvs.xMin, uvs.yMax);
  GXTexCoord2f32(clip.GetTexCoord(1).GetX(), clip.GetTexCoord(1).GetY());
  GXTexCoord2f32(indirectUVs.xMin, indirectUVs.yMax);
  GXPosition3f32(-0.5f, 0.f, -0.5f);
  GXColor1u32(packedColor);
  GXTexCoord2f32(uvs.xMin, uvs.yMin);
  GXTexCoord2f32(clip.GetTexCoord(2).GetX(), clip.GetTexCoord(2).GetY());
  GXTexCoord2f32(indirectUVs.xMin, indirectUVs.yMin);
  GXPosition3f32(0.5f, 0.f, -0.5f);
  GXColor1u32(packedColor);
  GXTexCoord2f32(uvs.xMax, uvs.yMin);
  GXTexCoord2f32(clip.GetTexCoord(3).GetX(), clip.GetTexCoord(3).GetY());
  GXTexCoord2f32(indirectUVs.xMax, indirectUVs.yMin);
  CGX::End();
}

void CElementGen::EndModelRender(const SModelRenderState& state) const {
  if (mModelsUseLights) {
    CGraphics::DisableAllLights();
  }

  CGraphics::SetCullMode(kCM_Front);
  CTevCombiners::ResetStates();
  if (state.mModulateAlpha) {
    GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
  }
  CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_And, kAF_Always, 0);
  if (sMoveRedToAlphaBuffer) {
    GXSetAlphaUpdate(GX_TRUE);
  }
}

void CElementGen::EndIndirectModelRender(const SModelRenderState& state) {
  CGraphics::SetCullMode(kCM_Front);
  CGX::SetNumIndStages(0);
  CGX::SetTevDirect(GX_TEVSTAGE1);
  if (sMoveRedToAlphaBuffer) {
    GXSetAlphaUpdate(GX_TRUE);
  }
}

void CElementGen::RenderModels() {
  CGlobalRandom gr(mRandState);
  CParticleGlobals::SetParticleAccessParameters(nullptr);
  SModelRenderState state;
  if (!IsIndirectTextured()) {
    BeginModelRender(state);
  } else {
    if (!mLoadedGenDesc->mPMUS) {
      return;
    }
    BeginIndirectModelRender(state);
  }

  CVector3f offset(CVector3f::Zero());
  CTransform4f orientation(CTransform4f::Identity());
  if (!mLoadedGenDesc->mPMOO) {
    orientation = mOrientation;
  }
  orientation = orientation * mGlobalOrientation;
  bool constantRotation =
      mLoadedGenDesc->mPMRT != nullptr && mLoadedGenDesc->mPMRT->IsFastConstant();
  CVector3f translation =
      (mGlobalScaleTransformInverse * mLocalScaleTransformInverse) * mGlobalTranslation;
  CTransform4f rotation(CTransform4f::Identity());
  if (constantRotation) {
    CVector3f angles(CVector3f::Zero());
    mLoadedGenDesc->mPMRT->GetValue(mCurFrame, angles);
    rotation = CTransform4f::RotateZ(CRelAngle::FromDegrees(angles.GetZ()));
    rotation.RotateLocalY(CRelAngle::FromDegrees(angles.GetY()));
    rotation.RotateLocalX(CRelAngle::FromDegrees(angles.GetX()));
  }
  rotation = orientation * rotation;
  CParticleGlobals::SetEmitterTime(mCurFrame);

  CParticleListItem* sortItems = nullptr;
  CColor color = mModuColor;
  bool sorted = mLoadedGenDesc->mSORT;
  if (sorted) {
    CTransform4f view(CGraphics::GetViewMatrix());
    view.SetTranslation(CVector3f::Zero());
    CTransform4f camera(view.GetQuickInverse() * mGlobalOrientation);
    int count = mParticles.size();
    sortItems = static_cast< CParticleListItem* >(alloca(count * sizeof(CParticleListItem)));
    for (int i = 0; i < count; ++i) {
      CParticle& particle = mParticles[i];
      sortItems[i].mViewPoint =
          camera * (mTimeDeltaScale * (particle.mPos - particle.mPrevPos) + particle.mPrevPos);
      sortItems[i].mPartIdx = static_cast< ushort >(i);
    }
    static CParticleListItemViewPointComp comparator;
    rstl::sort(sortItems, sortItems + count, comparator);
  }

  for (int i = 0; i < mParticles.size(); ++i) {
    const int index = sorted ? sortItems[i].mPartIdx : i;
    CParticle& particle = mParticles[index];
    if (particle.mEndFrame == -1) {
      continue;
    }
    int frame = mCurFrame - particle.mStartFrame - 1;
    CParticleGlobals::SetParticleLifetime(particle.mEndFrame - particle.mStartFrame);
    CParticleGlobals::UpdateParticleLifetimeTweenValues(frame);
    CParticleGlobals::SetCurrentParticle(&particle);
    if (mEnableADV) {
      CParticleGlobals::SetParticleAccessParameters(mAdvValues[index].mValues);
    }
    if (mLoadedGenDesc->mPMOP != nullptr) {
      mLoadedGenDesc->mPMOP->GetValue(frame, offset);
    }

    CTransform4f transform = CTransform4f::Translate(
        translation +
        mGlobalOrientation.Rotate(mTimeDeltaScale * (particle.mPos - particle.mPrevPos) +
                                  particle.mPrevPos));
    if (mOrientType == kMOT_One) {
      CTransform4f parent(mParentMatrices[index], CVector3f::Zero());
      transform.AddTranslation((orientation * parent) * offset);
      transform *= parent;
    } else {
      transform.AddTranslation(orientation * offset);
    }
    if (mLoadedGenDesc->mPMOV != nullptr) {
      CVector3f direction(CVector3f::Zero());
      mLoadedGenDesc->mPMOV->GetValue(frame, direction);
      transform *= CTransform4f::LookAt(CVector3f::Zero(), direction, CVector3f::Up());
    }
    if (constantRotation) {
      transform *= rotation;
    } else if (mLoadedGenDesc->mPMRT != nullptr) {
      CVector3f angles(CVector3f::Zero());
      mLoadedGenDesc->mPMRT->GetValue(frame, angles);
      rotation = CTransform4f::RotateZ(CRelAngle::FromDegrees(angles.GetZ()));
      rotation.RotateLocalY(CRelAngle::FromDegrees(angles.GetY()));
      rotation.RotateLocalX(CRelAngle::FromDegrees(angles.GetX()));
      transform *= orientation * rotation;
    } else {
      transform *= rotation;
    }
    if (mLoadedGenDesc->mPMSC != nullptr) {
      CVector3f scale(CVector3f::Zero());
      mLoadedGenDesc->mPMSC->GetValue(frame, scale);
      transform *= CTransform4f::Scale(scale.GetX(), scale.GetY(), scale.GetZ());
    }
    if (mLoadedGenDesc->mPMCL != nullptr) {
      mLoadedGenDesc->mPMCL->GetValue(frame, color);
      color = CColor::Modulate(color, mModuColor);
    }
    CGraphics::SetModelMatrix(mGlobalScaleTransform * transform * mLocalScaleTransform);
    if (!IsIndirectTextured()) {
      RenderModelParticle(state, color, particle);
    } else {
      RenderIndirectModelParticle(state, color, particle);
    }
  }

  if (!IsIndirectTextured()) {
    EndModelRender(state);
  } else {
    EndIndirectModelRender(state);
  }
}
#endif

int CElementGen::GetParticleCountAll() const { return mRecursiveParticleCount; }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
int CElementGen::GetParticleCountAllInternal() const {
  int ret = mActiveParticleCount;
  for (AUTO(it, mActivePartChildren.begin()); it != mActivePartChildren.end(); ++it) {
    CParticleGen* child = *it;
    if (child->Get4CharId() == 'PART') {
      ret += static_cast< CElementGen* >(child)->GetParticleCountAll();
    }
  }
  return ret;
}
#else
int CElementGen::GetParticleCountAllInternal() const {
  int count = mActiveParticleCount;
  for (rstl::vector< CParticleGen* >::const_iterator it = mActivePartChildren.begin();
       it != mActivePartChildren.end(); ++it) {
    CParticleGen* child = *it;
    count += child->Get4CharId() == 'PART'
                 ? static_cast< CElementGen* >(child)->GetParticleCountAll()
                 : child->GetParticleCount();
  }
  return count;
}
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
int CElementGen::GetSystemCount() {
  int ret = static_cast< int >(mActiveParticleCount) > 0;
  for (AUTO(it, mActivePartChildren.begin()); it != mActivePartChildren.end(); ++it) {
    CParticleGen* child = *it;
    if (child->Get4CharId() == 'PART') {
      ret += static_cast< CElementGen* >(child)->GetSystemCount();
    } else {
      ret += 1;
    }
  }
  return ret;
}
#else
int CElementGen::GetSystemCount() const {
  int count = static_cast< int >(mActiveParticleCount) > 0;
  for (rstl::vector< CParticleGen* >::const_iterator it = mActivePartChildren.begin(),
                                                     end = mActivePartChildren.end();
       it != end; ++it) {
    count += (*it)->GetSystemCount();
  }
  return count;
}
#endif

void CElementGen::AccumulateBounds(const CVector3f& pos, float size) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  float x = pos.GetX();
  float y = pos.GetY();
  float z = pos.GetZ();
#else
  const float x = pos.GetX();
  const float y = pos.GetY();
  const float z = pos.GetZ();
#endif
  if (x > mAabbMax.GetX()) {
    mAabbMax.SetX(x);
  }
  if (x < mAabbMin.GetX()) {
    mAabbMin.SetX(x);
  }
  if (y > mAabbMax.GetY()) {
    mAabbMax.SetY(y);
  }
  if (y < mAabbMin.GetY()) {
    mAabbMin.SetY(y);
  }
  if (z > mAabbMax.GetZ()) {
    mAabbMax.SetZ(z);
  }
  if (z < mAabbMin.GetZ()) {
    mAabbMin.SetZ(z);
  }
  if (size > mMaxSize) {
    mMaxSize = size;
  }
}

void CElementGen::BuildParticleSystemBounds() {
  CAABox childBounds = CAABox::MakeMaxInvertedBox();
  bool accumulated = false;
#if !RSTL_PRIME1
  if (mEnableDynamicBounds)
#endif
  {
    for (AUTO(it, mActivePartChildren.begin()); it != mActivePartChildren.end(); ++it) {
      rstl::optional_object< CAABox > bounds = (*it)->GetBounds();
      if (bounds) {
        accumulated = true;
        const CAABox& box = *bounds;
        childBounds.AccumulateBounds(box.GetMinPoint());
        childBounds.AccumulateBounds(box.GetMaxPoint());
      }
    }
  }

  mRecursiveParticleCount = GetParticleCountAllInternal();

  if (
#if RSTL_PRIME1
      GetParticleCount() > 0
#else
      (mEnableDynamicBounds && GetParticleCount() > 0) || mEnableFixedBounds
#endif
  ) {
#if !RSTL_PRIME1
    const float halfSize = mMaxSize / 2.f;
#endif
    const CVector3f extent = CVector3f::ByElementMultiply(
#if RSTL_PRIME1
        CVector3f(mMaxSize, mMaxSize, mMaxSize),
#else
        CVector3f(halfSize, halfSize, halfSize),
#endif
        mGlobalScale);
#if RSTL_PRIME1
    const
#endif
        CAABox transformed = CAABox(mAabbMin, mAabbMax)
            .GetTransformedAABox((mGlobalScaleTransform * mGlobalOrientation) *
                                 mLocalScaleTransform);
    const CAABox box = transformed;
    mSystemBounds = CAABox(box.GetMinPoint() + mGlobalTranslation - extent,
                           box.GetMaxPoint() + mGlobalTranslation + extent);
  } else {
    mSystemBounds = CAABox::MakeMaxInvertedBox();
  }

  if (accumulated) {
    mSystemBounds.AccumulateBounds(childBounds.GetMinPoint());
    mSystemBounds.AccumulateBounds(childBounds.GetMaxPoint());
  }
}

rstl::optional_object< CAABox > CElementGen::GetBounds() const {
  if (GetParticleCountAll() <= 0) {
    return rstl::optional_object< CAABox >();
  }
  return rstl::optional_object< CAABox >(mSystemBounds);
}

#if RSTL_PRIME1
int CElementGen::GetEmitterTime() const { return mCurFrame; }
#endif

bool CElementGen::SystemHasLight() const { return mLightType != kLT_None; }

CLight CElementGen::GetLight() const {
  switch (mLightType) {
  case kLT_Directional: {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    float r = mLCLR.GetRed();
    float g = mLCLR.GetGreen();
    float b = mLCLR.GetBlue();
    float a = mLCLR.GetAlpha();
    return CLight::BuildDirectional(mLDIR.AsNormalized(),
                                    CColor(mLINT * r, mLINT * g, mLINT * b, mLINT * a));
#else
    const CColor color(
        rstl::min_val(1.f, mLINT * mLCLR.GetRed()), rstl::min_val(1.f, mLINT * mLCLR.GetGreen()),
        rstl::min_val(1.f, mLINT * mLCLR.GetBlue()), rstl::min_val(1.f, mLINT * mLCLR.GetAlpha()));
    return CLight::BuildDirectional(mLDIR.AsNormalized(), color);
#endif
  }
  case kLT_Spot: {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    float r = mLCLR.GetRed();
    float g = mLCLR.GetGreen();
    float b = mLCLR.GetBlue();
    float a = mLCLR.GetAlpha();
    CLight spotLight = CLight::BuildSpot(mLOFF, mLDIR.AsNormalized(),
                                         CColor(mLINT * r, mLINT * g, mLINT * b, mLINT * a), mLSLA);
    float quad = mFalloffType == kFT_Quadratic ? mLFOR : 0.f;
    float linear = mFalloffType == kFT_Linear ? mLFOR : 0.f;
    float constant = mFalloffType == kFT_Constant ? 1.f : 0.f;
    spotLight.SetAttenuation(constant, linear, quad);
    return spotLight;
#else
    const CColor color(
        rstl::min_val(1.f, mLINT * mLCLR.GetRed()), rstl::min_val(1.f, mLINT * mLCLR.GetGreen()),
        rstl::min_val(1.f, mLINT * mLCLR.GetBlue()), rstl::min_val(1.f, mLINT * mLCLR.GetAlpha()));
    CLight light = CLight::BuildSpot(mLOFF, mLDIR.AsNormalized(), color, mLSLA);
    const float quadratic = mFalloffType == kFT_Quadratic ? mLFOR : 0.f;
    const float linear = mFalloffType == kFT_Linear ? mLFOR : 0.f;
    const float constant = mFalloffType == kFT_Constant ? 1.f : 0.f;
    light.SetAttenuation(constant, linear, quadratic);
    return light;
#endif
  }
  default: {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    float quad = mFalloffType == kFT_Quadratic ? mLFOR : 0.f;
    float linear = mFalloffType == kFT_Linear ? mLFOR : 0.f;
    float constant = mFalloffType == kFT_Constant ? 1.f : 0.f;
    return CLight::BuildCustom(mLOFF, CVector3f(1.f, 0.f, 0.f), mLCLR, constant, linear, quad,
                               mLINT, 0.f, 0.f);
#else
    const float quadratic = mFalloffType == kFT_Quadratic ? mLFOR : 0.f;
    const float linear = mFalloffType == kFT_Linear ? mLFOR : 0.f;
    const float constant = mFalloffType == kFT_Constant ? 1.f : 0.f;
    return CLight::BuildCustom(mLOFF, CVector3f::Right(), mLCLR, constant, linear, quadratic, mLINT,
                               0.f, 0.f);
#endif
  }
  }
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
// Preserve the retained zero literal before GetLight; this helper is stripped.
static float StrippedParticleZero() { return 0.f; }
#else
float CElementGen::GetGenerationRate() {
  float rate = 0.f;
  if (mLoadedGenDesc->mGRTE->GetValue(mCurFrame, rate)) {
    return 0.f;
  } else {
    rate = rstl::max_val(0.f, rate * mGeneratorRate);
    return rate;
  }
}
#endif

void CElementGen::SetGeneratorRate(float rate) {
  mGeneratorRate = rstl::max_val(rate, 0.f);
  for (AUTO(it, mActivePartChildren.begin()); it != mActivePartChildren.end(); ++it) {
    CParticleGen* child = *it;
    if (child->Get4CharId() == 'PART') {
      child->SetGeneratorRate(mGeneratorRate);
    }
  }
}
float CElementGen::GetGeneratorRate() const { return mGeneratorRate; }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
uint CElementGen::Get4CharId() const { return 'PART'; }
#else
int CElementGen::GetNumSpawnedParticleSystems() const { return mActivePartChildren.size(); }
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
int CElementGen::GetNumActiveChildParticles() const { return mActivePartChildren.size(); }
#else
CParticleGen* CElementGen::SpawnedParticleSystem(int index) { return mActivePartChildren[index]; }
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CParticleGen* CElementGen::GetActiveChildParticle(int index) const {
  return mActivePartChildren[index];
}

void CElementGen::SetExternalVar(int index, float val) { mExternalVars[index] = val; }
#else
void CElementGen::SetExternalParam(uint index, float value) { mExternalVars[index] = value; }
#endif

float CElementGen::GetExternalVar(int index) const { return mExternalVars[index]; }

bool CElementGen::IsIndirectTextured() const {
  return mLoadedGenDesc->mTEXR != nullptr && mLoadedGenDesc->mTIND != nullptr;
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
bool CElementGen::GetParticleEmission() const { return mParticleEmission; }
#else
CElementGen::CAdvancedValues* CElementGen::ParticleAdditionalData(uint index) {
  return mEnableADV ? &mAdvValues[index] : nullptr;
}
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
const CTransform4f& CElementGen::GetGlobalOrientation() const { return mGlobalOrientation; }

const CVector3f& CElementGen::GetGlobalTranslation() const { return mGlobalTranslation; }
#else
const CElementGen::CAdvancedValues* CElementGen::GetParticleAdditionalData(uint index) const {
  return mEnableADV ? &mAdvValues[index] : nullptr;
}
#endif

