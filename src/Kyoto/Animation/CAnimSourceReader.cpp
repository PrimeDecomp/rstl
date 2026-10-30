#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CAnimSourceReader.hpp"
#include "Kyoto/Animation/CCharAnimTime.hpp"

CAnimSourceReader::CAnimSourceReader(const TSubAnimTypeToken< CAnimSource >& source,
                                     const CCharAnimTime& time
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
                                     , const CAnimPOIData* poiData
#endif
                                     )
: CAnimSourceReaderBase(rs_new CAnimSourceInfo(source)
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
                        , poiData
#endif
                        )
, mSource(source)
, mSteadyStateInfo(mSource->GetSteadyStateAnimInfo(time)) {
  PostConstruct(time);
}

SAdvancementResults CAnimSourceReader::VAdvanceView(const CCharAnimTime& dt) {

  const CCharAnimTime previousTime = mCurTime;
  const CCharAnimTime& duration = mSource->GetAnimationDuration();
  if (previousTime == duration) {
    mCurTime = CCharAnimTime::ZeroFlat();
    mPassedBoolCount = 0;
    mPassedIntCount = 0;
    mPassedParticleCount = 0;
    mPassedSoundCount = 0;
    return SAdvancementResults(dt, CAdvancementDeltas(
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
            CVector3f::Zero()
            , CQuaternion::NoRotation()
#endif
            ));

  }
  if (dt.EqualsZero()) {
    return SAdvancementResults(CCharAnimTime::ZeroFlat(), CAdvancementDeltas(
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
            CVector3f::Zero()
            , CQuaternion::NoRotation()
#endif
            ));

  }

  mCurTime += dt;
  CCharAnimTime remainingTime = CCharAnimTime::ZeroFlat();
  if (mCurTime > duration) {
    remainingTime = mCurTime - duration;
    mCurTime = duration;
  }
#if RSTL_PRIME1
  if (mSource->HasPOIData())
#endif
  {
    UpdatePOIStates();
  }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const CSegId root = CSegId::Root();
#else
  const CSegId root = CSegId(0);
#endif
  const CQuaternion oldRotation = mSource->GetRotation(root, previousTime);
  const CQuaternion newRotation = mSource->GetRotation(root, mCurTime);
  const CQuaternion inverseOldRotation = oldRotation.BuildInverted();
  CVector3f offset(0.f, 0.f, 0.f);
  if (mSource->HasOffset(root)) {
    const CVector3f oldOffset = mSource->GetOffset(root, previousTime);
    const CVector3f newOffset = mSource->GetOffset(root, mCurTime);
    offset = newOffset - oldOffset;
    const CQuaternion inverseNewRotation = newRotation.BuildInverted();
    const CMatrix3f inverseRotation = inverseNewRotation.BuildTransform();
    offset = inverseRotation * offset;
  }
  return SAdvancementResults(remainingTime,
                             CAdvancementDeltas(offset, newRotation * inverseOldRotation));

}

CCharAnimTime CAnimSourceReader::VGetTimeRemaining() const {
  return mSource->GetAnimationDuration() - mCurTime;
}

CSteadyStateAnimInfo CAnimSourceReader::VGetSteadyStateAnimInfo() const {
  return mSteadyStateInfo;
}

bool CAnimSourceReader::VHasOffset(const CSegId& seg) const {
  return mSource->HasOffset(seg);
}

CVector3f CAnimSourceReader::VGetOffset(const CSegId& seg) const {
  return mSource->GetOffset(seg, mCurTime);
}

CVector3f CAnimSourceReader::VGetOffset(const CSegId& seg, const CCharAnimTime& time) const {
  return mSource->GetOffset(seg, time);
}

CQuaternion CAnimSourceReader::VGetRotation(const CSegId& seg) const {
  return mSource->GetRotation(seg, mCurTime);
}

void CAnimSourceReader::VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& set) const {
  mSource->GetSegStatementSet(list, set, mCurTime);
}

void CAnimSourceReader::VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& set,
                                            const CCharAnimTime& time) const {
  mSource->GetSegStatementSet(list, set, time);
}

#if !RSTL_PRIME1

void CAnimSourceReader::VGetJointData_Linear(const CCharLayoutInfo& layout,
                                             CJointData_LinearStorage& data,
                                             const CCharAnimTime& time) const {
  mSource->GetJointData_Linear(layout, data, time);
}

void CAnimSourceReader::VGetJointData_Linear(const CCharLayoutInfo& layout,
                                             CJointData_LinearStorage& data) const {
  mSource->GetJointData_Linear(layout, data, mCurTime);
}
#endif

rstl::ownership_transfer< IAnimReader > CAnimSourceReader::VClone() const {
  return rstl::ownership_transfer< IAnimReader >(rs_new CAnimSourceReader(
      mSource
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
      , mPOIData
#endif
      , mCurTime, mSteadyStateInfo, mPassedBoolCount, mPassedIntCount, mPassedParticleCount,
      mPassedSoundCount, mBoolStates, mInt32States, mParticleStates));
}

