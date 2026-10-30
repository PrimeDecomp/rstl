#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CAnimTreeTimeScale.hpp"

#include "Kyoto/Animation/CBoolPOINode.hpp"
#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Animation/CParticlePOINode.hpp"
#include "Kyoto/Animation/CSoundPOINode.hpp"

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CAdvancementResults CAnimTreeTimeScale::VAdvanceView(const CCharAnimTime& dt) {
  if (dt.EqualsZero() && dt > CCharAnimTime::ZeroFlat())
#else
SAdvancementResults CAnimTreeTimeScale::VAdvanceView(const CCharAnimTime& dt) {
  if (dt.EqualsZero() && dt > CCharAnimTime::ZeroFlat()) {
#endif

    return mChild->AdvanceView(dt);
#if !RSTL_PRIME1
  }
#endif

  CCharAnimTime origAccelTime = mCurAccelTime;
  CCharAnimTime newTime = mCurAccelTime + dt;
  if (newTime < mTargetAccelTime) {
    CCharAnimTime integral = mTimeScale->TimeScaleIntegral(origAccelTime, newTime);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    CAdvancementResults res = mChild->AdvanceView(integral);
#else
    SAdvancementResults res = mChild->AdvanceView(integral);
#endif

    if (res.mRemTime.EqualsZero()) {
      mCurAccelTime = newTime;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      return CAdvancementResults(CCharAnimTime::ZeroFlat(), res.mDeltas);
#else
      return SAdvancementResults(CCharAnimTime::ZeroFlat(), res.mDeltas);
#endif

    } else {
      mCurAccelTime = mTimeScale->FindUpperLimit(origAccelTime, integral - res.mRemTime);
      CCharAnimTime elapsed = mCurAccelTime - origAccelTime;
      CCharAnimTime remaining = dt - elapsed;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      return CAdvancementResults(remaining, res.mDeltas);
#else
      return SAdvancementResults(remaining, res.mDeltas);
#endif

    }
  } else {
    CCharAnimTime newDt = mTimeScale->TimeScaleIntegral(origAccelTime, mTargetAccelTime);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    CAdvancementResults res(CCharAnimTime(0.f), CAdvancementDeltas());
    if (newDt.GreaterThanZero())
#else
    SAdvancementResults res(CCharAnimTime(0.f),
                           SAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation()));
    if (newDt.GreaterThanZero()) {
#endif

      res = mChild->AdvanceView(newDt);
#if !RSTL_PRIME1
    }
#endif

    CCharAnimTime remTime = res.mRemTime + (newTime - mTargetAccelTime);
    mCurAccelTime = mTargetAccelTime;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    return CAdvancementResults(remTime, res.mDeltas);
#else
    return SAdvancementResults(remTime, res.mDeltas);
#endif

  }
}

CCharAnimTime CAnimTreeTimeScale::VGetTimeRemaining() const {
  CCharAnimTime timeRem = mChild->GetTimeRemaining();
  if (mTargetAccelTime == CCharAnimTime::Infinity()) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    CCharAnimTime remaining =
        mTimeScale->FindUpperLimit(mCurAccelTime, timeRem) - mCurAccelTime;
#else
    CCharAnimTime remaining = mTimeScale->FindUpperLimit(mCurAccelTime, timeRem) - mCurAccelTime;
#endif

    return remaining;
  }
  return GetRealLifeTime(timeRem);
}

CSteadyStateAnimInfo CAnimTreeTimeScale::VGetSteadyStateAnimInfo() const {
  CSteadyStateAnimInfo info = mChild->GetSteadyStateAnimInfo();
  CCharAnimTime originalDuration = info.GetDuration();
  if (mTargetAccelTime == CCharAnimTime::Infinity()) {
    const CCharAnimTime duration =
        mTimeScale->FindUpperLimit(CCharAnimTime::ZeroFlat(), originalDuration);
    return CSteadyStateAnimInfo(info.IsLooping(), duration, info.GetOffset());
  } else {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    CCharAnimTime time =
        mCurAccelTime.GreaterThanZero()
#else
    CCharAnimTime time = mCurAccelTime.GreaterThanZero()
#endif

            ? mTimeScale->TimeScaleIntegral(CCharAnimTime::ZeroFlat(), mCurAccelTime)
            : CCharAnimTime::ZeroFlat();
    CCharAnimTime remaining = GetTimeRemaining();
    CCharAnimTime duration = mInitialTime + time + remaining;
    return CSteadyStateAnimInfo(info.IsLooping(), duration, info.GetOffset());
  }
}

