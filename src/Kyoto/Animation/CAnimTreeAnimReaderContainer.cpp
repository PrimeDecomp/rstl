#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CAnimTreeAnimReaderContainer.hpp"

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CAdvancementResults CAnimTreeAnimReaderContainer::VAdvanceView(const CCharAnimTime& time) {
  return mReader->AdvanceView(time);
#else
SAdvancementResults CAnimTreeAnimReaderContainer::VAdvanceView(const CCharAnimTime& time) {
  return mReader->VAdvanceView(time);
#endif

}

CCharAnimTime CAnimTreeAnimReaderContainer::VGetTimeRemaining() const {
  return mReader->GetTimeRemaining();
}

CSteadyStateAnimInfo CAnimTreeAnimReaderContainer::VGetSteadyStateAnimInfo() const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return mReader->GetSteadyStateAnimInfo();
#else
  return mReader->VGetSteadyStateAnimInfo();
#endif

}

bool CAnimTreeAnimReaderContainer::VHasOffset(const CSegId& seg) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return mReader->HasOffset(seg);
#else
  return mReader->VHasOffset(seg);
#endif

}

CVector3f CAnimTreeAnimReaderContainer::VGetOffset(const CSegId& seg) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return mReader->GetOffset(seg);
#else
  return mReader->VGetOffset(seg);
#endif

}

CQuaternion CAnimTreeAnimReaderContainer::VGetRotation(const CSegId& seg) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return mReader->GetRotation(seg);
#else
  return mReader->VGetRotation(seg);
#endif

}

uint CAnimTreeAnimReaderContainer::VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                                                   uint capacity, uint iterator, int unk) const {
  return mReader->GetBoolPOIList(time, listOut, capacity, iterator, unk);
#else
                                                   uint capacity, uint iterator,
                                                   int additive) const {
  return mReader->GetBoolPOIList(time, listOut, capacity, iterator, additive);
#endif

}

uint CAnimTreeAnimReaderContainer::VGetInt32POIList(const CCharAnimTime& time,
                                                    CInt32POINode* listOut, uint capacity,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                                                    uint iterator, int unk) const {
  return mReader->GetInt32POIList(time, listOut, capacity, iterator, unk);
#else
                                                    uint iterator, int additive) const {
  return mReader->GetInt32POIList(time, listOut, capacity, iterator, additive);
#endif

}

uint CAnimTreeAnimReaderContainer::VGetParticlePOIList(const CCharAnimTime& time,
                                                       CParticlePOINode* listOut, uint capacity,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                                                       uint iterator, int unk) const {
  return mReader->GetParticlePOIList(time, listOut, capacity, iterator, unk);
#else
                                                       uint iterator, int additive) const {
  return mReader->GetParticlePOIList(time, listOut, capacity, iterator, additive);
#endif

}

uint CAnimTreeAnimReaderContainer::VGetSoundPOIList(const CCharAnimTime& time,
                                                    CSoundPOINode* listOut, uint capacity,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                                                    uint iterator, int unk) const {
  return mReader->GetSoundPOIList(time, listOut, capacity, iterator, unk);
#else
                                                    uint iterator, int additive) const {
  return mReader->GetSoundPOIList(time, listOut, capacity, iterator, additive);
#endif

}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
bool CAnimTreeAnimReaderContainer::VGetBoolPOIState(const char* name) const {
  return mReader->GetBoolPOIState(name);
#else
bool CAnimTreeAnimReaderContainer::VGetBoolPOIState(uint nameHash) const {
  return mReader->VGetBoolPOIState(nameHash);
#endif

}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
s32 CAnimTreeAnimReaderContainer::VGetInt32POIState(const char* name) const {
  return mReader->GetInt32POIState(name);
#else
s32 CAnimTreeAnimReaderContainer::VGetInt32POIState(uint nameHash) const {
  return mReader->VGetInt32POIState(nameHash);
#endif

}

CParticleData::EParentedMode
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CAnimTreeAnimReaderContainer::VGetParticlePOIState(const char* name) const {
  return mReader->GetParticlePOIState(name);
#else
CAnimTreeAnimReaderContainer::VGetParticlePOIState(uint nameHash) const {
  return mReader->VGetParticlePOIState(nameHash);
#endif

}

void CAnimTreeAnimReaderContainer::VGetSegStatementSet(const CSegIdList& list,
                                                       CSegStatementSet& setOut) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  mReader->GetSegStatementSet(list, setOut);
#else
  mReader->VGetSegStatementSet(list, setOut);
#endif

}

void CAnimTreeAnimReaderContainer::VGetSegStatementSet(const CSegIdList& list,
                                                       CSegStatementSet& setOut,
                                                       const CCharAnimTime& time) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  mReader->GetSegStatementSet(list, setOut, time);
#else
  mReader->VGetSegStatementSet(list, setOut, time);
}

void CAnimTreeAnimReaderContainer::VGetJointData_Linear(const CCharLayoutInfo& layout,
                                                        CJointData_LinearStorage& data,
                                                        const CCharAnimTime& time) const {
  mReader->VGetJointData_Linear(layout, data, time);
}

void CAnimTreeAnimReaderContainer::VGetJointData_Linear(const CCharLayoutInfo& layout,
                                                        CJointData_LinearStorage& data) const {
  mReader->VGetJointData_Linear(layout, data);
#endif

}

rstl::ownership_transfer< IAnimReader > CAnimTreeAnimReaderContainer::VClone() const {
  return rs_new CAnimTreeAnimReaderContainer(mReader->Clone(), mName, mAnimDbIdx);
}

CAnimTreeEffectiveContribution
CAnimTreeAnimReaderContainer::VGetContributionOfHighestInfluence() const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return CAnimTreeEffectiveContribution(1.f, mName, mReader->GetSteadyStateAnimInfo(),
#else
  return CAnimTreeEffectiveContribution(1.f, mName, mReader->VGetSteadyStateAnimInfo(),
#endif

                                        mReader->GetTimeRemaining(), mAnimDbIdx);
}

rstl::optional_object< rstl::ownership_transfer< IAnimReader > >
CAnimTreeAnimReaderContainer::VSimplified() {
  return rstl::optional_object_null();
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CAnimTreeAnimReaderContainer::VSetPhase(float phase) { mReader->SetPhase(phase); }
#else
void CAnimTreeAnimReaderContainer::VSetPhase(float phase) { mReader->VSetPhase(phase); }
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CAdvancementResults
CAnimTreeAnimReaderContainer::VGetAdvancementResults(const CCharAnimTime& a,
                                                     const CCharAnimTime& b) const {
  return mReader->GetAdvancementResults(a, b);
#else
SAdvancementResults
CAnimTreeAnimReaderContainer::VGetAdvancementResults(const CCharAnimTime& time,
                                                     const CCharAnimTime& startOffset) const {
  return mReader->VGetAdvancementResults(time, startOffset);
#endif

}

rstl::rc_ptr< CAnimTreeNode > CAnimTreeAnimReaderContainer::VGetBestUnblendedChild() const {
  return rstl::rc_ptr< CAnimTreeNode >();
}

void CAnimTreeAnimReaderContainer::VGetWeightedReaders(
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    float w, rstl::reserved_vector< rstl::pair< float, IAnimReader* >, 16 >& out) const {
  out.push_back(rstl::pair< float, IAnimReader* >(w, const_cast< IAnimReader* >(&*mReader)));
#else
    float weight, rstl::reserved_vector< rstl::pair< float, IAnimReader* >, 16 >& out) const {
  out.push_back(rstl::pair< float, IAnimReader* >(weight, const_cast< IAnimReader* >(&*mReader)));
#endif

}

