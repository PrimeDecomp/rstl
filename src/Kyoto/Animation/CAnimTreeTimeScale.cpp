#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/RstlVersions.h"
#endif
#include "Kyoto/Animation/CAnimTreeTimeScale.hpp"

#if RSTL_CORRUPTION
#include "Kyoto/Animation/CAssetPOINode.hpp"
#include "Kyoto/Animation/CAssetSoundPOINode.hpp"
#endif
#include "Kyoto/Animation/CBoolPOINode.hpp"
#if !RSTL_CORRUPTION
#include "Kyoto/Animation/CInt32POINode.hpp"
#endif
#include "Kyoto/Animation/CParticlePOINode.hpp"
#include "Kyoto/Animation/CSoundPOINode.hpp"

SAdvancementResults CAnimTreeTimeScale::VAdvanceView(const CCharAnimTime& dt) {
  if (dt.EqualsZero() && dt > CCharAnimTime::ZeroFlat()) {
    return mChild->AdvanceView(dt);
  }

  CCharAnimTime origAccelTime = mCurAccelTime;
  CCharAnimTime newTime = mCurAccelTime + dt;
  if (newTime < mTargetAccelTime) {
    CCharAnimTime integral = mTimeScale->TimeScaleIntegral(origAccelTime, newTime);
    SAdvancementResults res = mChild->AdvanceView(integral);

    if (res.mRemTime.EqualsZero()) {
      mCurAccelTime = newTime;
      return SAdvancementResults(CCharAnimTime::ZeroFlat(), res.mDeltas);

    } else {
      mCurAccelTime = mTimeScale->FindUpperLimit(origAccelTime, integral - res.mRemTime);
      CCharAnimTime elapsed = mCurAccelTime - origAccelTime;
      CCharAnimTime remaining = dt - elapsed;
      return SAdvancementResults(remaining, res.mDeltas);

    }
  } else {
    CCharAnimTime newDt = mTimeScale->TimeScaleIntegral(origAccelTime, mTargetAccelTime);
#if RSTL_CORRUPTION
    SAdvancementResults res(CCharAnimTime(0.f),
                            SAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation()));
#else
    SAdvancementResults res(CCharAnimTime(0.f), CAdvancementDeltas(
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
                                                    CVector3f::Zero()
                                                    , CQuaternion::NoRotation()
#endif
                                                    ));
#endif
    if (newDt.GreaterThanZero()) {
      res = mChild->AdvanceView(newDt);
    }
    CCharAnimTime remTime = res.mRemTime + (newTime - mTargetAccelTime);
    mCurAccelTime = mTargetAccelTime;
    return SAdvancementResults(remTime, res.mDeltas);

  }
}

CCharAnimTime CAnimTreeTimeScale::VGetTimeRemaining() const {
  CCharAnimTime timeRem = mChild->GetTimeRemaining();
  if (mTargetAccelTime == CCharAnimTime::Infinity()) {
    CCharAnimTime remaining = mTimeScale->FindUpperLimit(mCurAccelTime, timeRem) - mCurAccelTime;
    return remaining;
  }
  return GetRealLifeTime(timeRem);
}

CSteadyStateAnimInfo CAnimTreeTimeScale::VGetSteadyStateAnimInfo() const {
  CSteadyStateAnimInfo info = mChild->GetSteadyStateAnimInfo();
#if !RSTL_CORRUPTION
  CCharAnimTime originalDuration = info.GetDuration();
#endif
  if (mTargetAccelTime == CCharAnimTime::Infinity()) {
    const CCharAnimTime duration =
#if RSTL_CORRUPTION
        mTimeScale->FindUpperLimit(CCharAnimTime::ZeroFlat(), info.GetDuration());
#else
        mTimeScale->FindUpperLimit(CCharAnimTime::ZeroFlat(), originalDuration);
#endif
    return CSteadyStateAnimInfo(info.IsLooping(), duration, info.GetOffset());
  } else {
    CCharAnimTime time = mCurAccelTime.GreaterThanZero()
            ? mTimeScale->TimeScaleIntegral(CCharAnimTime::ZeroFlat(), mCurAccelTime)
            : CCharAnimTime::ZeroFlat();
    CCharAnimTime remaining = GetTimeRemaining();
    CCharAnimTime duration = mInitialTime + time + remaining;
    return CSteadyStateAnimInfo(info.IsLooping(), duration, info.GetOffset());
  }
}

rstl::ownership_transfer< IAnimReader > CAnimTreeTimeScale::VClone() const {
  return rs_new_line(139) CAnimTreeTimeScale(Cast(mChild->Clone()), mTimeScale->Clone(),
                                             mCurAccelTime, mTargetAccelTime, mInitialTime, mName);
}

