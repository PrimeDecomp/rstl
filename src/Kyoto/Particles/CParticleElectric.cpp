#include "rstl/RstlVersions.h"
#include "rstl/list.hpp" // IWYU pragma: keep
#include "Kyoto/Particles/CParticleElectric.hpp" // IWYU pragma: keep

#include "Kyoto/Basics/CStopwatch.hpp"

#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CLight.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CMath.hpp"

#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Particles/CElectricDescription.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CParticleGlobals.hpp"
#include "Kyoto/Particles/CParticleSwoosh.hpp"
#include "Kyoto/Particles/IElement.hpp"

#include "dolphin/gx/GXVert.h"
#include "float.h"
#include "math.h"
#include "rstl/math.hpp"

#if !RSTL_PRIME1
static CVector3f sOneVector(1.f, 1.f, 1.f);
#endif

ushort CParticleElectric::sSeed = 99;

CParticleElectric::CParticleElectric(TToken< CElectricDescription > desc)
: mElecDesc(desc)
, mCurrentFrame(0)
, mLIFE(0)
, mCurTime(0.0)
, mTranslation(CVector3f::Zero())
, mOrientation(CTransform4f::Identity())
, mInvOrientation(CTransform4f::Identity())
, mGlobalTranslation(CVector3f::Zero())
, mGlobalOrientation(CTransform4f::Identity())
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
, mGlobalScale(1.f, 1.f, 1.f)
, mLocalScale(1.f, 1.f, 1.f)
#else
, mGlobalScale(sOneVector)
, mLocalScale(sOneVector)
#endif

, mCachedXf(CTransform4f::Identity())
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
, x128_(0.f)
#else
, x130_(0.f)
#endif

, mRenderTime(0.f)
, mMinBounds(CVector3f::Zero())
, mMaxBounds(CVector3f::Zero())
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
, x148_(0.f)
#else
, x150_(0.f)
#endif

, mRandState(sSeed++)
, mSSEG(8)
, mSCNT(1)
#if RSTL_PRIME1
, x158_(0)
#endif

, mGenRem(0.f)
#if !RSTL_PRIME1
, mGeneratorRate(1.f)
, mCumulativeParticles(0)
#endif

, mSystemBounds(CAABox::MakeMaxInvertedBox())
, mModuColor(CColor::White())
, mAllocated(false)
, mEmitting(true)
, mHaveGPSM(false)
, mHaveEPSM(false)
, mHaveSSWH(false)
, mHaveLWD(false)
, mTransformDirty(true)
#if !RSTL_PRIME1
, mTranslationDirty(false)
, mOrientationDirty(false)
, mGlobalScaleDirty(false)
#endif
{
#if !RSTL_PRIME1
  SetDrawFlags(mElecDesc->mDFLG);
#endif

  if (mElecDesc->mSSEG) {
    mElecDesc->mSSEG->GetValue(mCurrentFrame, mSSEG);
  }
  if (mElecDesc->mSCNT) {
    mElecDesc->mSCNT->GetValue(mCurrentFrame, mSCNT);
  }
  if (mSCNT > 32) {
    mSCNT = 32;
  }
  if (mElecDesc->mLIFE) {
    mElecDesc->mLIFE->GetValue(0, mLIFE);
  } else {
    mLIFE = 0x7fffff;
  }
  if (mElecDesc->mSSWH) {
    mHaveSSWH = true;
    for (int i = 0; i < mSCNT; ++i) {
      mSwooshGenerators.push_back(rs_new CParticleSwoosh(mElecDesc->mSSWH->GetToken(), mSSEG));
      CParticleSwoosh& swoosh = *mSwooshGenerators.back();
      const int count = swoosh.GetSwooshCount();
      for (int j = 0; j < count; ++j) {
        swoosh.SetWarmUp();
        swoosh.Update(0.0);
      }
    }
  }
  ++mSSEG;
  mCalculatedVerts = rstl::vector< CVector3f, rstl::aligned_allocator >(
      mSSEG, CVector3f::Zero(), rstl::aligned_allocator());
  mFractalOffsets = rstl::vector< CVector3f >(mSSEG, CVector3f::Zero(), rstl::rmemory_allocator());
  mFractalMags = rstl::vector< float >(mSSEG, 0.f, rstl::rmemory_allocator());
  if (mElecDesc->mGPSM) {
    mHaveGPSM = true;
    mGPSMGenerators.reserve(mSCNT);
    for (int i = 0; i < mSCNT; ++i) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      mGPSMGenerators.push_back(rs_new CElementGen(mElecDesc->mGPSM->GetToken()));
#else
      mGPSMGenerators.push_back_unsafe(rs_new CElementGen(mElecDesc->mGPSM->GetToken()));
#endif
      mGPSMGenerators.back()->SetParticleEmission(false);
    }
  }
  if (mElecDesc->mEPSM) {
    mHaveEPSM = true;
    mEPSMGenerators.reserve(mSCNT);
    for (int i = 0; i < mSCNT; ++i) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      mEPSMGenerators.push_back(rs_new CElementGen(mElecDesc->mEPSM->GetToken()));
#else
      mEPSMGenerators.push_back_unsafe(rs_new CElementGen(mElecDesc->mEPSM->GetToken()));
#endif
      mEPSMGenerators.back()->SetParticleEmission(false);
    }
  }
  if (mElecDesc->mLWD1 || mElecDesc->mLWD2 || mElecDesc->mLWD3) {
    mHaveLWD = true;
    for (int i = 0; i < mSCNT; ++i) {
      mLineManagers.push_back(rs_new CLineManager);
    }
  }
}

