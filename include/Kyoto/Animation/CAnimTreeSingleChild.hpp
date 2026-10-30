#ifndef _CANIMTREESINGLECHILD
#define _CANIMTREESINGLECHILD
#include "rstl/RstlVersions.h"

#include "Kyoto/Animation/CAnimTreeNode.hpp"

class CAnimTreeSingleChild : public CAnimTreeNode {
public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CAnimTreeSingleChild(const rstl::ncrc_ptr< CAnimTreeNode >& node, const rstl::string& name)
  : CAnimTreeNode(name), mChild(node) {}
#else
  CAnimTreeSingleChild(const rstl::ncrc_ptr< CAnimTreeNode >& child, const rstl::string& name)
  : CAnimTreeNode(name), mChild(child) {}
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  SAdvancementResults VAdvanceView(const CCharAnimTime& dt) override;
#else
  // IAnimReader
  ~CAnimTreeSingleChild() override {}
  SAdvancementResults VAdvanceView(const CCharAnimTime& time) override;
#endif

  CCharAnimTime VGetTimeRemaining() const override;
  bool VHasOffset(const CSegId& seg) const override;
  CVector3f VGetOffset(const CSegId& seg) const override;
  CQuaternion VGetRotation(const CSegId& seg) const override;
  uint VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, uint capacity,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                       uint iterator, int) const override;
#else
                       uint iterator, int additive) const override;
#endif

  uint VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, uint capacity,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                        uint iterator, int) const override;
#else
                        uint iterator, int additive) const override;
#endif

  uint VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, uint capacity,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                           uint iterator, int) const override;
#else
                           uint iterator, int additive) const override;
#endif

  uint VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, uint capacity,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                        uint iterator, int) const override;
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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  void VSetPhase(float) override;
  SAdvancementResults VGetAdvancementResults(const CCharAnimTime& a,
                                             const CCharAnimTime& b) const override;
#else
  void VGetJointData_Linear(const CCharLayoutInfo& layout, CJointData_LinearStorage& data,
                            const CCharAnimTime& time) const override;
  void VGetJointData_Linear(const CCharLayoutInfo& layout,
                            CJointData_LinearStorage& data) const override;
  void VSetPhase(float phase) override;
  SAdvancementResults VGetAdvancementResults(const CCharAnimTime& time,
                                             const CCharAnimTime& startOffset) const override;

  // CAnimTreeNode
#endif

  uint Depth() const override { return mChild->Depth() + 1; }
  uint VGetNumChildren() const override;
  void VGetWeightedReaders(
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      float w, rstl::reserved_vector< rstl::pair< float, IAnimReader* >, 16 >& out) const override;
#else
      float weight,
      rstl::reserved_vector< rstl::pair< float, IAnimReader* >, 16 >& out) const override;
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  void ReplaceChild(const rstl::ncrc_ptr< CAnimTreeNode >& node) { mChild = node; }
#else
  void ReplaceChild(const rstl::ncrc_ptr< CAnimTreeNode >& child) { mChild = child; }
#endif

protected:
  rstl::ncrc_ptr< CAnimTreeNode > mChild;
};
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CHECK_SIZEOF(CAnimTreeSingleChild, 0x18)
#else
CHECK_CHILD_SIZEOF(CAnimTreeSingleChild, CAnimTreeNode, 0x8)
#endif

#endif // _CANIMTREESINGLECHILD