SAdvancementResults CAnimSourceReader::VReverseView(const CCharAnimTime& dt) {

  const CCharAnimTime previousTime = mCurTime;
  const CCharAnimTime& duration = mSource->GetAnimationDuration();
  if (previousTime.EqualsZero()) {
    mCurTime = duration;
    return SAdvancementResults(dt, CAdvancementDeltas(
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
            CVector3f::Zero()
            , CQuaternion::NoRotation()
#endif
            ));

  }
  if (dt.EqualsZero()) {
    return SAdvancementResults(CCharAnimTime::ZeroFlat(), CAdvancementDeltas(
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
            CVector3f::Zero()
            , CQuaternion::NoRotation()
#endif
            ));

  }

  mCurTime -= dt;
  CCharAnimTime remainingTime = CCharAnimTime::ZeroFlat();
  if (mCurTime < CCharAnimTime()) {
    remainingTime = CCharAnimTime() - mCurTime;
    mCurTime = CCharAnimTime();
  }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const CSegId root = CSegId::Root();
#else
  const CSegId root = CSegId(0);
#endif
  const CQuaternion oldRotation = mSource->GetRotation(root, previousTime);
  const CQuaternion newRotation = mSource->GetRotation(root, mCurTime);
  const CQuaternion inverseOldRotation = oldRotation.BuildInverted();
  CVector3f offset(0.f, 0.f, 0.f);
  if (mSource->HasOffset(root)) {
    const CVector3f oldOffset = mSource->GetOffset(root, previousTime);
    const CVector3f newOffset = mSource->GetOffset(root, mCurTime);
    offset = newOffset - oldOffset;
    const CQuaternion inverseNewRotation = newRotation.BuildInverted();
    const CMatrix3f inverseRotation = inverseNewRotation.BuildTransform();
    offset = inverseRotation * offset;
  }
  return SAdvancementResults(remainingTime,
                             CAdvancementDeltas(offset, newRotation * inverseOldRotation));

}

void CAnimSourceReader::VSetPhase(float phase) {
#if RSTL_PRIME1
  mCurTime = CCharAnimTime(
      phase * mSource->GetSteadyStateAnimInfo(mCurTime).GetDuration().GetSeconds());
  if (mSource->HasPOIData())
#else
  const CCharAnimTime duration = mSource->GetSteadyStateAnimInfo(mCurTime).GetDuration();
  mCurTime = CCharAnimTime(phase * duration.GetSeconds());
#endif
  {
    UpdatePOIStates();
    if (!mCurTime.GreaterThanZero()) {
      mPassedBoolCount = 0;
      mPassedIntCount = 0;
      mPassedParticleCount = 0;
      mPassedSoundCount = 0;
    }
  }
}

bool CAnimSourceReader::VSupportsReverseView() const { return true; }

SAdvancementResults
CAnimSourceReader::VGetAdvancementResults(const CCharAnimTime& dt,
                                          const CCharAnimTime& startOffset) const {
  const CCharAnimTime previousTime = mCurTime + startOffset;
  CCharAnimTime currentTime = mCurTime + startOffset;
  const CCharAnimTime& duration = mSource->GetAnimationDuration();
  if (previousTime >= duration) {
    return SAdvancementResults(dt, CAdvancementDeltas(
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
            CVector3f::Zero()
            , CQuaternion::NoRotation()
#endif
            ));

  }
  if (dt.EqualsZero()) {
    return SAdvancementResults(CCharAnimTime::ZeroFlat(), CAdvancementDeltas(
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
            CVector3f::Zero()
            , CQuaternion::NoRotation()
#endif
            ));

  }

  currentTime += dt;
  CCharAnimTime remainingTime = CCharAnimTime::ZeroFlat();
  if (currentTime > duration) {
    remainingTime = currentTime - duration;
    currentTime = duration;
  }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const CSegId root = CSegId::Root();
#else
  const CSegId root = CSegId(0);
#endif
  CVector3f offset(0.f, 0.f, 0.f);
  const CQuaternion oldRotation = mSource->GetRotation(root, previousTime);
  const CQuaternion newRotation = mSource->GetRotation(root, currentTime);
  const CQuaternion inverseOldRotation = oldRotation.BuildInverted();
  if (mSource->HasOffset(root)) {
    const CVector3f oldOffset = mSource->GetOffset(root, previousTime);
    const CVector3f newOffset = mSource->GetOffset(root, currentTime);
    offset = newOffset - oldOffset;
    const CQuaternion inverseNewRotation = newRotation.BuildInverted();
    const CMatrix3f inverseRotation = inverseNewRotation.BuildTransform();
    offset = inverseRotation * offset;
  }
  return SAdvancementResults(remainingTime,
                             CAdvancementDeltas(offset, newRotation * inverseOldRotation));

}

#if !RSTL_PRIME1
CAnimSourceReader::~CAnimSourceReader() {}
#endif