CParticleElectric::~CParticleElectric() {}

void CParticleElectric::CalculatePoints() {
  rstl::reserved_vector< CVector3f, 4 > points;
  CVector3f pos = CVector3f::Zero();
  CVector3f vel = CVector3f::Zero();
#if !RSTL_PRIME1
  CParticleElectricManager& manager = mElectricManagers.back();
#endif

  if (mElecDesc->mIEMT) {
    mElecDesc->mIEMT->GetValue(mCurrentFrame, pos, vel);
  }
  if (mOverrideIPos) {
    pos = *mOverrideIPos;
  }
  if (mOverrideIVel) {
    vel = *mOverrideIVel;
  }
#if !RSTL_PRIME1
  manager.mInitialPos = pos;
  manager.mInitialVel = vel;
#endif

  if (vel.IsNonZero()) {
    points.push_back(pos);
    points.push_back(pos + vel);
    points.push_back(pos + vel * 2.f);
  } else {
    points.push_back(pos);
  }
  CVector3f fpos(0.f, 1.f, 0.f);
  CVector3f fvel = CVector3f::Zero();
  if (mElecDesc->mFEMT) {
    mElecDesc->mFEMT->GetValue(mCurrentFrame, fpos, fvel);
  }
  if (mOverrideFPos) {
    fpos = *mOverrideFPos;
  }
  if (mOverrideFVel) {
    fvel = *mOverrideFVel;
  }
#if !RSTL_PRIME1
  manager.mFinalPos = fpos;
  manager.mFinalVel = fvel;
#endif

  if (fvel.IsNonZero()) {
    if (points.size() == 3) {
      points[2] = fpos + fvel;
      points.push_back(fpos);
    } else {
      points.push_back(fpos + fvel * 2.f);
      points.push_back(fpos + fvel);
      points.push_back(fpos);
    }
  } else {
    points.push_back(fpos);
  }
  if (points.size() == 4) {
    const int segments = mSSEG - 1;
    const CVector3f& p0 = points[0];
    const CVector3f& p1 = points[1];
    const CVector3f& p2 = points[2];
    const float segDiv = 1.f / static_cast< float >(mSSEG - 1);
    mCalculatedVerts[0] = p0;
    float curDiv = segDiv;
    for (int i = 1; i < segments; ++i) {
      const float t = segDiv * mRandState.Range(-0.45f, 0.45f) + curDiv;
      const CVector3f point = CMath::GetBezierPoint(p0, p1, p2, points[3], t);
      mCalculatedVerts[i] = point;
      curDiv += segDiv;
    }
    mCalculatedVerts[segments] = points[3];
  } else {
    int i;
    mCalculatedVerts[0] = pos;
    const int segments = mSSEG - 1;
    const float segDiv = 1.f / static_cast< float >(segments);
#if !RSTL_PRIME1
    const CVector3f segDelta = (fpos - pos) * segDiv;
#endif

    CVector3f accum = mCalculatedVerts[0];
#if RSTL_PRIME1
    const CVector3f segDelta = (fpos - pos) * segDiv;
#endif

    for (i = 1; i < segments; ++i) {
      const float r = mRandState.Range(-0.45f, 0.45f);
      mCalculatedVerts[i] = accum + segDelta * r;
      accum += segDelta;
    }
    mCalculatedVerts[segments] = fpos;
  }
  for (int i = 0; i < mSSEG; ++i) {
    mFractalMags[i] = 0.f;
  }
  float ampl = 1.f;
  if (mElecDesc->mAMPL) {
    mElecDesc->mAMPL->GetValue(mCurrentFrame, ampl);
    ampl *= 2.f;
  }
  float ampd = 0.f;
  if (mElecDesc->mAMPD) {
    mElecDesc->mAMPD->GetValue(mCurrentFrame, ampd);
  }
  CalculateFractal(0, mCalculatedVerts.size() - 1, ampl, ampd);
  CVector3f v0 = mCalculatedVerts[0] - mCalculatedVerts[1];
  CVector3f v1 = mCalculatedVerts[mCalculatedVerts.size() - 1] - mCalculatedVerts[1];
  CVector3f up = CVector3f::Up();
  if (v0.CanBeNormalized() && v1.CanBeNormalized()) {
    v0.Normalize();
    v1.Normalize();
    float dot = CVector3f::Dot(v0, v1);
    dot = dot < 0.f ? -dot : dot;
    if (close_enough(dot, 1.f)) {
      up = CTransform4f::LookAt(mCalculatedVerts[0], mCalculatedVerts[1]).GetUp();
    } else {
      up = CVector3f::Cross(v0, v1).AsNormalized();
    }
  } else if (!(mCalculatedVerts[0] == mCalculatedVerts[1])) {
    up = CTransform4f::LookAt(mCalculatedVerts[0], mCalculatedVerts[1]).GetUp();
  }
  const float commonRand = mRandState.Range(0.f, 360.f);
  for (int i = 1; i < mCalculatedVerts.size() - 1; ++i) {
    const CVector3f delta = mCalculatedVerts[i] - mCalculatedVerts[i - 1];
    if (delta.IsNonZero()) {
      const float randomAngle = mFractalMags[i] / ampl * 16.f * mRandState.Range(-1.f, 1.f);
      const CRelAngle angle = CRelAngle::FromDegrees(randomAngle + commonRand);
      const CQuaternion rot = CQuaternion::AxisAngle(delta, angle);
      mFractalOffsets[i] = rot.Transform(mFractalMags[i] * up);
    }
  }
  for (int i = 1; i < mCalculatedVerts.size() - 1; ++i) {
    mCalculatedVerts[i] += mFractalOffsets[i];
  }
  if (mElecDesc->mZERY) {
    for (int i = 0; i < mCalculatedVerts.size(); ++i) {
      mCalculatedVerts[i].SetY(0.f);
    }
  }
}

