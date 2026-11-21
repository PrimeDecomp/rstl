#include "rstl/RstlVersions.h"
#include "rstl/RstlVersions.h"
#include "rstl/macros.hpp"
#include "Kyoto/Animation/CAnimSourceReaderBase.hpp"

#if RSTL_CORRUPTION
#include "Kyoto/Animation/CSegIdMaskMap.hpp"
#else
#include "Kyoto/Animation/CBoolPOINode.hpp"
#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Animation/CParticlePOINode.hpp"
#include "Kyoto/Animation/CSoundPOINode.hpp"
#include "Kyoto/Animation/CAnimPOIData.hpp"
#endif

#if RSTL_CORRUPTION
#include "Kyoto/Animation/CAnimPOIData.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#endif
#include "rstl/math.hpp"

template < class T >
#if RSTL_CORRUPTION
uint _getPOIList(const CCharAnimTime& time, T* listOut, uint capacity, uint iterator, const CSegIdMaskMap& mask,
#else
uint _getPOIList(const CCharAnimTime& time, T* listOut, uint capacity, uint iterator, int additive,
#endif
                 const rstl::vector< T >& stream, const CCharAnimTime& curTime,
                 const IAnimSourceInfo& sourceInfo, int passedCount) {
  uint ret = 0;
#if !RSTL_CORRUPTION
#if RSTL_PRIME1
  if (sourceInfo.HasPOIData())
#endif
  {
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
#if RSTL_CORRUPTION
      CCharAnimTime nodeTime(stream[passedCount].GetTime());
#endif
        int index = passedCount;
#if RSTL_CORRUPTION
      while (index < count &&
             (nodeTime <= endTime || close_enough(nodeTime.GetSeconds(), endTime.GetSeconds()))) {
        if (nodeTime >= curTime) {
#else
        const int initialIndex = index;
        CCharAnimTime nodeTime(stream[initialIndex].GetTime());
        while (index < count && nodeTime <= endTime) {
#endif
          const T& node = stream[index];
#if RSTL_CORRUPTION
          if ((node.GetUnk2C() & mask.GetMask(node.GetSegId())) && ret + iterator < capacity) {
#else
          if (ret + iterator < capacity) {
#endif
            listOut[iterator + ret] = T::CopyNodeMinusStartTime(node, curTime);
            ++ret;
#if RSTL_CORRUPTION
          }
#endif
          }
          ++index;
          if (index < count) {
            nodeTime = stream[index].GetTime();
          }
        }
      }
    }
#if !RSTL_CORRUPTION
  }
#endif
  return ret;
}

uint CAnimSourceReaderBase::VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut,
#if RSTL_CORRUPTION
                                            uint capacity, uint iterator, const CSegIdMaskMap& mask) const {
  return _getPOIList(time, listOut, capacity, iterator, mask, mPOIData->GetBoolPOIStream(),
                     mCurTime, *mSourceInfo, mPassedBoolCount);
#else
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

#endif
}

uint CAnimSourceReaderBase::VGetParticlePOIList(const CCharAnimTime& time,
                                                CParticlePOINode* listOut, uint capacity,
#if RSTL_CORRUPTION
                                                uint iterator, const CSegIdMaskMap& mask) const {
  return _getPOIList(time, listOut, capacity, iterator, mask, mPOIData->GetParticlePOIStream(),
                     mCurTime, *mSourceInfo, mPassedParticleCount);
#else
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

#endif
}

uint CAnimSourceReaderBase::VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut,
#if RSTL_CORRUPTION
                                             uint capacity, uint iterator, const CSegIdMaskMap& mask) const {
  return _getPOIList(time, listOut, capacity, iterator, mask, mPOIData->GetSoundPOIStream(),
                     mCurTime, *mSourceInfo, mPassedSoundCount);
#else
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

#endif
}

