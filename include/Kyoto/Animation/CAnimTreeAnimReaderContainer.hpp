#ifndef _CANIMTREEANIMREADERCONTAINER
#define _CANIMTREEANIMREADERCONTAINER
#include "rstl/RstlVersions.h"

#include "Kyoto/Animation/CAnimTreeNode.hpp"
#include "rstl/object_owner.hpp"

class CAnimTreeAnimReaderContainer : public CAnimTreeNode {
public:
#if RSTL_PRIME1
  ~CAnimTreeAnimReaderContainer() override {
    CCharAnimMemoryMetrics::SubtractFromTotalSize(8, CCharAnimMemoryMetrics::kASS_Two);
  }
  uint Depth() const override { return 1; }
  uint VGetNumChildren() const override { return 0; }
#endif

  CAnimTreeAnimReaderContainer(const rstl::ownership_transfer< IAnimReader >& reader,
                               const rstl::string& name, uint animDbIdx)
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  : CAnimTreeNode(name)
  , mReader(reader)
  , mAnimDbIdx(animDbIdx) {
    CCharAnimMemoryMetrics::AddToTotalSize(8, CCharAnimMemoryMetrics::kASS_Two);
  }
#else
  : CAnimTreeNode(name), mReader(reader), mAnimDbIdx(animDbIdx) {}
#endif

// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
  // IAnimReader
  ~CAnimTreeAnimReaderContainer() override {}
#endif
  SAdvancementResults VAdvanceView(const CCharAnimTime& time) override;

  CCharAnimTime VGetTimeRemaining() const override;
  CSteadyStateAnimInfo VGetSteadyStateAnimInfo() const override;
  bool VHasOffset(const CSegId& seg) const override;
  CVector3f VGetOffset(const CSegId& seg) const override;
  CQuaternion VGetRotation(const CSegId& seg) const override;
  uint VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, uint capacity,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                       uint iterator, int unk) const override;
#else
                       uint iterator, int additive) const override;
#endif

  uint VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, uint capacity,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                        uint iterator, int unk) const override;
#else
                        uint iterator, int additive) const override;
#endif

  uint VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, uint capacity,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                           uint iterator, int unk) const override;
#else
                           uint iterator, int additive) const override;
#endif

  uint VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, uint capacity,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                        uint iterator, int unk) const override;
  bool VGetBoolPOIState(const char* name) const override;
  s32 VGetInt32POIState(const char* name) const override;
  CParticleData::EParentedMode VGetParticlePOIState(const char* name) const override;
#else
                        uint iterator, int additive) const override;
  bool VGetBoolPOIState(uint nameHash) const override;
  s32 VGetInt32POIState(uint nameHash) const override;
  CParticleData::EParentedMode VGetParticlePOIState(uint nameHash) const override;
#endif

  void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut) const override;
  void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut,
                           const CCharAnimTime& time) const override;
#if !RSTL_PRIME1
  void VGetJointData_Linear(const CCharLayoutInfo& layout, CJointData_LinearStorage& data,
                            const CCharAnimTime& time) const override;
  void VGetJointData_Linear(const CCharLayoutInfo& layout,
                            CJointData_LinearStorage& data) const override;
#endif

  rstl::ownership_transfer< IAnimReader > VClone() const override;
#if RSTL_PRIME1
  CAnimTreeEffectiveContribution VGetContributionOfHighestInfluence() const override;
#endif

  rstl::optional_object< rstl::ownership_transfer< IAnimReader > > VSimplified() override;
  void VSetPhase(float phase) override;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  SAdvancementResults VGetAdvancementResults(const CCharAnimTime& a,
                                             const CCharAnimTime& b) const override;
#else
  SAdvancementResults VGetAdvancementResults(const CCharAnimTime& time,
                                             const CCharAnimTime& startOffset) const override;

  // CAnimTreeNode
  uint Depth() const override { return 1; }
  CAnimTreeEffectiveContribution VGetContributionOfHighestInfluence() const override;
  uint VGetNumChildren() const override { return 0; }
#endif

  rstl::rc_ptr< CAnimTreeNode > VGetBestUnblendedChild() const override;
  void VGetWeightedReaders(
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      float w, rstl::reserved_vector< rstl::pair< float, IAnimReader* >, 16 >& out) const override;
#else
      float weight,
      rstl::reserved_vector< rstl::pair< float, IAnimReader* >, 16 >& out) const override;
#endif

private:
  rstl::object_owner< IAnimReader, rstl::call_deep_clone< IAnimReader > > mReader;
  uint mAnimDbIdx;
};
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CHECK_SIZEOF(CAnimTreeAnimReaderContainer, 0x20)
#else
CHECK_CHILD_SIZEOF(CAnimTreeAnimReaderContainer, CAnimTreeNode, 0xc)
#endif

#endif // _CANIMTREEANIMREADERCONTAINER