rstl::ownership_transfer< IAnimReader > CAnimTreeTimeScale::VClone() const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return rs_new CAnimTreeTimeScale(Cast(mChild->Clone()), mTimeScale->Clone(),
                                   mCurAccelTime, mTargetAccelTime, mInitialTime, mName);
#else
  return rs_new CAnimTreeTimeScale(Cast(mChild->Clone()), mTimeScale->Clone(), mCurAccelTime,
                                   mTargetAccelTime, mInitialTime, mName);
#endif

}

rstl::rc_ptr< CAnimTreeNode > CAnimTreeTimeScale::VGetBestUnblendedChild() const {
  rstl::rc_ptr< CAnimTreeNode > child = mChild->GetBestUnblendedChild();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (child)
#else
  if (child) {
#endif

    return rs_new CAnimTreeTimeScale(Cast(child->Clone()), mTimeScale->Clone(), mCurAccelTime,
                                     mTargetAccelTime, mInitialTime, mName);
#if !RSTL_PRIME1
  }
#endif

  return child;
}

CAnimTreeEffectiveContribution CAnimTreeTimeScale::VGetContributionOfHighestInfluence() const {
  CAnimTreeEffectiveContribution contribution = mChild->GetContributionOfHighestInfluence();
  float weight = contribution.GetContributionWeight();
  rstl::string name = contribution.GetPrimitiveName();
  CSteadyStateAnimInfo info = GetSteadyStateAnimInfo();
  CCharAnimTime time = GetTimeRemaining();
  return CAnimTreeEffectiveContribution(weight, name, info, time,
                                        contribution.GetAnimDatabaseIndex());
}

uint CAnimTreeTimeScale::VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                                         uint capacity, uint iterator, int unk) const {
#else
                                         uint capacity, uint iterator, int additive) const {
#endif

  const CCharAnimTime useTime =
      time == CCharAnimTime::Infinity() ? mChild->GetTimeRemaining() : GetRealLifeTime(time);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const uint ret = mChild->GetBoolPOIList(useTime, listOut, capacity, iterator, unk);
#else
  const uint ret = mChild->GetBoolPOIList(useTime, listOut, capacity, iterator, additive);
#endif

  if (mTargetAccelTime > CCharAnimTime::ZeroFlat()) {
    for (uint i = 0; i < ret; ++i) {
      CCharAnimTime realTime = GetRealLifeTime(listOut[i].GetTime());
      listOut[iterator + i].SetTime(realTime);
    }
  }
  return ret;
}

uint CAnimTreeTimeScale::VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                                          uint capacity, uint iterator, int unk) const {
#else
                                          uint capacity, uint iterator, int additive) const {
#endif

  const CCharAnimTime useTime =
      time == CCharAnimTime::Infinity() ? mChild->GetTimeRemaining() : GetRealLifeTime(time);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const uint ret = mChild->GetInt32POIList(useTime, listOut, capacity, iterator, unk);
#else
  const uint ret = mChild->GetInt32POIList(useTime, listOut, capacity, iterator, additive);
#endif

  if (mTargetAccelTime > CCharAnimTime::ZeroFlat()) {
    for (uint i = 0; i < ret; ++i) {
      CCharAnimTime realTime = GetRealLifeTime(listOut[i].GetTime());
      listOut[i + iterator].SetTime(realTime);
    }
  }
  return ret;
}

