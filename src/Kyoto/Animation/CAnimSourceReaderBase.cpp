#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CAnimSourceReaderBase.hpp"

#include "Kyoto/Animation/CBoolPOINode.hpp"
#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Animation/CParticlePOINode.hpp"
#include "Kyoto/Animation/CSoundPOINode.hpp"
#include "Kyoto/Animation/CAnimPOIData.hpp"

#include "rstl/math.hpp"

template < class T >
uint _getPOIList(const CCharAnimTime& time, T* listOut, uint capacity, uint iterator, int additive,
                 const rstl::vector< T >& stream, const CCharAnimTime& curTime,
                 const IAnimSourceInfo& sourceInfo, int passedCount) {
  uint ret = 0;
#if RSTL_PRIME1
  if (sourceInfo.HasPOIData()) {
#endif

    int count = stream.size();
    if (count > 0) {
      const CCharAnimTime& duration = sourceInfo.GetAnimationDuration();
      CCharAnimTime totalTime = curTime + time;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      CCharAnimTime endTime = rstl::min_val(duration, totalTime);
#else
    CCharAnimTime endTime = rstl::min_val< const CCharAnimTime& >(duration, totalTime);
#endif

      if (passedCount < count) {
        int index = passedCount;
        const int initialIndex = index;
        CCharAnimTime nodeTime(stream[initialIndex].GetTime());
        while (index < count && nodeTime <= endTime) {
          const T& node = stream[index];
          if (ret + iterator < capacity) {
            listOut[iterator + ret] = T::CopyNodeMinusStartTime(node, curTime);
            ++ret;
          }
          ++index;
          if (index < count) {
            nodeTime = stream[index].GetTime();
          }
        }
      }
    }
#if RSTL_PRIME1
  }
#endif

  return ret;
}

uint CAnimSourceReaderBase::VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut,
                                            uint capacity, uint iterator, int additive) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (mSourceInfo->HasPOIData()) {
    const IAnimSourceInfo& sourceInfo = AnimSource();
    const rstl::vector< CBoolPOINode >& stream = sourceInfo.GetBoolPOIStream();
    return _getPOIList(time, listOut, capacity, iterator, additive, stream, mCurTime, sourceInfo,
                       mPassedBoolCount);
  }
  return 0;
#else
  return _getPOIList(time, listOut, capacity, iterator, additive, mPOIData->GetBoolPOIStream(),
                     mCurTime, *mSourceInfo, mPassedBoolCount);
#endif

}

uint CAnimSourceReaderBase::VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut,
                                             uint capacity, uint iterator, int additive) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (mSourceInfo->HasPOIData()) {
    const IAnimSourceInfo& sourceInfo = AnimSource();
    const rstl::vector< CInt32POINode >& stream = sourceInfo.GetInt32POIStream();
    return _getPOIList(time, listOut, capacity, iterator, additive, stream, mCurTime, sourceInfo,
                       mPassedIntCount);
  }
  return 0;
#else
  return _getPOIList(time, listOut, capacity, iterator, additive, mPOIData->GetInt32POIStream(),
                     mCurTime, *mSourceInfo, mPassedIntCount);
#endif

}

uint CAnimSourceReaderBase::VGetParticlePOIList(const CCharAnimTime& time,
                                                CParticlePOINode* listOut, uint capacity,
                                                uint iterator, int additive) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (mSourceInfo->HasPOIData()) {
    const IAnimSourceInfo& sourceInfo = AnimSource();
    const rstl::vector< CParticlePOINode >& stream = sourceInfo.GetParticlePOIStream();
    return _getPOIList(time, listOut, capacity, iterator, additive, stream, mCurTime, sourceInfo,
                       mPassedParticleCount);
  }
  return 0;
#else
  return _getPOIList(time, listOut, capacity, iterator, additive, mPOIData->GetParticlePOIStream(),
                     mCurTime, *mSourceInfo, mPassedParticleCount);
#endif

}

uint CAnimSourceReaderBase::VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut,
                                             uint capacity, uint iterator, int additive) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (mSourceInfo->HasPOIData()) {
    const IAnimSourceInfo& sourceInfo = AnimSource();
    const rstl::vector< CSoundPOINode >& stream = sourceInfo.GetSoundPOIStream();
    return _getPOIList(time, listOut, capacity, iterator, additive, stream, mCurTime, sourceInfo,
                       mPassedSoundCount);
  }
  return 0;