rstl::rc_ptr< CAnimTreeNode > CAnimTreeTimeScale::VGetBestUnblendedChild() const {
  rstl::rc_ptr< CAnimTreeNode > child = mChild->GetBestUnblendedChild();
  if (child) {
    return rs_new_line(154)
        CAnimTreeTimeScale(Cast(child->Clone()), mTimeScale->Clone(), mCurAccelTime,
                                     mTargetAccelTime, mInitialTime, mName);
  }
  return child;
}

CAnimTreeEffectiveContribution CAnimTreeTimeScale::VGetContributionOfHighestInfluence() const {
  CAnimTreeEffectiveContribution contribution = mChild->GetContributionOfHighestInfluence();
  float weight = contribution.GetContributionWeight();
#if !RSTL_CORRUPTION
  rstl::string name = contribution.GetPrimitiveName();
#endif
  CSteadyStateAnimInfo info = GetSteadyStateAnimInfo();
  CCharAnimTime time = GetTimeRemaining();
#if RSTL_CORRUPTION
  return CAnimTreeEffectiveContribution(weight, contribution.GetPrimitive(), info, time);
#else
  return CAnimTreeEffectiveContribution(weight, name, info, time,
                                        contribution.GetAnimDatabaseIndex());
#endif
}

uint CAnimTreeTimeScale::VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut,
#if RSTL_CORRUPTION
                                         uint capacity, uint iterator, const CSegIdMaskMap& additive) const {
#else
                                         uint capacity, uint iterator, int additive) const {
#endif
  const CCharAnimTime useTime =
      time == CCharAnimTime::Infinity() ? mChild->GetTimeRemaining() : GetRealLifeTime(time);
  const uint ret = mChild->GetBoolPOIList(useTime, listOut, capacity, iterator, additive);
  if (mTargetAccelTime > CCharAnimTime::ZeroFlat()) {
    for (uint i = 0; i < ret; ++i) {
      CCharAnimTime realTime = GetRealLifeTime(listOut[i].GetTime());
      listOut[iterator + i].SetTime(realTime);
    }
  }
  return ret;
}

#if RSTL_CORRUPTION
uint CAnimTreeTimeScale::VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut,
                                             uint capacity, uint iterator, const CSegIdMaskMap& additive) const {
#else
uint CAnimTreeTimeScale::VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut,
                                          uint capacity, uint iterator, int additive) const {
#endif
  const CCharAnimTime useTime =
      time == CCharAnimTime::Infinity() ? mChild->GetTimeRemaining() : GetRealLifeTime(time);
#if RSTL_CORRUPTION
  const uint ret = mChild->GetParticlePOIList(useTime, listOut, capacity, iterator, additive);
#else
  const uint ret = mChild->GetInt32POIList(useTime, listOut, capacity, iterator, additive);
  if (mTargetAccelTime > CCharAnimTime::ZeroFlat()) {
    for (uint i = 0; i < ret; ++i) {
      CCharAnimTime realTime = GetRealLifeTime(listOut[i].GetTime());
      listOut[i + iterator].SetTime(realTime);
    }
  }
  return ret;
}

uint CAnimTreeTimeScale::VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut,
                                             uint capacity, uint iterator, int additive) const {
  const CCharAnimTime useTime =
      time == CCharAnimTime::Infinity() ? mChild->GetTimeRemaining() : GetRealLifeTime(time);
  const uint ret = mChild->GetParticlePOIList(useTime, listOut, capacity, iterator, additive);
#endif
  if (mTargetAccelTime > CCharAnimTime::ZeroFlat()) {
    for (uint i = 0; i < ret; ++i) {
      CCharAnimTime realTime = GetRealLifeTime(listOut[i].GetTime());
      listOut[i + iterator].SetTime(realTime);
    }
  }
  return ret;
}

uint CAnimTreeTimeScale::VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut,
#if RSTL_CORRUPTION
                                          uint capacity, uint iterator, const CSegIdMaskMap& additive) const {
#else
                                          uint capacity, uint iterator, int additive) const {
#endif
  const CCharAnimTime useTime =
      time == CCharAnimTime::Infinity() ? mChild->GetTimeRemaining() : GetRealLifeTime(time);
  const uint ret = mChild->GetSoundPOIList(useTime, listOut, capacity, iterator, additive);
  if (mTargetAccelTime > CCharAnimTime::ZeroFlat()) {
    for (uint i = 0; i < ret; ++i) {
      CCharAnimTime realTime = GetRealLifeTime(listOut[i].GetTime());
      listOut[i + iterator].SetTime(realTime);
    }
  }
  return ret;
}