void CParticleElectric::CalculateFractal(int start, int end, float ampl, float ampd) {
  float mag;
  const float ratio = static_cast< float >(end - start) / static_cast< float >(mFractalMags.size());
  const float startMag = mFractalMags[start];
  const int mid = (start + end) / 2;
  mag = ratio * ampl;
  const float endMag = mFractalMags[end];
  const float base = (startMag + endMag) / 2.f + mag * mRandState.Float() - mag / 2.f;
  mFractalMags[mid] = base + (ampd * mRandState.Float() - ampd / 2.f);
  if ((start + end) % 2 == 1) {
    mFractalMags[end - 1] = mFractalMags[end];
  }
  if (mid - start > 1) {
    CalculateFractal(start, mid, ampl, ampd);
  }
  if (end - mid > 1) {
    CalculateFractal(mid, end, ampl, ampd);
  }
}

const bool CParticleElectric::Update(double dt) {
  CGlobalRandom random(mRandState);
  bool updated = false;
  if (mHaveGPSM) {
    for (int i = 0; i < mSCNT; ++i) {
      if (!mGPSMGenerators[i]->IsSystemDeletable()) {
        break;
      }
    }
  }
  if (mHaveEPSM) {
    for (int i = 0; i < mSCNT; ++i) {
      if (!mEPSMGenerators[i]->IsSystemDeletable()) {
        break;
      }
    }
  }
  const bool emitting = mEmitting && mCurrentFrame < mLIFE;
  double evalTime = mCurrentFrame * (1.0 / 60.0);
  mCurTime += dt;
  if (mTransformDirty) {
    UpdateCachedTransform();
#if !RSTL_PRIME1
  }
  if (mTranslationDirty || mOrientationDirty || mGlobalScaleDirty || mLocalScaleDirty) {
#endif

    const CTransform4f orientation = mCachedXf.GetRotation();
    const CVector3f translation = mCachedXf.GetTranslation();
    if (mHaveSSWH) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      for (AUTO(it, mElectricManagers.begin()); it != mElectricManagers.end(); ++it) {
        mSwooshGenerators[it->mIdx]->SetGlobalTranslation(translation);
        mSwooshGenerators[it->mIdx]->SetGlobalOrientation(orientation);
        mSwooshGenerators[it->mIdx]->SetGlobalScale(mGlobalScale);
        mSwooshGenerators[it->mIdx]->SetLocalScale(mLocalScale);
      }
#else
      for (rstl::list< CParticleElectricManager >::iterator it = mElectricManagers.begin();
           it != mElectricManagers.end(); ++it) {
        if (mTranslationDirty) {
          mSwooshGenerators[it->mIdx]->SetGlobalTranslation(translation);
        }
        if (mOrientationDirty) {
          mSwooshGenerators[it->mIdx]->SetGlobalOrientation(orientation);
        }
        if (mGlobalScaleDirty) {
          mSwooshGenerators[it->mIdx]->SetGlobalScale(mGlobalScale);
        }
        if (mLocalScaleDirty) {
          mSwooshGenerators[it->mIdx]->SetLocalScale(mLocalScale);
        }
      }
#endif
    }
    if (mHaveGPSM) {
      for (int i = 0; i < mSCNT; ++i) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
        mGPSMGenerators[i]->SetGlobalTranslation(translation);
        mGPSMGenerators[i]->SetGlobalOrientation(orientation);
        mGPSMGenerators[i]->SetGlobalScale(mGlobalScale);
        mGPSMGenerators[i]->SetLocalScale(mLocalScale);
#else
        if (mTranslationDirty) {
          mGPSMGenerators[i]->SetGlobalTranslation(translation);
        }
        if (mOrientationDirty) {
          mGPSMGenerators[i]->SetGlobalOrientation(orientation);
        }
        if (mGlobalScaleDirty) {
          mGPSMGenerators[i]->SetGlobalScale(mGlobalScale);
        }
        if (mLocalScaleDirty) {
          mGPSMGenerators[i]->SetLocalScale(mLocalScale);
        }
#endif

      }
    }
    if (mHaveEPSM) {
      for (int i = 0; i < mSCNT; ++i) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
        mEPSMGenerators[i]->SetGlobalTranslation(translation);
        mEPSMGenerators[i]->SetGlobalOrientation(orientation);
        mEPSMGenerators[i]->SetGlobalScale(mGlobalScale);
        mEPSMGenerators[i]->SetLocalScale(mLocalScale);
#else
        if (mTranslationDirty) {
          mEPSMGenerators[i]->SetGlobalTranslation(translation);
        }
        if (mOrientationDirty) {
          mEPSMGenerators[i]->SetGlobalOrientation(orientation);
        }
        if (mGlobalScaleDirty) {
          mEPSMGenerators[i]->SetGlobalScale(mGlobalScale);
        }
        if (mLocalScaleDirty) {
          mEPSMGenerators[i]->SetLocalScale(mLocalScale);
        }
#endif

      }
    }
    updated = true;
#if !RSTL_PRIME1
    mTranslationDirty = false;
    mOrientationDirty = false;
    mGlobalScaleDirty = false;
    mLocalScaleDirty = false;
