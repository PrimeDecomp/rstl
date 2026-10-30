#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CAnimTreeSingleChild.hpp"
#include "Kyoto/Math/CQuaternion.hpp"

SAdvancementResults CAnimTreeSingleChild::VAdvanceView(const CCharAnimTime& time) {
  return mChild->VAdvanceView(time);
}

CCharAnimTime CAnimTreeSingleChild::VGetTimeRemaining() const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return mChild->VGetTimeRemaining();
#else
  return mChild->GetTimeRemaining();
#endif
}

bool CAnimTreeSingleChild::VHasOffset(const CSegId& seg) const {
  return mChild->VHasOffset(seg);
}

CVector3f CAnimTreeSingleChild::VGetOffset(const CSegId& seg) const {
  return mChild->VGetOffset(seg);
}

CQuaternion CAnimTreeSingleChild::VGetRotation(const CSegId& seg) const {
  return mChild->VGetRotation(seg);
}

void CAnimTreeSingleChild::VGetSegStatementSet(const CSegIdList& list,
                                               CSegStatementSet& setOut) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return mChild->VGetSegStatementSet(list, setOut);
#else
  mChild->VGetSegStatementSet(list, setOut);
#endif
}

void CAnimTreeSingleChild::VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut,
                                               const CCharAnimTime& time) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return mChild->VGetSegStatementSet(list, setOut, time);
#else
  mChild->VGetSegStatementSet(list, setOut, time);
#endif
}

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

uint CAnimTreeSingleChild::VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut,
                                           uint capacity, uint iterator, int additive) const {
  return mChild->GetBoolPOIList(time, listOut, capacity, iterator, additive);
}

uint CAnimTreeSingleChild::VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut,
                                            uint capacity, uint iterator, int additive) const {
  return mChild->GetInt32POIList(time, listOut, capacity, iterator, additive);
}

uint CAnimTreeSingleChild::VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut,
                                               uint capacity, uint iterator, int additive) const {
  return mChild->GetParticlePOIList(time, listOut, capacity, iterator, additive);
}

uint CAnimTreeSingleChild::VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut,
                                            uint capacity, uint iterator, int additive) const {
  return mChild->GetSoundPOIList(time, listOut, capacity, iterator, additive);
}

bool CAnimTreeSingleChild::VGetBoolPOIState(
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    const char* name
#else
    uint name
#endif
    ) const {
  return mChild->VGetBoolPOIState(name);
}

s32 CAnimTreeSingleChild::VGetInt32POIState(
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    const char* name
#else
    uint name
#endif
    ) const {
  return mChild->VGetInt32POIState(name);
}

CParticleData::EParentedMode CAnimTreeSingleChild::VGetParticlePOIState(
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    const char* name
#else
    uint name
#endif
    ) const {
  return mChild->VGetParticlePOIState(name);
}

uint CAnimTreeSingleChild::VGetNumChildren() const { return mChild->VGetNumChildren() + 1; }

void CAnimTreeSingleChild::VSetPhase(float phase) { mChild->VSetPhase(phase); }

SAdvancementResults
CAnimTreeSingleChild::VGetAdvancementResults(const CCharAnimTime& time,
                                             const CCharAnimTime& startOffset) const {
  return mChild->VGetAdvancementResults(time, startOffset);
}

void CAnimTreeSingleChild::VGetWeightedReaders(
    float weight, rstl::reserved_vector< rstl::pair< float, IAnimReader* >, 16 >& out) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return mChild->VGetWeightedReaders(weight, out);
#else
  mChild->VGetWeightedReaders(weight, out);
#endif
}
