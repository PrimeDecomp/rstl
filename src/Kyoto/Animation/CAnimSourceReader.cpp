#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CAnimSourceReader.hpp"
#if RSTL_PRIME1
#include "Kyoto/Animation/CCharAnimTime.hpp"
#endif

CAnimSourceReader::CAnimSourceReader(const TSubAnimTypeToken< CAnimSource >& source,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                                     const CCharAnimTime& time)
: CAnimSourceReaderBase(rs_new CAnimSourceInfo(source))
#else
                                     const CCharAnimTime& time, const CAnimPOIData* poiData)
: CAnimSourceReaderBase(rs_new CAnimSourceInfo(source), poiData)
#endif

, mSource(source)
, mSteadyStateInfo(mSource->GetSteadyStateAnimInfo(time)) {
  PostConstruct(time);
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CAdvancementResults CAnimSourceReader::VAdvanceView(const CCharAnimTime& dt) {
#else
SAdvancementResults CAnimSourceReader::VAdvanceView(const CCharAnimTime& dt) {
#endif

  const CCharAnimTime previousTime = mCurTime;
  const CCharAnimTime& duration = mSource->GetAnimationDuration();
  if (previousTime == duration) {
    mCurTime = CCharAnimTime::ZeroFlat();
    mPassedBoolCount = 0;
    mPassedIntCount = 0;
    mPassedParticleCount = 0;
    mPassedSoundCount = 0;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    return CAdvancementResults(dt, CAdvancementDeltas());
#else
    return SAdvancementResults(dt,
                               SAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation()));
#endif

  }
  if (dt.EqualsZero()) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    return CAdvancementResults(CCharAnimTime::ZeroFlat(), CAdvancementDeltas());
#else
    return SAdvancementResults(CCharAnimTime::ZeroFlat(),
                               SAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation()));
#endif

  }

  mCurTime += dt;
  CCharAnimTime remainingTime = CCharAnimTime::ZeroFlat();
  if (mCurTime > duration) {
    remainingTime = mCurTime - duration;
    mCurTime = duration;
  }
#if RSTL_PRIME1
  if (mSource->HasPOIData()) {
#endif

    UpdatePOIStates();
#if RSTL_PRIME1
  }
#endif

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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return CAdvancementResults(remainingTime,
                             CAdvancementDeltas(offset, newRotation * inverseOldRotation));
#else
  return SAdvancementResults(remainingTime,
                             SAdvancementDeltas(offset, newRotation * inverseOldRotation));
#endif

}

CCharAnimTime CAnimSourceReader::VGetTimeRemaining() const {
  return mSource->GetAnimationDuration() - mCurTime;
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CSteadyStateAnimInfo CAnimSourceReader::VGetSteadyStateAnimInfo() const {
  return mSteadyStateInfo;
}
#else
CSteadyStateAnimInfo CAnimSourceReader::VGetSteadyStateAnimInfo() const { return mSteadyStateInfo; }
#endif

bool CAnimSourceReader::VHasOffset(const CSegId& seg) const { return mSource->HasOffset(seg); }

CVector3f CAnimSourceReader::VGetOffset(const CSegId& seg) const {
  return mSource->GetOffset(seg, mCurTime);
}

CVector3f CAnimSourceReader::VGetOffset(const CSegId& seg, const CCharAnimTime& time) const {
  return mSource->GetOffset(seg, time);
}

CQuaternion CAnimSourceReader::VGetRotation(const CSegId& seg) const {
  return mSource->GetRotation(seg, mCurTime);
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CAnimSourceReader::VGetSegStatementSet(const CSegIdList& list,
                                            CSegStatementSet& setOut) const {
  mSource->GetSegStatementSet(list, setOut, mCurTime);
#else
void CAnimSourceReader::VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& set) const {
  mSource->GetSegStatementSet(list, set, mCurTime);
#endif

}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CAnimSourceReader::VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut,
#else
void CAnimSourceReader::VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& set,
#endif

                                            const CCharAnimTime& time) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  mSource->GetSegStatementSet(list, setOut, time);
#else
  mSource->GetSegStatementSet(list, set, time);
}

void CAnimSourceReader::VGetJointData_Linear(const CCharLayoutInfo& layout,
                                             CJointData_LinearStorage& data,
                                             const CCharAnimTime& time) const {
  mSource->GetJointData_Linear(layout, data, time);
}

