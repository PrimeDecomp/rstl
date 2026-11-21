#ifndef _CANIMTREEDOUBLECHILD
#define _CANIMTREEDOUBLECHILD
#include "rstl/RstlVersions.h"
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"

#if RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#endif
#include "Kyoto/Animation/CAnimTreeNode.hpp"
#include "rstl/math.hpp"

class CAnimTreeDoubleChild : public CAnimTreeNode {
public:
  class CDoubleChildAdvancementResult {
  public:
#if RSTL_CORRUPTION
    CDoubleChildAdvancementResult(const CCharAnimTime& time, const CAdvancementDeltas& leftDeltas,
                                  const CAdvancementDeltas& rightDeltas);
#else
    CDoubleChildAdvancementResult(const CCharAnimTime& trueAdvancement,
                                  const CAdvancementDeltas& leftDeltas,
                                  const CAdvancementDeltas& rightDeltas);
#endif
    const CCharAnimTime& GetTrueAdvancement() const { return mTrueAdvancement; }
    const CAdvancementDeltas& GetLeftAdvancementDeltas() const { return mLeftDeltas; }
    const CAdvancementDeltas& GetRightAdvancementDeltas() const { return mRightDeltas; }

  private:
    CCharAnimTime mTrueAdvancement;
    CAdvancementDeltas mLeftDeltas;
    CAdvancementDeltas mRightDeltas;
  };

  CAnimTreeDoubleChild(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                       const rstl::ncrc_ptr< CAnimTreeNode >& b, const rstl::string& name);
  // IAnimReader
  ~CAnimTreeDoubleChild() override;
  SAdvancementResults VAdvanceView(const CCharAnimTime& time) override;

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
  void VSetPhase(float phase) override;
  SAdvancementResults VGetAdvancementResults(const CCharAnimTime& time,
                                             const CCharAnimTime& startOffset) const override;

  // CAnimTreeNode
  uint Depth() const override { return rstl::max_val(mA->Depth(), mB->Depth()) + 1; }
  CAnimTreeEffectiveContribution VGetContributionOfHighestInfluence() const override;
#if RSTL_CORRUPTION
  uint VGetNumDirectChildren() const override;
#endif
  uint VGetNumChildren() const override;
  rstl::rc_ptr< CAnimTreeNode > VGetBestUnblendedChild() const override;
  void VGetWeightedReaders(
      float weight,
      rstl::reserved_vector< rstl::pair< float, IAnimReader* >, 16 >& out) const override;
#if RSTL_CORRUPTION
  rstl::rc_ptr< CAnimTreeNode > VGetDirectChild(uint index) const override;
#endif

  virtual float VGetRightChildWeight() const = 0;
  void ReplaceLeftChild(const rstl::ncrc_ptr< CAnimTreeNode >& child) { mA = child; }
  void ReplaceRightChild(const rstl::ncrc_ptr< CAnimTreeNode >& child) { mB = child; }

  float GetLeftChildWeight() const { return 1.f - VGetRightChildWeight(); }
  float GetRightChildWeight() const { return VGetRightChildWeight(); }

  const rstl::rc_ptr< CAnimTreeNode >& GetLeftChild() const { return mA; }
  const rstl::rc_ptr< CAnimTreeNode >& GetRightChild() const { return mB; }

protected:
  CDoubleChildAdvancementResult AdvanceViewBothChildren(const CCharAnimTime& time, bool runLeft,
                                                        bool loopLeft);

  rstl::ncrc_ptr< CAnimTreeNode > mA;
  rstl::ncrc_ptr< CAnimTreeNode > mB;
};
#if RSTL_CORRUPTION
CHECK_SIZEOF(CAnimTreeDoubleChild, 0x24)
#else
CHECK_CHILD_SIZEOF(CAnimTreeDoubleChild, CAnimTreeNode, RSTL_PRIME1 ? (RSTL_VERSION >= RSTL_R3IJ ? 0xc : 0x8) : 0x10)
#endif

#endif // _CANIMTREEDOUBLECHILD