#if RSTL_CORRUPTION
uint CAnimSourceReaderBase::VGetAssetSoundPOIList(const CCharAnimTime& time,
                                                  CAssetSoundPOINode* listOut, uint capacity,
                                                  uint iterator, const CSegIdMaskMap& mask) const {
  return _getPOIList(time, listOut, capacity, iterator, mask,
                     mPOIData->GetAssetSoundPOIStream(), mCurTime, *mSourceInfo,
                     mPassedAssetSoundCount);
}
#endif
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_CORRUPTION
uint CAnimSourceReaderBase::VGetAssetPOIList(const CCharAnimTime& time, CAssetPOINode* listOut,
                                             uint capacity, uint iterator, const CSegIdMaskMap& mask) const {
  return _getPOIList(time, listOut, capacity, iterator, mask, mPOIData->GetAssetPOIStream(),
                     mCurTime, *mSourceInfo, mPassedAssetCount);
}
#else
#if RSTL_PRIME1
bool CAnimSourceReaderBase::VGetBoolPOIState(const char* name) const {
#else
bool CAnimSourceReaderBase::VGetBoolPOIState(uint name) const {
#endif
#endif
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_CORRUPTION
bool CAnimSourceReaderBase::VGetBoolPOIState(uint nameHash) const {
  for (int i = 0; i < mBoolStates.size(); ++i) {
    if (mBoolStates[i].first == nameHash) {
#else
#if RSTL_PRIME1
  int count = mBoolStates.size();
  for (int i = 0; i < count; ++i)
#else
  for (int i = 0; i < mBoolStates.size(); ++i)
#endif
  {
    if (mBoolStates[i].first == name) {
#endif
      return mBoolStates[i].second;
    }
  }
  return false;
}

#if RSTL_CORRUPTION
CParticleData::EParentedMode CAnimSourceReaderBase::VGetParticlePOIState(uint nameHash) const {
  for (int i = 0; i < mParticleStates.size(); ++i) {
    if (mParticleStates[i].first == nameHash) {
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
s32 CAnimSourceReaderBase::VGetInt32POIState(const char* name) const {
#else
s32 CAnimSourceReaderBase::VGetInt32POIState(uint name) const {
#endif
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  int count = mInt32States.size();
  for (int i = 0; i < count; ++i)
#else
  for (int i = 0; i < mInt32States.size(); ++i)
#endif
  {
    if (mInt32States[i].first == name) {
      return mInt32States[i].second;
    }
  }
  return 0;
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CParticleData::EParentedMode CAnimSourceReaderBase::VGetParticlePOIState(const char* name) const {
#else
CParticleData::EParentedMode CAnimSourceReaderBase::VGetParticlePOIState(uint name) const {
#endif
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  int count = mParticleStates.size();
  for (int i = 0; i < count; ++i)
#else
  for (int i = 0; i < mParticleStates.size(); ++i)
#endif
  {
    if (mParticleStates[i].first == name) {
#endif
      return mParticleStates[i].second;
    }
  }
  return CParticleData::kPM_Initial;
}

void CAnimSourceReaderBase::UpdatePOIStates() {
#if RSTL_CORRUPTION
  const rstl::vector< CBoolPOINode >& boolNodes = mPOIData->GetBoolPOIStream();
  const rstl::vector< CParticlePOINode >& particleNodes = mPOIData->GetParticlePOIStream();
  const rstl::vector< CSoundPOINode >& soundNodes = mPOIData->GetSoundPOIStream();
  const rstl::vector< CAssetSoundPOINode >& assetSoundNodes = mPOIData->GetAssetSoundPOIStream();
  const rstl::vector< CAssetPOINode >& assetNodes = mPOIData->GetAssetPOIStream();
#else
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

#endif
  int boolCount = boolNodes.size();
#if !RSTL_CORRUPTION
  int int32Count = int32Nodes.size();
#endif
  int particleCount = particleNodes.size();
  int soundCount = soundNodes.size();
#if RSTL_CORRUPTION
  int assetSoundCount = assetSoundNodes.size();
  int assetCount = assetNodes.size();
#endif
  while (mPassedBoolCount < boolCount && boolNodes[mPassedBoolCount].GetTime() <= mCurTime) {
    const CBoolPOINode& node = boolNodes[mPassedBoolCount];
    int index = node.GetIndex();
    if (index >= 0
#if !RSTL_PRIME1
        && index < mBoolStates.size()
#endif
    ) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      mBoolStates[index] =
          rstl::pair< rstl::string, bool >(mBoolStates[index].first, node.GetValue());
#else
      mBoolStates[index] = rstl::pair< uint, bool >(mBoolStates[index].first, node.GetValue());
#endif
    }
    ++mPassedBoolCount;
  }
#if !RSTL_CORRUPTION
  while (mPassedIntCount < int32Count && int32Nodes[mPassedIntCount].GetTime() <= mCurTime) {
    const CInt32POINode& node = int32Nodes[mPassedIntCount];
    int index = node.GetIndex();
    if (index >= 0
#if !RSTL_PRIME1
        && index < mInt32States.size()
#endif
    ) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      mInt32States[index] =
          rstl::pair< rstl::string, int >(mInt32States[index].first, node.GetValue());
#else
      mInt32States[index] = rstl::pair< uint, int >(mInt32States[index].first, node.GetValue());
#endif
    }
    ++mPassedIntCount;
  }
#endif
  while (mPassedParticleCount < particleCount &&
#if RSTL_CORRUPTION
         (particleNodes[mPassedParticleCount].GetTime() <= mCurTime ||
          close_enough(particleNodes[mPassedParticleCount].GetTime().GetSeconds(),
                       mCurTime.GetSeconds()))) {
#else
         particleNodes[mPassedParticleCount].GetTime() <= mCurTime) {
#endif
    const CParticlePOINode& node = particleNodes[mPassedParticleCount];
    int index = node.GetIndex();
#if RSTL_CORRUPTION
    if (index >= 0 && index < mParticleStates.size()) {
      mParticleStates[index] = rstl::pair< uint, CParticleData::EParentedMode >(
          mParticleStates[index].first, node.GetParentedMode());
#else
    if (index >= 0
#if !RSTL_PRIME1
        && index < mParticleStates.size()
#endif
    ) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      mParticleStates[index] = rstl::pair< rstl::string, CParticleData::EParentedMode >(
#else
      mParticleStates[index] = rstl::pair< uint, CParticleData::EParentedMode >(
#endif
          mParticleStates[index].first, node.GetParticleData().GetParentedMode());
#endif
    }
    ++mPassedParticleCount;
  }
#if RSTL_CORRUPTION
  while (
      mPassedSoundCount < soundCount &&
      (soundNodes[mPassedSoundCount].GetTime() <= mCurTime ||
       close_enough(soundNodes[mPassedSoundCount].GetTime().GetSeconds(), mCurTime.GetSeconds()))) {
#else
  while (mPassedSoundCount < soundCount && soundNodes[mPassedSoundCount].GetTime() <= mCurTime) {
#endif
    ++mPassedSoundCount;
#if RSTL_CORRUPTION
  }
  while (mPassedAssetSoundCount < assetSoundCount &&
         (assetSoundNodes[mPassedAssetSoundCount].GetTime() <= mCurTime ||
          close_enough(assetSoundNodes[mPassedAssetSoundCount].GetTime().GetSeconds(),
                       mCurTime.GetSeconds()))) {
    ++mPassedAssetSoundCount;
  }
  while (
      mPassedAssetCount < assetCount &&
      (assetNodes[mPassedAssetCount].GetTime() <= mCurTime ||
       close_enough(assetNodes[mPassedAssetCount].GetTime().GetSeconds(), mCurTime.GetSeconds()))) {
    ++mPassedAssetCount;
#endif
  }
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
rstl::set< rstl::pair< rstl::string, int > > CAnimSourceReaderBase::GetUniqueBoolPOIs() const {
  const rstl::vector< CBoolPOINode >& nodes = mSourceInfo->GetBoolPOIStream();
  int count = nodes.size();
  rstl::set< rstl::pair< rstl::string, int > > result;
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
      result.insert(rstl::pair< rstl::string, int >(node.GetString(), node.GetIndex()));
    }
#else
    if (nodes[i].GetSaveState()) {
      result.insert(rstl::pair< uint, int >(nodes[i].GetNameHash(), nodes[i].GetIndex()));
    }
#endif
  }
  return result;
}

#if RSTL_CORRUPTION
rstl::set< rstl::pair< uint, int > > CAnimSourceReaderBase::GetUniqueParticlePOIs() const {
  const rstl::vector< CParticlePOINode >& nodes = mPOIData->GetParticlePOIStream();
  const int count = nodes.size();
  rstl::set< rstl::pair< uint, int > > result;
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
rstl::set< rstl::pair< rstl::string, int > > CAnimSourceReaderBase::GetUniqueInt32POIs() const {
  const rstl::vector< CInt32POINode >& nodes = mSourceInfo->GetInt32POIStream();
  int count = nodes.size();
  rstl::set< rstl::pair< rstl::string, int > > result;
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
      result.insert(rstl::pair< rstl::string, int >(node.GetString(), node.GetIndex()));
    }
#else
    if (nodes[i].GetSaveState()) {
      result.insert(rstl::pair< uint, int >(nodes[i].GetNameHash(), nodes[i].GetIndex()));
    }
#endif
  }
  return result;
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
rstl::set< rstl::pair< rstl::string, int > > CAnimSourceReaderBase::GetUniqueParticlePOIs() const {
  const rstl::vector< CParticlePOINode >& nodes = mSourceInfo->GetParticlePOIStream();
  int count = nodes.size();
  rstl::set< rstl::pair< rstl::string, int > > result;
#else
rstl::set< rstl::pair< uint, int > > CAnimSourceReaderBase::GetUniqueParticlePOIs() const {
  const rstl::vector< CParticlePOINode >& nodes = mPOIData->GetParticlePOIStream();
  const int count = nodes.size();
  rstl::set< rstl::pair< uint, int > > result;
#endif
#endif
  for (int i = 0; i < count; ++i) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    const CParticlePOINode& node = nodes[i];
    if (node.GetSaveState()) {
      result.insert(rstl::pair< rstl::string, int >(node.GetString(), node.GetIndex()));
    }
#else
    if (nodes[i].GetSaveState()) {
      result.insert(rstl::pair< uint, int >(nodes[i].GetNameHash(), nodes[i].GetIndex()));
    }
#endif
  }
  return result;
}

void CAnimSourceReaderBase::PostConstruct(const CCharAnimTime& time) {
  mPassedBoolCount = 0;
#if !RSTL_CORRUPTION
  mPassedIntCount = 0;
#endif
  mPassedParticleCount = 0;
  mPassedSoundCount = 0;
#if RSTL_CORRUPTION
  mPassedAssetSoundCount = 0;
  mPassedAssetCount = 0;
  const rstl::set< rstl::pair< uint, int > > boolPOIs = GetUniqueBoolPOIs();
  const rstl::set< rstl::pair< uint, int > > particlePOIs = GetUniqueParticlePOIs();
#else
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
#endif
    int boolCount = boolPOIs.size();
#if !RSTL_CORRUPTION
    int int32Count = int32POIs.size();
#endif
    int particleCount = particlePOIs.size();
#if RSTL_CORRUPTION
    mBoolStates.resize(boolCount, rstl::pair< uint, bool >(CPOINode::skInvalidNameHash, false));
  mParticleStates.resize(particleCount, rstl::pair< uint, CParticleData::EParentedMode >(
                                            CPOINode::skInvalidNameHash, CParticleData::kPM_Initial));
  for (rstl::set< rstl::pair< uint, int > >::const_iterator it = boolPOIs.begin();
       it != boolPOIs.end();) {
      uint name = it->first;
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    mBoolStates.resize(boolCount, rstl::pair< rstl::string, bool >(rstl::string_l(""), false));
    mInt32States.resize(int32Count, rstl::pair< rstl::string, int >(rstl::string_l(""), 0));
    mParticleStates.resize(particleCount,
                           rstl::pair< rstl::string, CParticleData::EParentedMode >(
                               rstl::string_l(""), CParticleData::kPM_Initial));
#else
    mBoolStates.resize(boolCount, rstl::pair< uint, bool >(CPOINode::skInvalidNameHash, false));
    mInt32States.resize(int32Count, rstl::pair< uint, int >(CPOINode::skInvalidNameHash, 0));
    mParticleStates.resize(
        particleCount,
        rstl::pair< uint, CParticleData::EParentedMode >(CPOINode::skInvalidNameHash,
                                                         CParticleData::kPM_Initial));
#endif
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    for (AUTO(it, boolPOIs.begin()); it != boolPOIs.end();)
#else
    for (rstl::set< rstl::pair< uint, int > >::const_iterator it = boolPOIs.begin(),
                                                              end = boolPOIs.end();
         it != end;)
#endif
    {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      rstl::string name = it->first;
#else
      uint name = it->first;
#endif
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
#if RSTL_CORRUPTION
  for (rstl::set< rstl::pair< uint, int > >::const_iterator it = particlePOIs.begin();
       it != particlePOIs.end();) {
      uint name = it->first;
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    for (AUTO(it, int32POIs.begin()); it != int32POIs.end();)
#else
    for (rstl::set< rstl::pair< uint, int > >::const_iterator it = int32POIs.begin(),
                                                              end = int32POIs.end();
         it != end;)
#endif
    {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      rstl::string name = it->first;
#else
      uint name = it->first;
#endif
#endif
      int index = it->second;
#if RSTL_CORRUPTION
      if (index >= 0 && index < mParticleStates.size()) {
        mParticleStates[index] =
            rstl::pair< uint, CParticleData::EParentedMode >(name, CParticleData::kPM_Initial);
      }
#else
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
    for (AUTO(it, particlePOIs.begin()); it != particlePOIs.end();)
#else
    for (rstl::set< rstl::pair< uint, int > >::const_iterator it = particlePOIs.begin(),
                                                              end = particlePOIs.end();
         it != end;)
#endif
    {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      rstl::string name = it->first;
#else
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
#endif
      ++it;
    }
#if RSTL_PRIME1
  }
#endif
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
#if !RSTL_CORRUPTION
      mPassedIntCount = 0;
#endif
      mPassedParticleCount = 0;
      mPassedSoundCount = 0;
#if RSTL_CORRUPTION
      mPassedAssetSoundCount = 0;
      mPassedAssetCount = 0;
#endif
    }
  }
}
