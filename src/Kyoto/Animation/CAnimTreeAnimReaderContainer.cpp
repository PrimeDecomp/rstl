#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CAnimTreeAnimReaderContainer.hpp"

SAdvancementResults CAnimTreeAnimReaderContainer::VAdvanceView(const CCharAnimTime& time) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return mReader->AdvanceView(time);
#else
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
                                                   uint capacity, uint iterator,
                                                   int additive) const {
  return mReader->GetBoolPOIList(time, listOut, capacity, iterator, additive);
}

uint CAnimTreeAnimReaderContainer::VGetInt32POIList(const CCharAnimTime& time,
                                                    CInt32POINode* listOut, uint capacity,
                                                    uint iterator, int additive) const {
  return mReader->GetInt32POIList(time, listOut, capacity, iterator, additive);
}

uint CAnimTreeAnimReaderContainer::VGetParticlePOIList(const CCharAnimTime& time,
                                                       CParticlePOINode* listOut, uint capacity,
                                                       uint iterator, int additive) const {
  return mReader->GetParticlePOIList(time, listOut, capacity, iterator, additive);
}

uint CAnimTreeAnimReaderContainer::VGetSoundPOIList(const CCharAnimTime& time,
                                                    CSoundPOINode* listOut, uint capacity,
                                                    uint iterator, int additive) const {
  return mReader->GetSoundPOIList(time, listOut, capacity, iterator, additive);
}

bool CAnimTreeAnimReaderContainer::VGetBoolPOIState(
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    const char* name
#else
    uint name
#endif
    ) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return mReader->GetBoolPOIState(name);
#else
  return mReader->VGetBoolPOIState(name);
#endif
}

s32 CAnimTreeAnimReaderContainer::VGetInt32POIState(
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    const char* name
#else
    uint name
#endif
    ) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return mReader->GetInt32POIState(name);
#else
  return mReader->VGetInt32POIState(name);
#endif
}

CParticleData::EParentedMode CAnimTreeAnimReaderContainer::VGetParticlePOIState(
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    const char* name
#else
    uint name
#endif
    ) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return mReader->GetParticlePOIState(name);
#else
  return mReader->VGetParticlePOIState(name);
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
#endif
}

#if !RSTL_PRIME1
void CAnimTreeAnimReaderContainer::VGetJointData_Linear(const CCharLayoutInfo& layout,
                                                        CJointData_LinearStorage& data,
                                                        const CCharAnimTime& time) const {
  mReader->VGetJointData_Linear(layout, data, time);
}

void CAnimTreeAnimReaderContainer::VGetJointData_Linear(const CCharLayoutInfo& layout,
                                                        CJointData_LinearStorage& data) const {
  mReader->VGetJointData_Linear(layout, data);
}
#endif

rstl::ownership_transfer< IAnimReader > CAnimTreeAnimReaderContainer::VClone() const {
  return rs_new CAnimTreeAnimReaderContainer(mReader->Clone(), mName, mAnimDbIdx);
}

CAnimTreeEffectiveContribution
CAnimTreeAnimReaderContainer::VGetContributionOfHighestInfluence() const {
  return CAnimTreeEffectiveContribution(1.f, mName
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                                        , mReader->GetSteadyStateAnimInfo()
#else
                                        , mReader->VGetSteadyStateAnimInfo()
#endif
                                        , mReader->GetTimeRemaining(), mAnimDbIdx);
}

rstl::optional_object< rstl::ownership_transfer< IAnimReader > >
CAnimTreeAnimReaderContainer::VSimplified() {
  return rstl::optional_object_null();
}

void CAnimTreeAnimReaderContainer::VSetPhase(float phase) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  mReader->SetPhase(phase);
#else
  mReader->VSetPhase(phase);
#endif
}

SAdvancementResults
CAnimTreeAnimReaderContainer::VGetAdvancementResults(const CCharAnimTime& time,
                                                     const CCharAnimTime& startOffset) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return mReader->GetAdvancementResults(time, startOffset);
#else
  return mReader->VGetAdvancementResults(time, startOffset);
#endif
}

rstl::rc_ptr< CAnimTreeNode > CAnimTreeAnimReaderContainer::VGetBestUnblendedChild() const {
  return rstl::rc_ptr< CAnimTreeNode >();
}

void CAnimTreeAnimReaderContainer::VGetWeightedReaders(
    float weight, rstl::reserved_vector< rstl::pair< float, IAnimReader* >, 16 >& out) const {
  out.push_back(rstl::pair< float, IAnimReader* >(weight, const_cast< IAnimReader* >(&*mReader)));
}