void CAnimSourceReader::VGetJointData_Linear(const CCharLayoutInfo& layout,
                                             CJointData_LinearStorage& data) const {
  mSource->GetJointData_Linear(layout, data, mCurTime);
#endif

}

rstl::ownership_transfer< IAnimReader > CAnimSourceReader::VClone() const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return rstl::ownership_transfer< IAnimReader >(
      rs_new CAnimSourceReader(mSource, mCurTime, mSteadyStateInfo, mPassedBoolCount,
                               mPassedIntCount, mPassedParticleCount, mPassedSoundCount,
                               mBoolStates, mInt32States, mParticleStates));
#else
  return rstl::ownership_transfer< IAnimReader >(rs_new CAnimSourceReader(
      mSource, mPOIData, mCurTime, mSteadyStateInfo, mPassedBoolCount, mPassedIntCount,
      mPassedParticleCount, mPassedSoundCount, mBoolStates, mInt32States, mParticleStates));
#endif

}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CAdvancementResults CAnimSourceReader::VReverseView(const CCharAnimTime& dt) {
#else
SAdvancementResults CAnimSourceReader::VReverseView(const CCharAnimTime& dt) {
#endif

  const CCharAnimTime previousTime = mCurTime;
  const CCharAnimTime& duration = mSource->GetAnimationDuration();
  if (previousTime.EqualsZero()) {
    mCurTime = duration;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    return CAdvancementResults(dt, CAdvancementDeltas());
#else
    return SAdvancementResults(dt,
                               SAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation()));
#endif

  }
  if (dt.EqualsZero()) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    return CAdvancementResults(CCharAnimTime::ZeroFlat(), CAdvancementDeltas());
#else
    return SAdvancementResults(CCharAnimTime::ZeroFlat(),
                               SAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation()));
#endif

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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return CAdvancementResults(remainingTime,
                             CAdvancementDeltas(offset, newRotation * inverseOldRotation));
#else
  return SAdvancementResults(remainingTime,
                             SAdvancementDeltas(offset, newRotation * inverseOldRotation));
#endif

}

void CAnimSourceReader::VSetPhase(float phase) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  mCurTime = CCharAnimTime(
      phase * mSource->GetSteadyStateAnimInfo(mCurTime).GetDuration().GetSeconds());
  if (mSource->HasPOIData()) {
#else
  const CCharAnimTime duration = mSource->GetSteadyStateAnimInfo(mCurTime).GetDuration();
  mCurTime = CCharAnimTime(phase * duration.GetSeconds());
#endif

    UpdatePOIStates();
    if (!mCurTime.GreaterThanZero()) {
      mPassedBoolCount = 0;
      mPassedIntCount = 0;
      mPassedParticleCount = 0;
      mPassedSoundCount = 0;
    }
  }
#if RSTL_PRIME1
}
#endif

bool CAnimSourceReader::VSupportsReverseView() const { return true; }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CAdvancementResults
#else
SAdvancementResults
#endif

CAnimSourceReader::VGetAdvancementResults(const CCharAnimTime& dt,
                                          const CCharAnimTime& startOffset) const {
  const CCharAnimTime previousTime = mCurTime + startOffset;
  CCharAnimTime currentTime = mCurTime + startOffset;
  const CCharAnimTime& duration = mSource->GetAnimationDuration();
  if (previousTime >= duration) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    return CAdvancementResults(dt, CAdvancementDeltas());
#else
    return SAdvancementResults(dt,
                               SAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation()));
#endif

  }
  if (dt.EqualsZero()) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    return CAdvancementResults(CCharAnimTime::ZeroFlat(), CAdvancementDeltas());
#else
    return SAdvancementResults(CCharAnimTime::ZeroFlat(),
                               SAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation()));
#endif

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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return CAdvancementResults(remainingTime,
                             CAdvancementDeltas(offset, newRotation * inverseOldRotation));
#else
  return SAdvancementResults(remainingTime,
                             SAdvancementDeltas(offset, newRotation * inverseOldRotation));
#endif

}

#if !RSTL_PRIME1
CAnimSourceReader::~CAnimSourceReader() {}
#endif

