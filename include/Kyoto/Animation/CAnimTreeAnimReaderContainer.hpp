#ifndef _CANIMTREEANIMREADERCONTAINER
#define _CANIMTREEANIMREADERCONTAINER
#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"
#endif

#if RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#endif
#include "Kyoto/Animation/CAnimTreeNode.hpp"
#include "rstl/object_owner.hpp"

class CAnimTreeAnimReaderContainer : public CAnimTreeNode {
public:
#if !RSTL_CORRUPTION
  ~CAnimTreeAnimReaderContainer() override {
#if RSTL_PRIME1
    CCharAnimMemoryMetrics::SubtractFromTotalSize(8, CCharAnimMemoryMetrics::kASS_Two);
#endif
  }
#if RSTL_PRIME1
  uint Depth() const override { return 1; }
  uint VGetNumChildren() const override { return 0; }
#endif

#endif
  CAnimTreeAnimReaderContainer(const rstl::ownership_transfer< IAnimReader >& reader,
#if RSTL_CORRUPTION
                               const CPrimitive& primitive)
  : CAnimTreeNode(primitive.GetName()), mReader(reader), mPrimitive(primitive) {}
#else
                               const rstl::string& name, uint animDbIdx)
  : CAnimTreeNode(name)
  , mReader(reader)
  , mAnimDbIdx(animDbIdx) {
#endif
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_CORRUPTION
#if RSTL_PRIME1
    CCharAnimMemoryMetrics::AddToTotalSize(8, CCharAnimMemoryMetrics::kASS_Two);
#endif
  }
#endif

#if RSTL_CORRUPTION
  ~CAnimTreeAnimReaderContainer() override {}
#endif
  SAdvancementResults VAdvanceView(const CCharAnimTime& time) override;

  CCharAnimTime VGetTimeRemaining() const override;
  CSteadyStateAnimInfo VGetSteadyStateAnimInfo() const override;
  bool VHasOffset(const CSegId& seg) const override;
  CVector3f VGetOffset(const CSegId& seg) const override;
  CQuaternion VGetRotation(const CSegId& seg) const override;
  uint VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, uint capacity,
#if RSTL_CORRUPTION
                       uint iterator, const CSegIdMaskMap& mask) const override;
#else
                       uint iterator, int additive) const override;

  uint VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, uint capacity,
                        uint iterator, int additive) const override;

#endif
  uint VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, uint capacity,
#if RSTL_CORRUPTION
                           uint iterator, const CSegIdMaskMap& mask) const override;
#else
                           uint iterator, int additive) const override;

#endif
  uint VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, uint capacity,
#if RSTL_CORRUPTION
                        uint iterator, const CSegIdMaskMap& mask) const override;
  uint VGetAssetSoundPOIList(const CCharAnimTime& time, CAssetSoundPOINode* listOut, uint capacity,
                             uint iterator, const CSegIdMaskMap& mask) const override;
  uint VGetAssetPOIList(const CCharAnimTime& time, CAssetPOINode* listOut, uint capacity,
                        uint iterator, const CSegIdMaskMap& mask) const override;
  bool VGetBoolPOIState(uint nameHash) const override;
  CParticleData::EParentedMode VGetParticlePOIState(uint nameHash) const override;
#else
                        uint iterator, int additive) const override;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  bool VGetBoolPOIState(const char* name) const override;
  s32 VGetInt32POIState(const char* name) const override;
  CParticleData::EParentedMode VGetParticlePOIState(const char* name) const override;
#else
  bool VGetBoolPOIState(uint nameHash) const override;
  s32 VGetInt32POIState(uint nameHash) const override;
  CParticleData::EParentedMode VGetParticlePOIState(uint nameHash) const override;
#endif

#endif
  void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut) const override;
  void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut,
                           const CCharAnimTime& time) const override;
#if RSTL_CORRUPTION
  void VGetJointData_Linear(const CCharLayoutInfo& layout, CJointData_LinearStorage& data,
                            const CCharAnimTime& time) const override;
  void VGetJointData_Linear(const CCharLayoutInfo& layout,
                            CJointData_LinearStorage& data) const override;
  const CVisibilityAnimationDataReader* VGetVisibilityDataAtCurrentTime() const override;
  const CVisibilityAnimationDataReader*
  VGetVisibilityDataAtTime(const CCharAnimTime& time) const override;
#else
#if !RSTL_PRIME1
  void VGetJointData_Linear(const CCharLayoutInfo& layout, CJointData_LinearStorage& data,
                            const CCharAnimTime& time) const override;
  void VGetJointData_Linear(const CCharLayoutInfo& layout,
                            CJointData_LinearStorage& data) const override;
#endif

#endif
  rstl::ownership_transfer< IAnimReader > VClone() const override;
#if !RSTL_CORRUPTION
#if RSTL_PRIME1
  CAnimTreeEffectiveContribution VGetContributionOfHighestInfluence() const override;
#endif

#endif
  rstl::optional_object< rstl::ownership_transfer< IAnimReader > > VSimplified() override;
  void VSetPhase(float phase) override;
  SAdvancementResults VGetAdvancementResults(const CCharAnimTime& time,
                                             const CCharAnimTime& startOffset) const override;
#if RSTL_CORRUPTION

  // CAnimTreeNode
  uint Depth() const override { return 1; }
  CAnimTreeEffectiveContribution VGetContributionOfHighestInfluence() const override;
  uint VGetNumDirectChildren() const override { return 0; }
  uint VGetNumChildren() const override { return 0; }
#else
#if !RSTL_PRIME1
  uint Depth() const override { return 1; }
  CAnimTreeEffectiveContribution VGetContributionOfHighestInfluence() const override;
  uint VGetNumChildren() const override { return 0; }
#endif
#endif
  rstl::rc_ptr< CAnimTreeNode > VGetBestUnblendedChild() const override;
  void VGetWeightedReaders(
      float weight,
      rstl::reserved_vector< rstl::pair< float, IAnimReader* >, 16 >& out) const override;
#if RSTL_CORRUPTION
  rstl::rc_ptr< CAnimTreeNode > VGetDirectChild(uint index) const override {
    return rstl::rc_ptr< CAnimTreeNode >(nullptr);
  }
#endif

private:
  rstl::object_owner< IAnimReader, rstl::call_deep_clone< IAnimReader > > mReader;
#if RSTL_CORRUPTION
  CPrimitive mPrimitive;
#else
  uint mAnimDbIdx;
#endif
};
#if RSTL_CORRUPTION
CHECK_SIZEOF(CAnimTreeAnimReaderContainer, 0x40)
#else
CHECK_CHILD_SIZEOF(CAnimTreeAnimReaderContainer, CAnimTreeNode, (RSTL_PRIME1 && RSTL_VERSION >= RSTL_R3IJ) ? 0x10 : 0xc)
#endif

#endif // _CANIMTREEANIMREADERCONTAINER