#else
  return _getPOIList(time, listOut, capacity, iterator, additive, mPOIData->GetSoundPOIStream(),
                     mCurTime, *mSourceInfo, mPassedSoundCount);
#endif

}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
bool CAnimSourceReaderBase::VGetBoolPOIState(const char* name) const {
  int count = mBoolStates.size();
  for (int i = 0; i < count; ++i) {
    if (mBoolStates[i].first == name) {
#else
bool CAnimSourceReaderBase::VGetBoolPOIState(uint nameHash) const {
  for (int i = 0; i < mBoolStates.size(); ++i) {
    if (mBoolStates[i].first == nameHash) {
#endif

      return mBoolStates[i].second;
    }
  }
  return false;
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
s32 CAnimSourceReaderBase::VGetInt32POIState(const char* name) const {
  int count = mInt32States.size();
  for (int i = 0; i < count; ++i) {
    if (mInt32States[i].first == name) {
#else
s32 CAnimSourceReaderBase::VGetInt32POIState(uint nameHash) const {
  for (int i = 0; i < mInt32States.size(); ++i) {
    if (mInt32States[i].first == nameHash) {
#endif

      return mInt32States[i].second;
    }
  }
  return 0;
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CParticleData::EParentedMode CAnimSourceReaderBase::VGetParticlePOIState(const char* name) const {
  int count = mParticleStates.size();
  for (int i = 0; i < count; ++i) {
    if (mParticleStates[i].first == name) {
#else
CParticleData::EParentedMode CAnimSourceReaderBase::VGetParticlePOIState(uint nameHash) const {
  for (int i = 0; i < mParticleStates.size(); ++i) {
    if (mParticleStates[i].first == nameHash) {
#endif

      return mParticleStates[i].second;
    }
  }
  return CParticleData::kPM_Initial;
}

void CAnimSourceReaderBase::UpdatePOIStates() {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const IAnimSourceInfo& sourceInfo = AnimSource();
  const rstl::vector< CBoolPOINode >& boolNodes = sourceInfo.GetBoolPOIStream();
  const rstl::vector< CInt32POINode >& int32Nodes = sourceInfo.GetInt32POIStream();
  const rstl::vector< CParticlePOINode >& particleNodes = sourceInfo.GetParticlePOIStream();
  const rstl::vector< CSoundPOINode >& soundNodes = sourceInfo.GetSoundPOIStream();
#else
  const rstl::vector< CBoolPOINode >& boolNodes = mPOIData->GetBoolPOIStream();
  const rstl::vector< CInt32POINode >& int32Nodes = mPOIData->GetInt32POIStream();
  const rstl::vector< CParticlePOINode >& particleNodes = mPOIData->GetParticlePOIStream();
  const rstl::vector< CSoundPOINode >& soundNodes = mPOIData->GetSoundPOIStream();
#endif

  int boolCount = boolNodes.size();
  int int32Count = int32Nodes.size();
  int particleCount = particleNodes.size();
  int soundCount = soundNodes.size();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  while (mPassedBoolCount < boolCount &&
         boolNodes[mPassedBoolCount].GetTime() <= mCurTime) {
#else
  while (mPassedBoolCount < boolCount && boolNodes[mPassedBoolCount].GetTime() <= mCurTime) {
#endif

    const CBoolPOINode& node = boolNodes[mPassedBoolCount];
    int index = node.GetIndex();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    if (index >= 0) {
      mBoolStates[index] =
          rstl::pair< rstl::string, bool >(mBoolStates[index].first, node.GetValue());
#else
    if (index >= 0 && index < mBoolStates.size()) {
      mBoolStates[index] = rstl::pair< uint, bool >(mBoolStates[index].first, node.GetValue());
#endif

    }
    ++mPassedBoolCount;
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  while (mPassedIntCount < int32Count &&
         int32Nodes[mPassedIntCount].GetTime() <= mCurTime) {
#else
  while (mPassedIntCount < int32Count && int32Nodes[mPassedIntCount].GetTime() <= mCurTime) {
#endif

    const CInt32POINode& node = int32Nodes[mPassedIntCount];
    int index = node.GetIndex();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    if (index >= 0) {
      mInt32States[index] =
          rstl::pair< rstl::string, int >(mInt32States[index].first, node.GetValue());
#else
    if (index >= 0 && index < mInt32States.size()) {
      mInt32States[index] = rstl::pair< uint, int >(mInt32States[index].first, node.GetValue());
#endif

    }
    ++mPassedIntCount;
  }
  while (mPassedParticleCount < particleCount &&
         particleNodes[mPassedParticleCount].GetTime() <= mCurTime) {
    const CParticlePOINode& node = particleNodes[mPassedParticleCount];
    int index = node.GetIndex();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    if (index >= 0) {
      mParticleStates[index] = rstl::pair< rstl::string, CParticleData::EParentedMode >(
#else
    if (index >= 0 && index < mParticleStates.size()) {
      mParticleStates[index] = rstl::pair< uint, CParticleData::EParentedMode >(
#endif

          mParticleStates[index].first, node.GetParticleData().GetParentedMode());
    }
    ++mPassedParticleCount;
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  while (mPassedSoundCount < soundCount &&
         soundNodes[mPassedSoundCount].GetTime() <= mCurTime) {
#else
  while (mPassedSoundCount < soundCount && soundNodes[mPassedSoundCount].GetTime() <= mCurTime) {
#endif

    ++mPassedSoundCount;
  }
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
rstl::set< rstl::pair< rstl::string, int > > CAnimSourceReaderBase::GetUniqueBoolPOIs() const {
  const rstl::vector< CBoolPOINode >& nodes = mSourceInfo->GetBoolPOIStream();
  int count = nodes.size();
  rstl::set< rstl::pair< rstl::string, int > > ret;
#else
rstl::set< rstl::pair< uint, int > > CAnimSourceReaderBase::GetUniqueBoolPOIs() const {
  const rstl::vector< CBoolPOINode >& nodes = mPOIData->GetBoolPOIStream();
  const int count = nodes.size();
  rstl::set< rstl::pair< uint, int > > result;
#endif

  for (int i = 0; i < count; ++i) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    const CBoolPOINode& node = nodes[i];
    if (node.GetSaveState()) {
      ret.insert(rstl::pair< rstl::string, int >(node.GetString(), node.GetIndex()));
#else
    if (nodes[i].GetSaveState()) {
      result.insert(rstl::pair< uint, int >(nodes[i].GetNameHash(), nodes[i].GetIndex()));
#endif

    }
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return ret;
#else
  return result;
#endif

}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
rstl::set< rstl::pair< rstl::string, int > > CAnimSourceReaderBase::GetUniqueInt32POIs() const {
  const rstl::vector< CInt32POINode >& nodes = mSourceInfo->GetInt32POIStream();
  int count = nodes.size();
  rstl::set< rstl::pair< rstl::string, int > > ret;
#else
rstl::set< rstl::pair< uint, int > > CAnimSourceReaderBase::GetUniqueInt32POIs() const {
  const rstl::vector< CInt32POINode >& nodes = mPOIData->GetInt32POIStream();
  const int count = nodes.size();
  rstl::set< rstl::pair< uint, int > > result;
#endif

  for (int i = 0; i < count; ++i) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    const CInt32POINode& node = nodes[i];
    if (node.GetSaveState()) {
      ret.insert(rstl::pair< rstl::string, int >(node.GetString(), node.GetIndex()));
#else
    if (nodes[i].GetSaveState()) {
      result.insert(rstl::pair< uint, int >(nodes[i].GetNameHash(), nodes[i].GetIndex()));
#endif

    }
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return ret;
#else
  return result;
#endif

}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
rstl::set< rstl::pair< rstl::string, int > > CAnimSourceReaderBase::GetUniqueParticlePOIs() const {
  const rstl::vector< CParticlePOINode >& nodes = mSourceInfo->GetParticlePOIStream();
  int count = nodes.size();
  rstl::set< rstl::pair< rstl::string, int > > ret;
#else
rstl::set< rstl::pair< uint, int > > CAnimSourceReaderBase::GetUniqueParticlePOIs() const {
  const rstl::vector< CParticlePOINode >& nodes = mPOIData->GetParticlePOIStream();
  const int count = nodes.size();
  rstl::set< rstl::pair< uint, int > > result;
#endif

  for (int i = 0; i < count; ++i) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    const CParticlePOINode& node = nodes[i];
    if (node.GetSaveState()) {
      ret.insert(rstl::pair< rstl::string, int >(node.GetString(), node.GetIndex()));
#else
    if (nodes[i].GetSaveState()) {
      result.insert(rstl::pair< uint, int >(nodes[i].GetNameHash(), nodes[i].GetIndex()));
#endif

    }
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return ret;
#else
  return result;
#endif

}

void CAnimSourceReaderBase::PostConstruct(const CCharAnimTime& time) {
  mPassedBoolCount = 0;
  mPassedIntCount = 0;
  mPassedParticleCount = 0;
  mPassedSoundCount = 0;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const IAnimSourceInfo& sourceInfo = AnimSource();
  if (sourceInfo.HasPOIData()) {
    const rstl::set< rstl::pair< rstl::string, int > > boolPOIs = GetUniqueBoolPOIs();
    const rstl::set< rstl::pair< rstl::string, int > > int32POIs = GetUniqueInt32POIs();
    const rstl::set< rstl::pair< rstl::string, int > > particlePOIs = GetUniqueParticlePOIs();
#else
  const rstl::set< rstl::pair< uint, int > > boolPOIs = GetUniqueBoolPOIs();
  const rstl::set< rstl::pair< uint, int > > int32POIs = GetUniqueInt32POIs();
  const rstl::set< rstl::pair< uint, int > > particlePOIs = GetUniqueParticlePOIs();
#endif

    int boolCount = boolPOIs.size();
    int int32Count = int32POIs.size();
    int particleCount = particlePOIs.size();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    mBoolStates.resize(boolCount, rstl::pair< rstl::string, bool >(rstl::string_l(""), false));
    mInt32States.resize(int32Count, rstl::pair< rstl::string, int >(rstl::string_l(""), 0));
    mParticleStates.resize(particleCount,
                              rstl::pair< rstl::string, CParticleData::EParentedMode >(
                                  rstl::string_l(""), CParticleData::kPM_Initial));
    for (AUTO(it, boolPOIs.begin()); it != boolPOIs.end();) {
      rstl::string name = it->first;
#else
  mBoolStates.resize(boolCount, rstl::pair< uint, bool >(CPOINode::skInvalidNameHash, false));
  mInt32States.resize(int32Count, rstl::pair< uint, int >(CPOINode::skInvalidNameHash, 0));
  mParticleStates.resize(
      particleCount, rstl::pair< uint, CParticleData::EParentedMode >(CPOINode::skInvalidNameHash,
                                                                      CParticleData::kPM_Initial));
  for (rstl::set< rstl::pair< uint, int > >::const_iterator it = boolPOIs.begin(),
                                                            end = boolPOIs.end();
       it != end;) {
    uint name = it->first;
#endif

      int index = it->second;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      mBoolStates[index] = rstl::pair< rstl::string, bool >(name, false);
#else
    if (index >= 0 && index < mBoolStates.size()) {
      mBoolStates[index] = rstl::pair< uint, bool >(name, false);
    }
#endif

      ++it;
    }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    for (AUTO(it, int32POIs.begin()); it != int32POIs.end();) {
      rstl::string name = it->first;
#else
  for (rstl::set< rstl::pair< uint, int > >::const_iterator it = int32POIs.begin(),
                                                            end = int32POIs.end();
       it != end;) {
    uint name = it->first;
#endif

      int index = it->second;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      mInt32States[index] = rstl::pair< rstl::string, int >(name, 0);
#else
    if (index >= 0 && index < mInt32States.size()) {
      mInt32States[index] = rstl::pair< uint, int >(name, 0);
    }
#endif

      ++it;
    }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    for (AUTO(it, particlePOIs.begin()); it != particlePOIs.end();) {
      rstl::string name = it->first;
#else
  for (rstl::set< rstl::pair< uint, int > >::const_iterator it = particlePOIs.begin(),
                                                            end = particlePOIs.end();
       it != end;) {
    uint name = it->first;
#endif

      int index = it->second;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      mParticleStates[index] = rstl::pair< rstl::string, CParticleData::EParentedMode >(
          name, CParticleData::kPM_Initial);
#else
    if (index >= 0 && index < mParticleStates.size()) {
      mParticleStates[index] =
          rstl::pair< uint, CParticleData::EParentedMode >(name, CParticleData::kPM_Initial);
    }
#endif

      ++it;
#if RSTL_PRIME1
    }
#endif

  }
  CCharAnimTime remaining = time;
  if (remaining.GreaterThanZero()) {
    while (remaining.GreaterThanZero()) {
      remaining = VAdvanceView(remaining).GetRemainder();
    }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  } else if (sourceInfo.HasPOIData()) {
#else
  } else {
#endif

    UpdatePOIStates();
    if (!time.GreaterThanZero()) {
      mPassedBoolCount = 0;
      mPassedIntCount = 0;
      mPassedParticleCount = 0;
      mPassedSoundCount = 0;
    }
  }
}

