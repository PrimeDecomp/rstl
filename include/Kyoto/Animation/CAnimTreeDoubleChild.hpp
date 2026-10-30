#ifndef _CANIMTREEDOUBLECHILD
#define _CANIMTREEDOUBLECHILD
#include "rstl/RstlVersions.h"

#include "Kyoto/Animation/CAnimTreeNode.hpp"
#include "rstl/math.hpp"

class CAnimTreeDoubleChild : public CAnimTreeNode {
public:
  class CDoubleChildAdvancementResult {
#if !RSTL_PRIME1
  public:
    CDoubleChildAdvancementResult(const CCharAnimTime& time, const CAdvancementDeltas& left,
                                  const CAdvancementDeltas& right);
    const CCharAnimTime& GetTrueAdvancement() const { return mTrueAdvancement; }
    const CAdvancementDeltas& GetLeftAdvancementDeltas() const { return mLeftDeltas; }
    const CAdvancementDeltas& GetRightAdvancementDeltas() const { return mRightDeltas; }

  private:
#endif

    CCharAnimTime mTrueAdvancement;
    CAdvancementDeltas mLeftDeltas;
    CAdvancementDeltas mRightDeltas;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  public:
    CDoubleChildAdvancementResult(const CCharAnimTime& trueAdvancement,
                                  const CAdvancementDeltas& leftDeltas,
                                  const CAdvancementDeltas& rightDeltas);
    const CAdvancementDeltas& GetLeftAdvancementDeltas() const { return mLeftDeltas; }
    const CAdvancementDeltas& GetRightAdvancementDeltas() const { return mRightDeltas; }
    const CCharAnimTime& GetTrueAdvancement() const { return mTrueAdvancement; }
#endif
  };

#if RSTL_PRIME1
protected:
  CDoubleChildAdvancementResult AdvanceViewBothChildren(const CCharAnimTime& time, bool runLeft,
                                                        bool loopLeft);

public:
#endif

  CAnimTreeDoubleChild(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                       const rstl::ncrc_ptr< CAnimTreeNode >& b, const rstl::string& name);
  // IAnimReader
  ~CAnimTreeDoubleChild() override;
  SAdvancementResults VAdvanceView(const CCharAnimTime& time) override;

  uint VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, uint capacity,
                       uint iterator, int additive) const override;

  uint VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, uint capacity,
                        uint iterator, int additive) const override;

  uint VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, uint capacity,
                           uint iterator, int additive) const override;

  uint VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, uint capacity,
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
  void VSetPhase(float phase) override;
  SAdvancementResults VGetAdvancementResults(const CCharAnimTime& time,
                                             const CCharAnimTime& startOffset) const override;

  // CAnimTreeNode
  uint Depth() const override { return rstl::max_val(mA->Depth(), mB->Depth()) + 1; }
  CAnimTreeEffectiveContribution VGetContributionOfHighestInfluence() const override;
  uint VGetNumChildren() const override;
  rstl::rc_ptr< CAnimTreeNode > VGetBestUnblendedChild() const override;
  void VGetWeightedReaders(
      float weight,
      rstl::reserved_vector< rstl::pair< float, IAnimReader* >, 16 >& out) const override;

  virtual float VGetRightChildWeight() const = 0;
  void ReplaceLeftChild(const rstl::ncrc_ptr< CAnimTreeNode >& child) { mA = child; }
  void ReplaceRightChild(const rstl::ncrc_ptr< CAnimTreeNode >& child) { mB = child; }

  float GetLeftChildWeight() const { return 1.f - VGetRightChildWeight(); }
  float GetRightChildWeight() const { return VGetRightChildWeight(); }

  const rstl::rc_ptr< CAnimTreeNode >& GetLeftChild() const { return mA; }
  const rstl::rc_ptr< CAnimTreeNode >& GetRightChild() const { return mB; }

protected:
#if !RSTL_PRIME1
  CDoubleChildAdvancementResult AdvanceViewBothChildren(const CCharAnimTime& time, bool runLeft,
                                                        bool loopLeft);
#endif

  rstl::ncrc_ptr< CAnimTreeNode > mA;
  rstl::ncrc_ptr< CAnimTreeNode > mB;
};
CHECK_CHILD_SIZEOF(CAnimTreeDoubleChild, CAnimTreeNode, RSTL_PRIME1 ? (RSTL_VERSION >= RSTL_R3IJ ? 0xc : 0x8) : 0x10)

#endif // _CANIMTREEDOUBLECHILD