#if RSTL_CORRUPTION
uint CAnimTreeTimeScale::VGetAssetSoundPOIList(const CCharAnimTime& time,
                                               CAssetSoundPOINode* listOut, uint capacity,
                                               uint iterator, const CSegIdMaskMap& additive) const {
  const CCharAnimTime useTime =
      time == CCharAnimTime::Infinity() ? mChild->GetTimeRemaining() : GetRealLifeTime(time);
  const uint ret = mChild->GetAssetSoundPOIList(useTime, listOut, capacity, iterator, additive);
  if (mTargetAccelTime > CCharAnimTime::ZeroFlat()) {
    for (uint i = 0; i < ret; ++i) {
      CCharAnimTime realTime = GetRealLifeTime(listOut[i].GetTime());
      listOut[i + iterator].SetTime(realTime);
#else
bool CAnimTreeTimeScale::VGetBoolPOIState(
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    const char* name
#else
    uint name
#endif
    ) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return mChild->GetBoolPOIState(name);
#else
  return mChild->VGetBoolPOIState(name);
#endif
#endif
}
#if !RSTL_CORRUPTION
s32 CAnimTreeTimeScale::VGetInt32POIState(
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    const char* name
#else
    uint name
#endif
    ) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return mChild->GetInt32POIState(name);
#else
  return mChild->VGetInt32POIState(name);
#endif
#endif
}
#if RSTL_CORRUPTION
  return ret;
}
#else
CParticleData::EParentedMode CAnimTreeTimeScale::VGetParticlePOIState(
#endif
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_CORRUPTION
uint CAnimTreeTimeScale::VGetAssetPOIList(const CCharAnimTime& time, CAssetPOINode* listOut,
                                          uint capacity, uint iterator, const CSegIdMaskMap& additive) const {
  const CCharAnimTime useTime =
      time == CCharAnimTime::Infinity() ? mChild->GetTimeRemaining() : GetRealLifeTime(time);
  const uint ret = mChild->GetAssetPOIList(useTime, listOut, capacity, iterator, additive);
  if (mTargetAccelTime > CCharAnimTime::ZeroFlat()) {
    for (uint i = 0; i < ret; ++i) {
      CCharAnimTime realTime = GetRealLifeTime(listOut[i].GetTime());
      listOut[i + iterator].SetTime(realTime);
    }
  }
  return ret;
}
#else
#if RSTL_PRIME1
    const char* name
#else
    uint name
#endif
    ) const {
#endif
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_CORRUPTION
bool CAnimTreeTimeScale::VGetBoolPOIState(uint nameHash) const {
  return mChild->VGetBoolPOIState(nameHash);
}

CParticleData::EParentedMode CAnimTreeTimeScale::VGetParticlePOIState(uint nameHash) const {
  return mChild->VGetParticlePOIState(nameHash);
#else
#if RSTL_PRIME1
  return mChild->GetParticlePOIState(name);
#else
  return mChild->VGetParticlePOIState(name);
#endif
#endif
}

rstl::optional_object< rstl::ownership_transfer< IAnimReader > > CAnimTreeTimeScale::VSimplified() {
  rstl::optional_object< rstl::ownership_transfer< IAnimReader > > simp = mChild->Simplified();
  if (simp) {
    return rstl::ownership_transfer< IAnimReader >(rs_new_line(393) CAnimTreeTimeScale(
        Cast(*simp), mTimeScale->Clone(), mCurAccelTime, mTargetAccelTime, mInitialTime, mName));
  }
  if (mCurAccelTime == mTargetAccelTime) {
    return mChild->Clone();
  }
  return rstl::optional_object_null();
}

#if RSTL_CORRUPTION
void CAnimTreeTimeScale::VSetPhase(float phase) { mChild->VSetPhase(phase); }
#else
void CAnimTreeTimeScale::VSetPhase(float phase) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  mChild->SetPhase(phase);
#else
  mChild->VSetPhase(phase);
#endif
}
#endif

CCharAnimTime CAnimTreeTimeScale::GetRealLifeTime(const CCharAnimTime& time) const {
  CCharAnimTime timeRem = mChild->GetTimeRemaining();
  CCharAnimTime ret(rstl::min_val(time.GetSeconds(), timeRem.GetSeconds()));
  if (mTargetAccelTime > CCharAnimTime::ZeroFlat()) {
    CCharAnimTime accelRemaining = mTargetAccelTime - mCurAccelTime;
    if (ret < accelRemaining) {
      return mTimeScale->TimeScaleIntegral(mCurAccelTime, mCurAccelTime + ret);
    } else {
      CCharAnimTime integral = mTimeScale->TimeScaleIntegral(mCurAccelTime, mTargetAccelTime);
      if (integral > ret) {
        CCharAnimTime upper = mTimeScale->FindUpperLimit(mCurAccelTime, ret);
        return upper - mCurAccelTime;
      } else {
#if RSTL_CORRUPTION
        return accelRemaining + (ret - integral);
#else
        return integral + (ret - integral);
#endif
      }
    }
  }
  return ret;
}

rstl::string CAnimTreeTimeScale::CreatePrimitiveName(const rstl::ncrc_ptr< CAnimTreeNode >& node,
                                                     float scaleA, const CCharAnimTime& time,
                                                     float scaleB) {
#if RSTL_CORRUPTION
  return rstl::string_l("");
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return rstl::string("");
#else
  return rstl::string_l("");
#endif
#endif
}
