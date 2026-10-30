#ifndef _CANIMTREETRANSITION
#define _CANIMTREETRANSITION
#include "rstl/RstlVersions.h"

#include "Kyoto/Animation/CAnimTreeTweenBase.hpp"
#include "Kyoto/Animation/CPOINode.hpp"

class CAnimTreeTransition : public CAnimTreeTweenBase {
public:
#if !RSTL_PRIME1
  CAnimTreeTransition(const bool characterSpaceBlend, const rstl::ncrc_ptr< CAnimTreeNode >& a,
                      const rstl::ncrc_ptr< CAnimTreeNode >& b, const CCharAnimTime& duration,
                      const bool runA, RSTL_PRIME1_CONST int flags, const rstl::string& name);
  CAnimTreeTransition(const bool characterSpaceBlend, const rstl::ncrc_ptr< CAnimTreeNode >& a,
                      const rstl::ncrc_ptr< CAnimTreeNode >& b, const CCharAnimTime& duration,
                      const CCharAnimTime& timeInTrans, RSTL_PRIME1_CONST bool runA,
                      RSTL_PRIME1_CONST bool loopA, RSTL_PRIME1_CONST int flags,
                      const rstl::string& name, RSTL_PRIME1_CONST bool initialized);

  // IAnimReader
#endif

  ~CAnimTreeTransition() override;
  SAdvancementResults VAdvanceView(const CCharAnimTime& time) override;

  CCharAnimTime VGetTimeRemaining() const override;
  CSteadyStateAnimInfo VGetSteadyStateAnimInfo() const override;
  rstl::ownership_transfer< IAnimReader > VClone() const override;
  rstl::optional_object< rstl::ownership_transfer< IAnimReader > > VSimplified() override;
  // CAnimTreeNode
  rstl::rc_ptr< CAnimTreeNode > VGetBestUnblendedChild() const override;
  // CAnimTreeTweenBase
  void SetBlendingWeight(float weight) override;
  rstl::optional_object< rstl::ownership_transfer< IAnimReader > > VReverseSimplified() override;
  float VGetBlendingWeight() const override;

#if RSTL_PRIME1
  CAnimTreeTransition(const bool characterSpaceBlend, const rstl::ncrc_ptr< CAnimTreeNode >& a,
                      const rstl::ncrc_ptr< CAnimTreeNode >& b, const CCharAnimTime& duration,
                      const bool runA, RSTL_PRIME1_CONST int flags, const rstl::string& name);
  CAnimTreeTransition(const bool characterSpaceBlend, const rstl::ncrc_ptr< CAnimTreeNode >& a,
                      const rstl::ncrc_ptr< CAnimTreeNode >& b, const CCharAnimTime& duration,
                      const CCharAnimTime& timeInTrans, RSTL_PRIME1_CONST bool runA,
                      RSTL_PRIME1_CONST bool loopA, RSTL_PRIME1_CONST int flags,
                      const rstl::string& name, RSTL_PRIME1_CONST bool initialized);
#endif

  static rstl::string CreatePrimitiveName(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                          const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                          RSTL_PRIME1_CONST float duration);

private:
  rstl::pair< CCharAnimTime, CAdvancementDeltas >

  AdvanceViewForTransitionalPeriod(const CCharAnimTime& time);
#if !RSTL_PRIME1
  // Guessed name.
  static uint GetLoopPOIHash();
#endif

  CCharAnimTime mTransDur;
  CCharAnimTime mTimeInTrans;
  const bool mRunA;
  bool mLoopA;
  bool mInitialized;
};
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CHECK_SIZEOF(CAnimTreeTransition, 0x38)
#else
CHECK_CHILD_SIZEOF(CAnimTreeTransition, CAnimTreeTweenBase, 0x14)
#endif

#endif // _CANIMTREETRANSITION