#endif

  }
  while (evalTime < mCurTime) {
    CParticleGlobals::SetEmitterTime(mCurrentFrame);
    UpdateElectricalEffects();
    if (emitting) {
      AddElectricalEffects();
    }
    if (mHaveGPSM) {
      if (mCurrentFrame >= mLIFE) {
        for (int i = 0; i < mSCNT; ++i) {
          mGPSMGenerators[i]->EndLifetime();
        }
      }
      for (int i = 0; i < mSCNT; ++i) {
        mGPSMGenerators[i]->Update(1.0 / 60.0);
      }
    }
    if (mHaveEPSM) {
      if (mCurrentFrame >= mLIFE) {
        for (int i = 0; i < mSCNT; ++i) {
          mEPSMGenerators[i]->EndLifetime();
        }
      }
      for (int i = 0; i < mSCNT; ++i) {
        mEPSMGenerators[i]->Update(1.0 / 60.0);
      }
    }
    updated = true;
    evalTime += 1.0 / 60.0;
    ++mCurrentFrame;
  }
  if (updated) {
    BuildBounds();
  }
  return updated;
}

void CParticleElectric::Render() {
  const float startTime = CStopwatch::GetGlobalTime();
#if !RSTL_PRIME1
  if (ShouldDraw()) {
#endif
  if (!mElectricManagers.empty()) {
    if (mTransformDirty) {
      UpdateCachedTransform();
    }
    if (mHaveSSWH) {
      RenderSwooshes();
    }
    if (mHaveLWD) {
      RenderLines();
    }
  }
  if (mHaveGPSM) {
    for (int i = 0; i < mSCNT; ++i) {
      mGPSMGenerators[i]->Render();
    }
  }
  if (mHaveEPSM) {
    for (int i = 0; i < mSCNT; ++i) {
      mEPSMGenerators[i]->Render();
    }
  }
#if !RSTL_PRIME1
  }
#endif
  mRenderTime = CStopwatch::GetGlobalTime() - startTime;
}

void CParticleElectric::RenderSwooshes() {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  for (AUTO(it, mElectricManagers.begin()); it != mElectricManagers.end(); ++it) {
#else
  for (rstl::list< CParticleElectricManager >::iterator it = mElectricManagers.begin();
       it != mElectricManagers.end(); ++it) {
#endif

    mSwooshGenerators[it->mIdx]->Render();
  }
}

void CParticleElectric::RenderLines() {
#if !RSTL_PRIME1
  if (CElementGen::sMoveRedToAlphaBuffer) {
    GXSetAlphaUpdate(GX_FALSE);
  }
#endif

  CGraphics::DisableAllLights();
  CGraphics::SetDepthWriteMode(true, kE_LEqual, false);
  CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_One, kLO_Clear);
  CGraphics::SetModelMatrix(CTransform4f::Translate(mGlobalTranslation) * mGlobalOrientation *
                            CTransform4f::Translate(mTranslation) * mOrientation *
                            CTransform4f::Scale(mGlobalScale) * CTransform4f::Scale(mLocalScale));
  CGraphics::SetCullMode(kCM_None);
#if !RSTL_PRIME1
  const bool textured = mElecDesc->mTEXR != nullptr;
#endif

  SetupLineGXMaterial();
#if !RSTL_PRIME1
  if (textured) {
    mElecDesc->mTEXR->GetValueTexture(0)->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
    for (rstl::list< CParticleElectricManager >::iterator it = mElectricManagers.begin();
         it != mElectricManagers.end(); ++it) {
      CLineManager& line = *mLineManagers[it->mIdx];
      int frame = mCurrentFrame - it->mStartFrame;
      CParticleGlobals::SetParticleLifetime(it->mEndFrame - it->mStartFrame);
      CParticleGlobals::UpdateParticleLifetimeTweenValues(frame);
      SUVElementSet uvs;
      mElecDesc->mTEXR->GetValueUV(frame, uvs);
      if (mElecDesc->mLWD1) {
        DrawTexturedLineStrip(line.mVerts, line.mWidth1, line.mColor1, uvs);
      }
      if (mElecDesc->mLWD2) {
        DrawTexturedLineStrip(line.mVerts, line.mWidth2, line.mColor2, uvs);
      }
      if (mElecDesc->mLWD3) {
        DrawTexturedLineStrip(line.mVerts, line.mWidth3, line.mColor3, uvs);
      }
    }
  } else {
#endif
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    for (AUTO(it, mElectricManagers.begin()); it != mElectricManagers.end(); ++it) {
#else
    for (rstl::list< CParticleElectricManager >::iterator it = mElectricManagers.begin();
         it != mElectricManagers.end(); ++it) {
#endif
    CLineManager& line = *mLineManagers[it->mIdx];
    if (mElecDesc->mLWD1) {
      DrawLineStrip(line.mVerts, line.mWidth1, line.mColor1);
    }
    if (mElecDesc->mLWD2) {
      DrawLineStrip(line.mVerts, line.mWidth2, line.mColor2);
    }
    if (mElecDesc->mLWD3) {
      DrawLineStrip(line.mVerts, line.mWidth3, line.mColor3);
    }
  }
#if !RSTL_PRIME1
  }
#endif

  CGraphics::SetCullMode(kCM_Front);
  CGraphics::SetLineWidth(1.f, kTO_Zero);
#if !RSTL_PRIME1
  if (CElementGen::sMoveRedToAlphaBuffer) {
    GXSetAlphaUpdate(GX_TRUE);
  }
#endif

}

void CParticleElectric::SetupLineGXMaterial() {
#if !RSTL_PRIME1
  if (mElecDesc->mTEXR) {
    static const GXVtxDescList vtxDescTex[] = {
        {GX_VA_POS, GX_DIRECT}, {GX_VA_TEX0, GX_DIRECT}, {GX_VA_NULL, GX_NONE}};
    CGX::SetVtxDescv(vtxDescTex);
    CGX::SetNumChans(0);
    CGX::SetNumTexGens(1);
    CGX::SetNumTevStages(1);
    CGX::SetNumIndStages(0);
    CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE,
                     GX_AF_NONE);
    CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_ZERO);
    CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
    CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
    CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
    CGX::SetTevDirect(GX_TEVSTAGE0);
    CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE,
                        GX_PTIDENTITY);
  } else {
#endif
  static const GXVtxDescList vtxDesc[] = {{GX_VA_POS, GX_DIRECT}, {GX_VA_NULL, GX_NONE}};
  CGX::SetVtxDescv(vtxDesc);
  CGX::SetNumChans(1);
  CGX::SetNumTexGens(0);
  CGX::SetNumTevStages(1);
  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_KONST);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
