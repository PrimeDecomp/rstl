#ifndef _CANIMTREETRANSITION
#define _CANIMTREETRANSITION
#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"
#endif

#if RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#endif
#include "Kyoto/Animation/CAnimTreeTweenBase.hpp"
#include "Kyoto/Animation/CPOINode.hpp"

class CAnimTreeTransition : public CAnimTreeTweenBase {
public:
  CAnimTreeTransition(const bool characterSpaceBlend, const rstl::ncrc_ptr< CAnimTreeNode >& a,
                      const rstl::ncrc_ptr< CAnimTreeNode >& b, const CCharAnimTime& duration,
#if RSTL_CORRUPTION
                      int blendCurve, const bool runA, int flags, const rstl::string& name);
#else
                      const bool runA, RSTL_PRIME1_CONST int flags, const rstl::string& name);
#endif
  CAnimTreeTransition(const bool characterSpaceBlend, const rstl::ncrc_ptr< CAnimTreeNode >& a,
                      const rstl::ncrc_ptr< CAnimTreeNode >& b, const CCharAnimTime& duration,
#if RSTL_CORRUPTION
                      int blendCurve, const CCharAnimTime& timeInTrans, const bool runA, bool loopA,
                      int flags, const rstl::string& name, bool initialized);

#else
                      const CCharAnimTime& timeInTrans, RSTL_PRIME1_CONST bool runA,
                      RSTL_PRIME1_CONST bool loopA, RSTL_PRIME1_CONST int flags,
                      const rstl::string& name, RSTL_PRIME1_CONST bool initialized);
#if !RSTL_PRIME1

  // IAnimReader
#endif
#endif

  ~CAnimTreeTransition() override;
  SAdvancementResults VAdvanceView(const CCharAnimTime& time) override;

  CCharAnimTime VGetTimeRemaining() const override;
  CSteadyStateAnimInfo VGetSteadyStateAnimInfo() const override;
#if RSTL_CORRUPTION
  uint VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, uint capacity,
                       uint iterator, const CSegIdMaskMap& mask) const override;
  uint VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, uint capacity,
                           uint iterator, const CSegIdMaskMap& mask) const override;
  uint VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, uint capacity,
                        uint iterator, const CSegIdMaskMap& mask) const override;
  uint VGetAssetSoundPOIList(const CCharAnimTime& time, CAssetSoundPOINode* listOut, uint capacity,
                             uint iterator, const CSegIdMaskMap& mask) const override;
  uint VGetAssetPOIList(const CCharAnimTime& time, CAssetPOINode* listOut, uint capacity,
                        uint iterator, const CSegIdMaskMap& mask) const override;
#endif
  rstl::ownership_transfer< IAnimReader > VClone() const override;
  rstl::optional_object< rstl::ownership_transfer< IAnimReader > > VSimplified() override;
#if RSTL_CORRUPTION
  SAdvancementResults VGetAdvancementResults(const CCharAnimTime& time,
                                            const CCharAnimTime& startOffset) const override;

#endif
  // CAnimTreeNode
  rstl::rc_ptr< CAnimTreeNode > VGetBestUnblendedChild() const override;
  // CAnimTreeTweenBase
  void SetBlendingWeight(float weight) override;
  rstl::optional_object< rstl::ownership_transfer< IAnimReader > > VReverseSimplified() override;
  float VGetBlendingWeight() const override;


  static rstl::string CreatePrimitiveName(const rstl::ncrc_ptr< CAnimTreeNode >& a,
#if RSTL_CORRUPTION
                                          const rstl::ncrc_ptr< CAnimTreeNode >& b, float duration);
#else
                                          const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                          RSTL_PRIME1_CONST float duration);
#endif

private:
  rstl::pair< CCharAnimTime, CAdvancementDeltas >

  AdvanceViewForTransitionalPeriod(const CCharAnimTime& time);
  // Guessed name.
  static uint GetLoopPOIHash();

  CCharAnimTime mTransDur;
  CCharAnimTime mTimeInTrans;
  const bool mRunA;
  bool mLoopA;
  bool mInitialized;
#if !RSTL_CORRUPTION
};
CHECK_CHILD_SIZEOF(CAnimTreeTransition, CAnimTreeTweenBase, 0x14)
#endif
  // Guessed name.
#if RSTL_CORRUPTION
  int mBlendCurve;
};
CHECK_SIZEOF(CAnimTreeTransition, 0x44)


#endif
#endif // _CANIMTREETRANSITION
