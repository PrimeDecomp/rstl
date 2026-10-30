#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CAnimTreeSingleChild.hpp"
#include "Kyoto/Math/CQuaternion.hpp"

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
SAdvancementResults CAnimTreeSingleChild::VAdvanceView(const CCharAnimTime& dt) {
  return mChild->VAdvanceView(dt);
#else
SAdvancementResults CAnimTreeSingleChild::VAdvanceView(const CCharAnimTime& time) {
  return mChild->VAdvanceView(time);
#endif

}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CCharAnimTime CAnimTreeSingleChild::VGetTimeRemaining() const {
  return mChild->VGetTimeRemaining();
}
#else
CCharAnimTime CAnimTreeSingleChild::VGetTimeRemaining() const { return mChild->GetTimeRemaining(); }
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
bool CAnimTreeSingleChild::VHasOffset(const CSegId& seg) const {
  return mChild->VHasOffset(seg);
}
#else
bool CAnimTreeSingleChild::VHasOffset(const CSegId& seg) const { return mChild->VHasOffset(seg); }
#endif

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
}

void CAnimTreeSingleChild::VGetJointData_Linear(const CCharLayoutInfo& layout,
                                                CJointData_LinearStorage& data,
                                                const CCharAnimTime& time) const {
  mChild->VGetJointData_Linear(layout, data, time);
}

void CAnimTreeSingleChild::VGetJointData_Linear(const CCharLayoutInfo& layout,
                                                CJointData_LinearStorage& data) const {
  mChild->VGetJointData_Linear(layout, data);
#endif

}

uint CAnimTreeSingleChild::VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                                           uint capacity, uint iterator, int unk) const {
  return mChild->GetBoolPOIList(time, listOut, capacity, iterator, unk);
#else
                                           uint capacity, uint iterator, int additive) const {
  return mChild->GetBoolPOIList(time, listOut, capacity, iterator, additive);
#endif

}

uint CAnimTreeSingleChild::VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                                            uint capacity, uint iterator, int unk) const {
  return mChild->GetInt32POIList(time, listOut, capacity, iterator, unk);
#else
                                            uint capacity, uint iterator, int additive) const {
  return mChild->GetInt32POIList(time, listOut, capacity, iterator, additive);
#endif

}

uint CAnimTreeSingleChild::VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                                               uint capacity, uint iterator, int unk) const {
  return mChild->GetParticlePOIList(time, listOut, capacity, iterator, unk);
#else
                                               uint capacity, uint iterator, int additive) const {
  return mChild->GetParticlePOIList(time, listOut, capacity, iterator, additive);
#endif

}

uint CAnimTreeSingleChild::VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                                            uint capacity, uint iterator, int unk) const {
  return mChild->GetSoundPOIList(time, listOut, capacity, iterator, unk);
#else
                                            uint capacity, uint iterator, int additive) const {
  return mChild->GetSoundPOIList(time, listOut, capacity, iterator, additive);
#endif

}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
bool CAnimTreeSingleChild::VGetBoolPOIState(const char* name) const {
  return mChild->VGetBoolPOIState(name);
#else
bool CAnimTreeSingleChild::VGetBoolPOIState(uint nameHash) const {
  return mChild->VGetBoolPOIState(nameHash);
#endif

}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
s32 CAnimTreeSingleChild::VGetInt32POIState(const char* name) const {
  return mChild->VGetInt32POIState(name);
#else
s32 CAnimTreeSingleChild::VGetInt32POIState(uint nameHash) const {
  return mChild->VGetInt32POIState(nameHash);
#endif

}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CParticleData::EParentedMode CAnimTreeSingleChild::VGetParticlePOIState(const char* name) const {
  return mChild->VGetParticlePOIState(name);
#else
CParticleData::EParentedMode CAnimTreeSingleChild::VGetParticlePOIState(uint nameHash) const {
  return mChild->VGetParticlePOIState(nameHash);
#endif

}

uint CAnimTreeSingleChild::VGetNumChildren() const { return mChild->VGetNumChildren() + 1; }

void CAnimTreeSingleChild::VSetPhase(float phase) { mChild->VSetPhase(phase); }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
SAdvancementResults CAnimTreeSingleChild::VGetAdvancementResults(const CCharAnimTime& a,
                                                                 const CCharAnimTime& b) const {
  return mChild->VGetAdvancementResults(a, b);
#else
SAdvancementResults
CAnimTreeSingleChild::VGetAdvancementResults(const CCharAnimTime& time,
                                             const CCharAnimTime& startOffset) const {
  return mChild->VGetAdvancementResults(time, startOffset);
#endif

}

void CAnimTreeSingleChild::VGetWeightedReaders(
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    float w, rstl::reserved_vector< rstl::pair< float, IAnimReader* >, 16 >& out) const {
  return mChild->VGetWeightedReaders(w, out);
#else
    float weight, rstl::reserved_vector< rstl::pair< float, IAnimReader* >, 16 >& out) const {
  mChild->VGetWeightedReaders(weight, out);
#endif

}