#if !RSTL_PRIME1
  }
#endif
}

void CParticleElectric::DrawLineStrip(
    const rstl::vector< CVector3f, rstl::aligned_allocator >& verts, float width,
    const CColor& color) {
  CGraphics::SetLineWidth(width, kTO_Zero);
  CGX::SetTevKColor(GX_KCOLOR0, CColor::Modulate(GetModulationColor(), color).GetGXColor());
  const int count = verts.size();
  CGX::Begin(GX_LINESTRIP, GX_VTXFMT0, count);
  for (int i = 0; i < count; ++i) {
    const CVector3f& vert = verts[i];
    GXPosition3f32(vert[kDX], vert[kDY], vert[kDZ]);
#if !RSTL_PRIME1
    CGraphics::DelayStreamVertex();
#endif
  }
  CGX::End();
}

#if !RSTL_PRIME1
void CParticleElectric::DrawTexturedLineStrip(
    const rstl::vector< CVector3f, rstl::aligned_allocator >& verts, float width,
    const CColor& color, const SUVElementSet& uvs) {
  const int count = verts.size();
  const float du = (uvs.xMax - uvs.xMin) / static_cast< float >(count - 1);
  const float dv = (uvs.yMax - uvs.yMin) / static_cast< float >(count - 1);
  float u = uvs.xMin;
  float v = uvs.yMin;
  CGraphics::SetLineWidth(width, kTO_Zero);
  CGX::SetTevKColor(GX_KCOLOR0, CColor::Modulate(GetModulationColor(), color).GetGXColor());
  CGX::Begin(GX_LINESTRIP, GX_VTXFMT0, count);
  for (int i = 0; i < count; ++i) {
    const CVector3f& vert = verts[i];
    GXPosition3f32(vert[kDX], vert[kDY], vert[kDZ]);
    GXTexCoord2f32(u, v);
    u += du;
    v += dv;
    CGraphics::DelayStreamVertex();
  }
  CGX::End();
}
#endif

