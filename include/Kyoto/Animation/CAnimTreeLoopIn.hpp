#ifndef _CANIMTREELOOPIN
#define _CANIMTREELOOPIN
#include "rstl/RstlVersions.h"
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"

#if RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#endif
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
#if RSTL_CORRUPTION
                       uint iterator, const CSegIdMaskMap& mask) const override;
#else
                       uint iterator, int unk) const override;
  uint VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, uint capacity,
                        uint iterator, int unk) const override;
#endif
  uint VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, uint capacity,
#if RSTL_CORRUPTION
                           uint iterator, const CSegIdMaskMap& mask) const override;
#else
                           uint iterator, int unk) const override;
#endif
  uint VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, uint capacity,
#if RSTL_CORRUPTION
                        uint iterator, const CSegIdMaskMap& mask) const override;
  const CVisibilityAnimationDataReader* VGetVisibilityDataAtCurrentTime() const override;
  const CVisibilityAnimationDataReader*
  VGetVisibilityDataAtTime(const CCharAnimTime& time) const override;
#else
                        uint iterator, int unk) const override;
#endif
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
#if RSTL_CORRUPTION
                  const rstl::ncrc_ptr< CAnimTreeNode >& b, const bool didLoopIn,
#else
                  const rstl::ncrc_ptr< CAnimTreeNode >& b, RSTL_ECHOES_CONST bool didLoopIn,
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
#if RSTL_CORRUPTION
CHECK_SIZEOF(CAnimTreeLoopIn, 0xd4)
#else
CHECK_CHILD_SIZEOF(CAnimTreeLoopIn, CAnimTreeSingleChild, RSTL_PRIME1 ? 0x78 : (RSTL_VERSION >= RSTL_R32J ? 0x74 : 0x84))
#endif

#endif // _CANIMTREELOOPIN