uint CAnimTreeTimeScale::VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                                             uint capacity, uint iterator, int unk) const {
#else
                                             uint capacity, uint iterator, int additive) const {
#endif

  const CCharAnimTime useTime =
      time == CCharAnimTime::Infinity() ? mChild->GetTimeRemaining() : GetRealLifeTime(time);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const uint ret = mChild->GetParticlePOIList(useTime, listOut, capacity, iterator, unk);
#else
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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                                          uint capacity, uint iterator, int unk) const {
#else
                                          uint capacity, uint iterator, int additive) const {
#endif

  const CCharAnimTime useTime =
      time == CCharAnimTime::Infinity() ? mChild->GetTimeRemaining() : GetRealLifeTime(time);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const uint ret = mChild->GetSoundPOIList(useTime, listOut, capacity, iterator, unk);
#else
  const uint ret = mChild->GetSoundPOIList(useTime, listOut, capacity, iterator, additive);
#endif

  if (mTargetAccelTime > CCharAnimTime::ZeroFlat()) {
    for (uint i = 0; i < ret; ++i) {
      CCharAnimTime realTime = GetRealLifeTime(listOut[i].GetTime());
      listOut[i + iterator].SetTime(realTime);
    }
  }
  return ret;
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
bool CAnimTreeTimeScale::VGetBoolPOIState(const char* name) const {
  return mChild->GetBoolPOIState(name);
#else
bool CAnimTreeTimeScale::VGetBoolPOIState(uint nameHash) const {
  return mChild->VGetBoolPOIState(nameHash);
#endif

}
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
s32 CAnimTreeTimeScale::VGetInt32POIState(const char* name) const {
  return mChild->GetInt32POIState(name);
#else
s32 CAnimTreeTimeScale::VGetInt32POIState(uint nameHash) const {
  return mChild->VGetInt32POIState(nameHash);
#endif

}
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CParticleData::EParentedMode CAnimTreeTimeScale::VGetParticlePOIState(const char* name) const {
  return mChild->GetParticlePOIState(name);
#else
CParticleData::EParentedMode CAnimTreeTimeScale::VGetParticlePOIState(uint nameHash) const {
  return mChild->VGetParticlePOIState(nameHash);
#endif

}

rstl::optional_object< rstl::ownership_transfer< IAnimReader > > CAnimTreeTimeScale::VSimplified() {
  rstl::optional_object< rstl::ownership_transfer< IAnimReader > > simp = mChild->Simplified();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (simp)
#else
  if (simp) {
#endif

    return rstl::ownership_transfer< IAnimReader >(
        rs_new CAnimTreeTimeScale(Cast(*simp), mTimeScale->Clone(), mCurAccelTime,
                                  mTargetAccelTime, mInitialTime, mName));
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (mCurAccelTime == mTargetAccelTime)
#else
  }
  if (mCurAccelTime == mTargetAccelTime) {
#endif

    return mChild->Clone();
#if !RSTL_PRIME1
  }
#endif

  return rstl::optional_object_null();
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CAnimTreeTimeScale::VSetPhase(float phase) { mChild->SetPhase(phase); }
#else
void CAnimTreeTimeScale::VSetPhase(float phase) { mChild->VSetPhase(phase); }
#endif

CCharAnimTime CAnimTreeTimeScale::GetRealLifeTime(const CCharAnimTime& time) const {
  CCharAnimTime timeRem = mChild->GetTimeRemaining();
  CCharAnimTime ret(rstl::min_val(time.GetSeconds(), timeRem.GetSeconds()));
  if (mTargetAccelTime > CCharAnimTime::ZeroFlat()) {
    CCharAnimTime accelRemaining = mTargetAccelTime - mCurAccelTime;
    if (ret < accelRemaining) {
      return mTimeScale->TimeScaleIntegral(mCurAccelTime, mCurAccelTime + ret);
    } else {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      CCharAnimTime integral =
          mTimeScale->TimeScaleIntegral(mCurAccelTime, mTargetAccelTime);
#else
      CCharAnimTime integral = mTimeScale->TimeScaleIntegral(mCurAccelTime, mTargetAccelTime);
#endif

      if (integral > ret) {
        CCharAnimTime upper = mTimeScale->FindUpperLimit(mCurAccelTime, ret);
        return upper - mCurAccelTime;
      } else {
        return integral + (ret - integral);
      }
    }
  }
  return ret;
}

rstl::string CAnimTreeTimeScale::CreatePrimitiveName(const rstl::ncrc_ptr< CAnimTreeNode >& node,
                                                     float scaleA, const CCharAnimTime& time,
                                                     float scaleB) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return rstl::string("");
#else
  return rstl::string_l("");
#endif

}

