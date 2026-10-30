#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CMetaAnimPlay.hpp"
#include "Kyoto/Animation/CAllFormatsAnimSource.hpp"
#include "Kyoto/Animation/CAnimSysContext.hpp"
#include "Kyoto/Animation/CAnimTreeAnimReaderContainer.hpp"
#include "Kyoto/IObjectStore.hpp"
#include "Kyoto/CSimplePool.hpp"

#include "Kyoto/Streams/COutputStream.hpp"

rstl::ncrc_ptr< CAnimTreeNode >
CMetaAnimPlay::VGetAnimationTree(const CAnimSysContext& animSys,
                                 const CMetaAnimTreeBuildOrders& orders) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (orders.mRecursiveAdvance)
    return GetAnimationTree(
        animSys, CMetaAnimTreeBuildOrders::PreAdvanceForAll(*orders.mRecursiveAdvance));
#else
  if (orders.mRecursiveAdvance) {
    return GetAnimationTree(animSys,
                            CMetaAnimTreeBuildOrders::PreAdvanceForAll(*orders.mRecursiveAdvance));
  }
#endif

  IObjectStore& pool = animSys.GetSimplePool();
  const SObjectTag tag('ANIM', mPrimitive.GetResourceId());
  TLockedToken< CAllFormatsAnimSource > anim = pool.GetObj(tag);
#if !RSTL_PRIME1
  const int animIdx = mPrimitive.GetAnimDatabaseIndex();
#endif

  rstl::ownership_transfer< IAnimReader > reader =
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      CAllFormatsAnimSource::GetNewReader(anim, mStartTime);
  return rs_new CAnimTreeAnimReaderContainer(reader, mPrimitive.GetName(),
                                             mPrimitive.GetAnimDatabaseIndex());
#else
      CAllFormatsAnimSource::GetNewReader(anim, mStartTime, animSys.GetEventData(animIdx));
  return rs_new CAnimTreeAnimReaderContainer(reader, mPrimitive.GetName(), animIdx);
#endif

}

void CMetaAnimPlay::GetUniquePrimitives(rstl::set< CPrimitive >& primsOut) const {
  primsOut.insert(mPrimitive);
}

void CMetaAnimPlay::WriteAnimData(COutputStream& out) const {
  mPrimitive.PutTo(out);
  mStartTime.PutTo(out);
}

