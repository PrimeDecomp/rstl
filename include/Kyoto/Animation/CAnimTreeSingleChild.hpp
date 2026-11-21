#ifndef _CANIMTREESINGLECHILD
#define _CANIMTREESINGLECHILD
#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"
#endif

#if RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#endif
#include "Kyoto/Animation/CAnimTreeNode.hpp"

class CAnimTreeSingleChild : public CAnimTreeNode {
public:
  CAnimTreeSingleChild(const rstl::ncrc_ptr< CAnimTreeNode >& child, const rstl::string& name)
  : CAnimTreeNode(name), mChild(child) {}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_CORRUPTION
  ~CAnimTreeSingleChild() override {}
#else
#if !RSTL_PRIME1
  // IAnimReader
  ~CAnimTreeSingleChild() override {}
#endif
#endif
  SAdvancementResults VAdvanceView(const CCharAnimTime& time) override;

  CCharAnimTime VGetTimeRemaining() const override;
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
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
  void VGetJointData_Linear(const CCharLayoutInfo& layout, CJointData_LinearStorage& data,
                            const CCharAnimTime& time) const override;
  void VGetJointData_Linear(const CCharLayoutInfo& layout,
                            CJointData_LinearStorage& data) const override;
#endif
#endif
  void VSetPhase(float phase) override;
  SAdvancementResults VGetAdvancementResults(const CCharAnimTime& time,
                                             const CCharAnimTime& startOffset) const override;

  // CAnimTreeNode
  uint Depth() const override { return mChild->Depth() + 1; }
#if RSTL_CORRUPTION
  uint VGetNumDirectChildren() const override;
#endif
  uint VGetNumChildren() const override;
  void VGetWeightedReaders(
      float weight,
      rstl::reserved_vector< rstl::pair< float, IAnimReader* >, 16 >& out) const override;
#if RSTL_CORRUPTION
  rstl::rc_ptr< CAnimTreeNode > VGetDirectChild(uint index) const override;
#endif

  void ReplaceChild(const rstl::ncrc_ptr< CAnimTreeNode >& child) { mChild = child; }

protected:
  rstl::ncrc_ptr< CAnimTreeNode > mChild;
};
#if RSTL_CORRUPTION
CHECK_SIZEOF(CAnimTreeSingleChild, 0x1c)
#else
CHECK_CHILD_SIZEOF(CAnimTreeSingleChild, CAnimTreeNode, (RSTL_PRIME1 && RSTL_VERSION < RSTL_R3IJ) ? 0x4 : 0x8)
#endif

#endif // _CANIMTREESINGLECHILD
