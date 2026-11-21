#include "rstl/RstlVersions.h"
#if RSTL_CORRUPTION
#include "Kyoto/Animation/CAnimationDatabase.hpp"

#include "Kyoto/Alloc/Assert.hpp"
#include "Kyoto/Animation/CAnimation.hpp"
#include "Kyoto/Animation/CPrimitive.hpp"
#include "Kyoto/Animation/IMetaAnim.hpp"



CAnimationDatabase::CAnimationDatabase(const rstl::vector< CAnimation >& animations) {
  rstl::vector< CAnimation >::const_iterator it = animations.begin();
  rstl::vector< CAnimation >::const_iterator end = animations.end();
  mMetaAnims.reserve(animations.size());
  for (; it != end; ++it) {
    mMetaAnims.push_back_unsafe(it->GetMetaAnim());
  }
}

rstl::string CAnimationDatabase::GetMetaAnimName(uint idx) const {
  return rstl::string_l("Meta-animation name unavailable in Release mode.");
}

rstl::rc_ptr< IMetaAnim > CAnimationDatabase::GetMetaAnim(uint idx) const {
  if (idx < mMetaAnims.size()) {
    return mMetaAnims[idx];
  }

  return rstl::rc_ptr< IMetaAnim >(nullptr);
}

void CAnimationDatabase::GetUniquePrimitivesFromMetaAnim(rstl::set< CPrimitive >& primsOut,
                                                         const rstl::string& name) const {
  RS_VERIFY_THROW(132, false, false,
                  "CAnimationDatabase::GetUniquePrimitivesFromMetaAnim(rstl::set, rstl::string) "
                  "should not be used!");
}
#endif