void CParticleElectric::AddElectricalEffects() {
  float rate = 0.f;
  if (mElecDesc->mGRAT) {
    if (mElecDesc->mGRAT->GetValue(mCurrentFrame, rate)) {
      mElectricManagers.clear();
      for (int i = 0; i < mAllocated.size(); ++i) {
        mAllocated[i] = false;
      }
      return;
    } else {
      rate = rstl::max_val(0.f, rate);
    }
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  mGenRem += rate;
  const int count = static_cast< int >(floor(mGenRem));
#else
  mGenRem += rate * mGeneratorRate;
  const int count = static_cast< int >(floorf(mGenRem));
#endif

  mGenRem -= count;
  CreateNewParticles(count);
}

#if !RSTL_PRIME1
CParticleElectric::CParticleElectricManager::CParticleElectricManager(uint idx, uint slif,
                                                                      uint startFrame)
: mIdx(idx)
, mSLIF(slif)
, mStartFrame(startFrame)
, mEndFrame(mStartFrame + mSLIF)
, mGPSMIdx(-1)
, mEPSMIdx(-1)
, mInitialPos(CVector3f::Zero())
, mInitialVel(CVector3f::Zero())
, mFinalPos(CVector3f::Zero())
, mFinalVel(CVector3f::Zero()) {}
#endif

void CParticleElectric::CreateNewParticles(int count) {
#if !RSTL_PRIME1
  CParticleGlobals::SParticleSystem system('ELSC', this);
#endif

  int allocIdx = 0;
  for (int i = 0; i < count; ++i) {
    if (mElectricManagers.size() >= mSCNT) {
      return;
    }
    const CTransform4f orientation = mCachedXf.GetRotation();
    const CVector3f translation = mCachedXf.GetTranslation();
    for (; allocIdx < mAllocated.size(); ++allocIdx) {
      if (mAllocated[allocIdx]) {
        continue;
      }
      mAllocated[allocIdx] = true;
      int lifetime = 1;
      if (mElecDesc->mSLIF) {
        mElecDesc->mSLIF->GetValue(mCurrentFrame, lifetime);
      }
      mElectricManagers.push_back(CParticleElectricManager(allocIdx, lifetime, mCurrentFrame));
      CParticleElectricManager& manager = mElectricManagers.back();
      int frame = mCurrentFrame - manager.mStartFrame;
      CParticleGlobals::SetParticleLifetime(manager.mEndFrame - manager.mStartFrame);
      CParticleGlobals::UpdateParticleLifetimeTweenValues(frame);
#if !RSTL_PRIME1
      CParticleGlobals::SetCurrentParticle(reinterpret_cast< CElementGen::CParticle* >(&manager));
#endif

      CalculatePoints();
      if (mHaveSSWH) {
        CParticleSwoosh& swoosh = *mSwooshGenerators[allocIdx];
        swoosh.SetParticleEmission(true);
        swoosh.SetGlobalTranslation(translation);
        swoosh.SetGlobalOrientation(orientation);
        swoosh.SetGlobalScale(mGlobalScale);
        swoosh.SetLocalScale(mLocalScale);
        CColor color = CColor::White();
        CElectricDescription* desc = *mElecDesc;
        if (desc->mCOLR) {
          mElecDesc->mCOLR->GetValue(frame, color);
        }
        swoosh.SetModulationColor(CColor::Modulate(color, mModuColor));
        int curParticle = swoosh.mCurParticle;
        for (int j = 0; j < swoosh.mSwooshes.size(); ++j) {
          curParticle = (curParticle + 1) % swoosh.mSwooshes.size();
          CParticleSwoosh::SSwooshData& data = swoosh.mSwooshes[curParticle];
          data.mTranslation = mCalculatedVerts[j];
        }
      }
      if (mHaveLWD) {
        CLineManager& line = *mLineManagers[allocIdx];
        line.mVerts = mCalculatedVerts;
        UpdateLine(allocIdx, 0);
        if (!mHaveSSWH) {
          mMinBounds = CVector3f(FLT_MAX, FLT_MAX, FLT_MAX);
          mMaxBounds = CVector3f(-FLT_MAX, -FLT_MAX, -FLT_MAX);
          for (int j = 0; j < mCalculatedVerts.size(); ++j) {
            const CVector3f& vert = mCalculatedVerts[j];
            mMaxBounds = CVector3f(rstl::max_val(vert.GetX(), mMaxBounds.GetX()),
                                       rstl::max_val(vert.GetY(), mMaxBounds.GetY()),
                                       rstl::max_val(vert.GetZ(), mMaxBounds.GetZ()));
            mMinBounds = CVector3f(rstl::min_val(vert.GetX(), mMinBounds.GetX()),
                                       rstl::min_val(vert.GetY(), mMinBounds.GetY()),
                                       rstl::min_val(vert.GetZ(), mMinBounds.GetZ()));
          }
          line.mBounds = CAABox(mMinBounds, mMaxBounds);
        }
      }
      if (mHaveGPSM) {
        for (int j = 0; j < mSCNT; ++j) {
          if (!mGPSMGenerators[j]->GetParticleEmission()) {
            const CTransform4f scale =
                CTransform4f::Scale(mGlobalScale) * CTransform4f::Scale(mLocalScale);
            mGPSMGenerators[j]->SetTranslation(scale * mCalculatedVerts.front());
            mGPSMGenerators[j]->SetParticleEmission(true);
            manager.mGPSMIdx = j;
            break;
          }
        }
      }
      if (mHaveEPSM) {
        for (int j = 0; j < mSCNT; ++j) {
          if (!mEPSMGenerators[j]->GetParticleEmission()) {
            const CTransform4f scale =
                CTransform4f::Scale(mGlobalScale) * CTransform4f::Scale(mLocalScale);
            mEPSMGenerators[j]->SetTranslation(scale * mCalculatedVerts.back());
            mEPSMGenerators[j]->SetParticleEmission(true);
            manager.mEPSMIdx = j;
            break;
          }
        }
      }
#if !RSTL_PRIME1
      ++mCumulativeParticles;
#endif

      break;
    }
  }
}

void CParticleElectric::UpdateElectricalEffects() {
#if !RSTL_PRIME1
  CParticleGlobals::SParticleSystem system('ELSC', this);
#endif
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  for (AUTO(it, mElectricManagers.begin()); it != mElectricManagers.end();) {
#else
  for (rstl::list< CParticleElectricManager >::iterator it = mElectricManagers.begin();
       it != mElectricManagers.end();) {
#endif

    CParticleElectricManager& manager = *it;
    if (manager.mSLIF <= 1) {
      mAllocated[manager.mIdx] = false;
      if (manager.mGPSMIdx != -1) {
        mGPSMGenerators[manager.mGPSMIdx]->SetParticleEmission(false);
      }
      if (manager.mEPSMIdx != -1) {
        mEPSMGenerators[manager.mEPSMIdx]->SetParticleEmission(false);
      }
      it = mElectricManagers.erase(it);
      continue;
    }
    int frame = mCurrentFrame - manager.mStartFrame;
    CParticleGlobals::SetParticleLifetime(manager.mEndFrame - manager.mStartFrame);
    CParticleGlobals::UpdateParticleLifetimeTweenValues(frame);
    if (mHaveSSWH) {
      CParticleSwoosh& swoosh = *mSwooshGenerators[manager.mIdx];
      CColor color = CColor::White();
      if (mElecDesc->mCOLR) {
        mElecDesc->mCOLR->GetValue(frame, color);
      }
      swoosh.SetModulationColor(CColor::Modulate(color, mModuColor));
#if !RSTL_PRIME1
      swoosh.UpdateAllBounds();
#endif

    }
    if (mHaveLWD) {
      UpdateLine(manager.mIdx, frame);
    }
    --manager.mSLIF;
    ++it;
  }
}

void CParticleElectric::UpdateLine(int idx, int frame) {
  CColorElement* element = mElecDesc->mLCL1;
  CLineManager& line = *mLineManagers[idx];
  if (element) {
    element->GetValue(frame, line.mColor1);
  }
  if (CColorElement* element = mElecDesc->mLCL2) {
    element->GetValue(frame, line.mColor2);
  }
  if (CColorElement* element = mElecDesc->mLCL3) {
    element->GetValue(frame, line.mColor3);
  }
  if (CRealElement* element = mElecDesc->mLWD1) {
    element->GetValue(frame, line.mWidth1);
#if !RSTL_PRIME1
    line.mWidth1 = rstl::min_val(42.5f, line.mWidth1);
#endif

  }
  if (CRealElement* element = mElecDesc->mLWD2) {
    element->GetValue(frame, line.mWidth2);
#if !RSTL_PRIME1
    line.mWidth2 = rstl::min_val(42.5f, line.mWidth2);
#endif

  }
  if (CRealElement* element = mElecDesc->mLWD3) {
    element->GetValue(frame, line.mWidth3);
#if !RSTL_PRIME1
    line.mWidth3 = rstl::min_val(42.5f, line.mWidth3);
#endif

  }
}

void CParticleElectric::BuildBounds() {
  if (GetParticleCount() <= 0) {
    mSystemBounds = CAABox::MakeMaxInvertedBox();
    return;
  }
  mSystemBounds = CAABox::MakeMaxInvertedBox();
  if (mHaveSSWH) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    for (AUTO(it, mElectricManagers.begin()); it != mElectricManagers.end(); ++it) {
#else
    for (rstl::list< CParticleElectricManager >::iterator it = mElectricManagers.begin();
         it != mElectricManagers.end(); ++it) {
#endif

      rstl::optional_object< CAABox > bounds = mSwooshGenerators[it->mIdx]->GetBounds();
      if (bounds) {
        mSystemBounds.Include(*bounds);
      }
    }
  } else if (mHaveLWD) {
    CAABox bounds = CAABox::MakeMaxInvertedBox();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    for (AUTO(it, mElectricManagers.begin()); it != mElectricManagers.end(); ++it) {
#else
    for (rstl::list< CParticleElectricManager >::iterator it = mElectricManagers.begin();
         it != mElectricManagers.end(); ++it) {
#endif

      bounds.Include(mLineManagers[it->mIdx]->mBounds);
    }
    if (!bounds.Invalid()) {
      mSystemBounds.Include(
          bounds.GetTransformedAABox(CTransform4f::Translate(mGlobalTranslation) *
                                     mGlobalOrientation * CTransform4f::Translate(mTranslation) *
                                     mOrientation * CTransform4f::Scale(mGlobalScale)));
    }
  }
  if (mHaveGPSM) {
    for (int i = 0; i < mSCNT; ++i) {
      rstl::optional_object< CAABox > bounds = mGPSMGenerators[i]->GetBounds();
      if (bounds) {
        mSystemBounds.Include(*bounds);
      }
    }
  }
  if (mHaveEPSM) {
    for (int i = 0; i < mSCNT; ++i) {
      rstl::optional_object< CAABox > bounds = mEPSMGenerators[i]->GetBounds();
      if (bounds) {
        mSystemBounds.Include(*bounds);
      }
    }
  }
}

rstl::optional_object< CAABox > CParticleElectric::GetBounds() const {
  if (GetParticleCount() <= 0) {
    return rstl::optional_object_null();
  }
  return mSystemBounds;
}

#if !RSTL_PRIME1
int CParticleElectric::GetEmitterTime() const { return mCurrentFrame; }
#endif

void CParticleElectric::SetModulationColor(const CColor& color) { mModuColor = color; }

bool CParticleElectric::IsSystemDeletable() const {
  if (mEmitting && mCurrentFrame < mLIFE) {
    return false;
  }
  if (!mElectricManagers.empty()) {
    return false;
  }
  if (mHaveGPSM) {
    for (int i = 0; i < mSCNT; ++i) {
      if (!mGPSMGenerators[i]->IsSystemDeletable()) {
        return false;
      }
    }
  }
  if (mHaveEPSM) {
    for (int i = 0; i < mSCNT; ++i) {
      if (!mEPSMGenerators[i]->IsSystemDeletable()) {
        return false;
      }
    }
  }
  return true;
}

int CParticleElectric::GetParticleCount() const {
  int count = 0;
  if (!mElectricManagers.empty()) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    for (AUTO(it, mElectricManagers.begin()); it != mElectricManagers.end(); ++it) {
#else
    for (rstl::list< CParticleElectricManager >::const_iterator it = mElectricManagers.begin();
         it != mElectricManagers.end(); ++it) {
#endif

      if (mHaveSSWH) {
        count += mSwooshGenerators[it->mIdx]->GetParticleCount();
      }
      if (mHaveLWD) {
        count += mSSEG;
      }
    }
  }
  if (mHaveGPSM) {
    for (int i = 0; i < mSCNT; ++i) {
      count += mGPSMGenerators[i]->GetParticleCount();
    }
  }
  if (mHaveEPSM) {
    for (int i = 0; i < mSCNT; ++i) {
      count += mEPSMGenerators[i]->GetParticleCount();
    }
  }
  return count;
}

#if !RSTL_PRIME1
int CParticleElectric::GetSystemCount() const {
  int count = mElectricManagers.size();
  if (mHaveGPSM) {
    for (int i = 0; i < mSCNT; ++i) {
      count += mGPSMGenerators[i]->GetSystemCount();
    }
  }
  if (mHaveEPSM) {
    for (int i = 0; i < mSCNT; ++i) {
      count += mEPSMGenerators[i]->GetSystemCount();
    }
  }
  return count;
}
#endif

void CParticleElectric::ForceParticleCreation(int count) {
  CGlobalRandom random(mRandState);
  CreateNewParticles(count);
}

void CParticleElectric::SetTranslation(const CVector3f& translation) {
  mTranslation = translation;
  mTransformDirty = true;
#if !RSTL_PRIME1
  mTranslationDirty = true;
#endif

}

void CParticleElectric::SetOrientation(const CTransform4f& orientation) {
  mOrientation = orientation;
  mInvOrientation = mOrientation.GetQuickInverse();
  mTransformDirty = true;
#if !RSTL_PRIME1
  mOrientationDirty = true;
#endif

}

void CParticleElectric::SetGlobalTranslation(const CVector3f& translation) {
  mGlobalTranslation = translation;
  mTransformDirty = true;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (mHaveSSWH) {
    for (AUTO(it, mElectricManagers.begin()); it != mElectricManagers.end(); ++it) {
      mSwooshGenerators[it->mIdx]->SetGlobalTranslation(mGlobalTranslation);
    }
  }
  if (mHaveGPSM) {
    for (int i = 0; i < mSCNT; ++i) {
      mGPSMGenerators[i]->SetGlobalTranslation(mGlobalTranslation);
    }
  }
  if (mHaveEPSM) {
    for (int i = 0; i < mSCNT; ++i) {
      mEPSMGenerators[i]->SetGlobalTranslation(mGlobalTranslation);
    }
  }
#else
  mTranslationDirty = true;
#endif

}

void CParticleElectric::SetGlobalOrientation(const CTransform4f& orientation) {
  mGlobalOrientation = orientation.GetRotation();
  mTransformDirty = true;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (mHaveSSWH) {
    for (AUTO(it, mElectricManagers.begin()); it != mElectricManagers.end(); ++it) {
      mSwooshGenerators[it->mIdx]->SetGlobalOrientation(mGlobalOrientation);
    }
  }
  if (mHaveGPSM) {
    for (int i = 0; i < mSCNT; ++i) {
      mGPSMGenerators[i]->SetGlobalOrientation(mGlobalOrientation);
    }
  }
  if (mHaveEPSM) {
    for (int i = 0; i < mSCNT; ++i) {
      mEPSMGenerators[i]->SetGlobalOrientation(mGlobalOrientation);
    }
  }
#else
  mOrientationDirty = true;
#endif

}

void CParticleElectric::SetGlobalScale(const CVector3f& scale) {
  mGlobalScale = scale;
  mTransformDirty = true;
#if !RSTL_PRIME1
  mGlobalScaleDirty = true;
#endif

}

void CParticleElectric::SetLocalScale(const CVector3f& scale) {
  mLocalScale = scale;
  mTransformDirty = true;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (mHaveSSWH) {
    for (AUTO(it, mElectricManagers.begin()); it != mElectricManagers.end(); ++it) {
      mSwooshGenerators[it->mIdx]->SetLocalScale(mLocalScale);
    }
  }
  if (mHaveGPSM) {
    for (int i = 0; i < mSCNT; ++i) {
      mGPSMGenerators[i]->SetLocalScale(mLocalScale);
    }
  }
  if (mHaveEPSM) {
    for (int i = 0; i < mSCNT; ++i) {
      mEPSMGenerators[i]->SetLocalScale(mLocalScale);
    }
  }
#else
  mLocalScaleDirty = true;
#endif

}

void CParticleElectric::UpdateCachedTransform() {
  mCachedXf = CTransform4f::Translate(mGlobalTranslation) * mGlobalOrientation *
                 CTransform4f::Translate(mTranslation) * mOrientation;
  mTransformDirty = false;
}

bool CParticleElectric::SystemHasLight() const {
  if (mHaveGPSM) {
    return mGPSMGenerators.front()->SystemHasLight();
  }
  if (mHaveEPSM) {
    return mEPSMGenerators.front()->SystemHasLight();
  }
  return false;
}

CLight CParticleElectric::GetLight() const {
  if (mHaveGPSM) {
    return mGPSMGenerators.front()->GetLight();
  }
  if (mHaveEPSM) {
    return mEPSMGenerators.front()->GetLight();
  }
  return CLight::BuildLocalAmbient(GetGlobalTranslation(), CColor::Orange());
}

// TODO: verify that this Prime/Echoes difference is real
void CParticleElectric::SetParticleEmission(RSTL_PRIME1_CONST bool emission) {
  mEmitting = emission;
}

uint CParticleElectric::Get4CharId() const { return 'ELSC'; }
#if !RSTL_PRIME1
void CParticleElectric::SetGeneratorRate(float rate) { mGeneratorRate = rstl::max_val(rate, 0.f); }

float CParticleElectric::GetGeneratorRate() const { return mGeneratorRate; }
#endif

void CParticleElectric::DestroyParticles() {}

#if !RSTL_PRIME1
const CColor& CParticleElectric::GetModulationColor() const { return mModuColor; }

const CVector3f& CParticleElectric::GetGlobalScale() const { return mGlobalScale; }

#endif
const CTransform4f& CParticleElectric::GetGlobalOrientation() const { return mGlobalOrientation; }

const CVector3f& CParticleElectric::GetGlobalTranslation() const { return mGlobalTranslation; }
const CTransform4f& CParticleElectric::GetOrientation() const { return mOrientation; }
const CVector3f& CParticleElectric::GetTranslation() const { return mTranslation; }

