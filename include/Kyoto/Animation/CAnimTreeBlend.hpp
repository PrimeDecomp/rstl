#ifndef _CANIMTREEBLEND
#define _CANIMTREEBLEND
#include "rstl/RstlVersions.h"

#include "Kyoto/Animation/CAnimTreeTweenBase.hpp"

class CAnimTreeBlend : public CAnimTreeTweenBase {
public:
#if !RSTL_PRIME1
  CAnimTreeBlend(const bool characterSpaceBlend, const rstl::ncrc_ptr< CAnimTreeNode >& a,
                 const rstl::ncrc_ptr< CAnimTreeNode >& b, float weight, const rstl::string& name)
  : CAnimTreeTweenBase(characterSpaceBlend, a, b, kBlendRoot_Offset | kBlendRoot_Rotation, name)
  , mBlendWeight(weight) {}

  // IAnimReader
#endif

  ~CAnimTreeBlend() override {}
  SAdvancementResults VAdvanceView(const CCharAnimTime& time) override;

  CCharAnimTime VGetTimeRemaining() const override;
  CSteadyStateAnimInfo VGetSteadyStateAnimInfo() const override;
  rstl::ownership_transfer< IAnimReader > VClone() const override;
  // CAnimTreeTweenBase
  void SetBlendingWeight(float weight) override;
  float VGetBlendingWeight() const override;
#if RSTL_PRIME1
  CAnimTreeBlend(const bool characterSpaceBlend, const rstl::ncrc_ptr< CAnimTreeNode >& a,
                 const rstl::ncrc_ptr< CAnimTreeNode >& b, float weight, const rstl::string& name)
  : CAnimTreeTweenBase(characterSpaceBlend, a, b, kBlendRoot_Offset | kBlendRoot_Rotation, name)
  , mBlendWeight(weight) {}
#endif

  static rstl::string CreatePrimitiveName(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                          const rstl::ncrc_ptr< CAnimTreeNode >& b, float weight);

private:
  float mBlendWeight;
};
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CHECK_SIZEOF(CAnimTreeBlend, 0x28)
#else
CHECK_CHILD_SIZEOF(CAnimTreeBlend, CAnimTreeTweenBase, 0x4)
#endif

#endif // _CANIMTREEBLEND
