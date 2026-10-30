#ifndef _CANIMTREELOOPIN
#define _CANIMTREELOOPIN
#include "rstl/RstlVersions.h"

#include "Kyoto/Animation/CAnimSysContext.hpp"
#include "Kyoto/Animation/CAnimTreeSingleChild.hpp"
#include "Kyoto/Animation/CSequenceFundamentals.hpp"

class CAnimTreeLoopIn : public CAnimTreeSingleChild {
public:
  // IAnimReader
  ~CAnimTreeLoopIn() override {}
  SAdvancementResults VAdvanceView(const CCharAnimTime& time) override;

  CCharAnimTime VGetTimeRemaining() const override;
  CSteadyStateAnimInfo VGetSteadyStateAnimInfo() const override;
  uint VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, uint capacity,
                       uint iterator, int unk) const override;
  uint VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, uint capacity,
                        uint iterator, int unk) const override;
  uint VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, uint capacity,
                           uint iterator, int unk) const override;
  uint VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, uint capacity,
                        uint iterator, int unk) const override;
  rstl::ownership_transfer< IAnimReader > VClone() const override;
  rstl::optional_object< rstl::ownership_transfer< IAnimReader > > VSimplified() override;
  // CAnimTreeNode
  CAnimTreeEffectiveContribution VGetContributionOfHighestInfluence() const override;
  rstl::rc_ptr< CAnimTreeNode > VGetBestUnblendedChild() const override;
  virtual bool VSupportsReverseView() const;

  CAnimTreeLoopIn(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                  const rstl::ncrc_ptr< CAnimTreeNode >& b,
                  const rstl::ncrc_ptr< CAnimTreeNode >& c, const CAnimSysContext& animCtx,
                  const rstl::string& name);
  CAnimTreeLoopIn(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                  const rstl::ncrc_ptr< CAnimTreeNode >& b,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                  bool didLoopIn,
#else
                  const bool didLoopIn,
#endif
                  const CAnimSysContext& animCtx, const rstl::string& name,
                  const CSequenceFundamentals& fundamentals, const CCharAnimTime& time);

  static rstl::string CreatePrimitiveName(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                          const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                          const rstl::ncrc_ptr< CAnimTreeNode >& c);

private:
  rstl::ncrc_ptr< CAnimTreeNode > mNextAnim;
  bool mDidLoopIn;
  CAnimSysContext mAnimCtx;
  CSequenceFundamentals mFundamentals;
  CCharAnimTime mCurTime;
};
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CHECK_SIZEOF(CAnimTreeLoopIn, 0x90)
#else
CHECK_CHILD_SIZEOF(CAnimTreeLoopIn, CAnimTreeSingleChild, RSTL_VERSION >= RSTL_R32J ? 0x74 : 0x84)
#endif

#endif // _CANIMTREELOOPIN
