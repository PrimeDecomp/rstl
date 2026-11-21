#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/RstlVersions.h"
#endif
#include "Kyoto/Animation/CAnimTreeSingleChild.hpp"
#if !RSTL_CORRUPTION
#include "Kyoto/Math/CQuaternion.hpp"
#endif

SAdvancementResults CAnimTreeSingleChild::VAdvanceView(const CCharAnimTime& time) {
  return mChild->VAdvanceView(time);
}

CCharAnimTime CAnimTreeSingleChild::VGetTimeRemaining() const {
#if RSTL_CORRUPTION
  return mChild->VGetTimeRemaining();
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return mChild->VGetTimeRemaining();
#else
  return mChild->GetTimeRemaining();
#endif
#endif
}

#if RSTL_CORRUPTION
bool CAnimTreeSingleChild::VHasOffset(const CSegId& seg) const { return mChild->VHasOffset(seg); }
#else
bool CAnimTreeSingleChild::VHasOffset(const CSegId& seg) const {
  return mChild->VHasOffset(seg);
}
#endif

CVector3f CAnimTreeSingleChild::VGetOffset(const CSegId& seg) const {
  return mChild->VGetOffset(seg);
}

CQuaternion CAnimTreeSingleChild::VGetRotation(const CSegId& seg) const {
  return mChild->VGetRotation(seg);
}

void CAnimTreeSingleChild::VGetSegStatementSet(const CSegIdList& list,
                                               CSegStatementSet& setOut) const {
#if RSTL_CORRUPTION
  mChild->VGetSegStatementSet(list, setOut);
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return mChild->VGetSegStatementSet(list, setOut);
#else
  mChild->VGetSegStatementSet(list, setOut);
#endif
#endif
}

void CAnimTreeSingleChild::VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut,
                                               const CCharAnimTime& time) const {
#if RSTL_CORRUPTION
  mChild->VGetSegStatementSet(list, setOut, time);
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return mChild->VGetSegStatementSet(list, setOut, time);
#else
  mChild->VGetSegStatementSet(list, setOut, time);
#endif
#endif
}

#if RSTL_CORRUPTION
void CAnimTreeSingleChild::VGetJointData_Linear(const CCharLayoutInfo& layout,
                                                CJointData_LinearStorage& data,
                                                const CCharAnimTime& time) const {
  mChild->VGetJointData_Linear(layout, data, time);
}

void CAnimTreeSingleChild::VGetJointData_Linear(const CCharLayoutInfo& layout,
                                                CJointData_LinearStorage& data) const {
  mChild->VGetJointData_Linear(layout, data);
}
#else
#if !RSTL_PRIME1
void CAnimTreeSingleChild::VGetJointData_Linear(const CCharLayoutInfo& layout,
                                                CJointData_LinearStorage& data,
                                                const CCharAnimTime& time) const {
  mChild->VGetJointData_Linear(layout, data, time);
}

void CAnimTreeSingleChild::VGetJointData_Linear(const CCharLayoutInfo& layout,
                                                CJointData_LinearStorage& data) const {
  mChild->VGetJointData_Linear(layout, data);
}
#endif
#endif

uint CAnimTreeSingleChild::VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut,
#if RSTL_CORRUPTION
                                           uint capacity, uint iterator, const CSegIdMaskMap& mask) const {
  return mChild->GetBoolPOIList(time, listOut, capacity, iterator, mask);
#else
                                           uint capacity, uint iterator, int additive) const {
  return mChild->GetBoolPOIList(time, listOut, capacity, iterator, additive);
}

uint CAnimTreeSingleChild::VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut,
                                            uint capacity, uint iterator, int additive) const {
  return mChild->GetInt32POIList(time, listOut, capacity, iterator, additive);
#endif
}

uint CAnimTreeSingleChild::VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut,
#if RSTL_CORRUPTION
                                               uint capacity, uint iterator, const CSegIdMaskMap& mask) const {
  return mChild->GetParticlePOIList(time, listOut, capacity, iterator, mask);
#else
                                               uint capacity, uint iterator, int additive) const {
  return mChild->GetParticlePOIList(time, listOut, capacity, iterator, additive);
#endif
}

