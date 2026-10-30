#ifndef _CANIMTREETRANSITION
#define _CANIMTREETRANSITION
#include "rstl/RstlVersions.h"

#include "Kyoto/Animation/CAnimTreeTweenBase.hpp"
#if !RSTL_PRIME1
#include "Kyoto/Animation/CPOINode.hpp"
#endif

class CAnimTreeTransition : public CAnimTreeTweenBase {
public:
#if !RSTL_PRIME1
  CAnimTreeTransition(const bool characterSpaceBlend, const rstl::ncrc_ptr< CAnimTreeNode >& a,
                      const rstl::ncrc_ptr< CAnimTreeNode >& b, const CCharAnimTime& duration,
                      const bool runA, int flags, const rstl::string& name);
  CAnimTreeTransition(const bool characterSpaceBlend, const rstl::ncrc_ptr< CAnimTreeNode >& a,
                      const rstl::ncrc_ptr< CAnimTreeNode >& b, const CCharAnimTime& duration,
                      const CCharAnimTime& timeInTrans, bool runA, bool loopA, int flags,
                      const rstl::string& name, bool initialized);

  // IAnimReader
#endif

  ~CAnimTreeTransition() override;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CAdvancementResults VAdvanceView(const CCharAnimTime& time) override;
#else
  SAdvancementResults VAdvanceView(const CCharAnimTime& time) override;
#endif

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
  CAnimTreeTransition(const bool b1, const rstl::ncrc_ptr< CAnimTreeNode >& a,
                      const rstl::ncrc_ptr< CAnimTreeNode >& b, const CCharAnimTime& transDur,
                      const bool runA, const int flags, const rstl::string& name);
  CAnimTreeTransition(const bool b1, const rstl::ncrc_ptr< CAnimTreeNode >& a,
                      const rstl::ncrc_ptr< CAnimTreeNode >& b, const CCharAnimTime& transDur,
                      const CCharAnimTime& timeInTrans, const bool runA, const bool loopA,
                      const int flags, const rstl::string& name, const bool initialized);
#endif

  static rstl::string CreatePrimitiveName(const rstl::ncrc_ptr< CAnimTreeNode >& a,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                                          const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                          const float duration);
#else
                                          const rstl::ncrc_ptr< CAnimTreeNode >& b, float duration);
#endif

private:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  rstl::pair< CCharAnimTime, CAdvancementDeltas >
#else
  rstl::pair< CCharAnimTime, SAdvancementDeltas >
#endif

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