uint CAnimTreeSingleChild::VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut,
#if RSTL_CORRUPTION
                                            uint capacity, uint iterator, const CSegIdMaskMap& mask) const {
  return mChild->GetSoundPOIList(time, listOut, capacity, iterator, mask);
#else
                                            uint capacity, uint iterator, int additive) const {
  return mChild->GetSoundPOIList(time, listOut, capacity, iterator, additive);
#endif
}

#if RSTL_CORRUPTION
uint CAnimTreeSingleChild::VGetAssetSoundPOIList(const CCharAnimTime& time,
                                                 CAssetSoundPOINode* listOut, uint capacity,
                                                 uint iterator, const CSegIdMaskMap& mask) const {
  return mChild->GetAssetSoundPOIList(time, listOut, capacity, iterator, mask);
#else
bool CAnimTreeSingleChild::VGetBoolPOIState(
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    const char* name
#else
    uint name
#endif
    ) const {
  return mChild->VGetBoolPOIState(name);
#endif
}

#if RSTL_CORRUPTION
uint CAnimTreeSingleChild::VGetAssetPOIList(const CCharAnimTime& time, CAssetPOINode* listOut,
                                            uint capacity, uint iterator, const CSegIdMaskMap& mask) const {
  return mChild->GetAssetPOIList(time, listOut, capacity, iterator, mask);
#else
s32 CAnimTreeSingleChild::VGetInt32POIState(
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    const char* name
#else
    uint name
#endif
    ) const {
  return mChild->VGetInt32POIState(name);
#endif
}

#if RSTL_CORRUPTION
bool CAnimTreeSingleChild::VGetBoolPOIState(uint nameHash) const {
  return mChild->VGetBoolPOIState(nameHash);
#else
CParticleData::EParentedMode CAnimTreeSingleChild::VGetParticlePOIState(
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    const char* name
#else
    uint name
#endif
    ) const {
  return mChild->VGetParticlePOIState(name);
#endif
}

#if RSTL_CORRUPTION
CParticleData::EParentedMode CAnimTreeSingleChild::VGetParticlePOIState(uint nameHash) const {
  return mChild->VGetParticlePOIState(nameHash);
}

uint CAnimTreeSingleChild::VGetNumDirectChildren() const { return 1; }

#endif
uint CAnimTreeSingleChild::VGetNumChildren() const { return mChild->VGetNumChildren() + 1; }
#if RSTL_CORRUPTION

rstl::rc_ptr< CAnimTreeNode > CAnimTreeSingleChild::VGetDirectChild(uint index) const {
  if (index == 0) {
    return mChild;
  }
  return rstl::rc_ptr< CAnimTreeNode >(nullptr);
}
#endif

void CAnimTreeSingleChild::VSetPhase(float phase) { mChild->VSetPhase(phase); }

SAdvancementResults
CAnimTreeSingleChild::VGetAdvancementResults(const CCharAnimTime& time,
                                             const CCharAnimTime& startOffset) const {
  return mChild->VGetAdvancementResults(time, startOffset);
}

void CAnimTreeSingleChild::VGetWeightedReaders(
    float weight, rstl::reserved_vector< rstl::pair< float, IAnimReader* >, 16 >& out) const {
#if RSTL_CORRUPTION
  mChild->VGetWeightedReaders(weight, out);
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return mChild->VGetWeightedReaders(weight, out);
#else
  mChild->VGetWeightedReaders(weight, out);
#endif
#endif
}
#if RSTL_CORRUPTION

const CVisibilityAnimationDataReader*
CAnimTreeSingleChild::VGetVisibilityDataAtCurrentTime() const {
  return mChild->GetVisibilityDataAtCurrentTime();
}

const CVisibilityAnimationDataReader*
CAnimTreeSingleChild::VGetVisibilityDataAtTime(const CCharAnimTime& time) const {
  return mChild->GetVisibilityDataAtTime(time);
}
#endif
