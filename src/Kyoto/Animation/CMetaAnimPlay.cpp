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
  if (orders.mRecursiveAdvance) {
    return GetAnimationTree(animSys,
                            CMetaAnimTreeBuildOrders::PreAdvanceForAll(*orders.mRecursiveAdvance));
  }

  IObjectStore& pool = animSys.GetSimplePool();
  const SObjectTag tag('ANIM', mPrimitive.GetResourceId());
  TLockedToken< CAllFormatsAnimSource > anim = pool.GetObj(tag);
#if !RSTL_PRIME1
  const int animIdx = mPrimitive.GetAnimDatabaseIndex();
#endif
  rstl::ownership_transfer< IAnimReader > reader =
      CAllFormatsAnimSource::GetNewReader(anim, mStartTime
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
                                          , animSys.GetEventData(animIdx)
#endif
                                          );
  return rs_new CAnimTreeAnimReaderContainer(reader, mPrimitive.GetName()
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                                             , mPrimitive.GetAnimDatabaseIndex()
#else
                                             , animIdx
#endif
                                             );
}

void CMetaAnimPlay::GetUniquePrimitives(rstl::set< CPrimitive >& primsOut) const {
  primsOut.insert(mPrimitive);
}

void CMetaAnimPlay::WriteAnimData(COutputStream& out) const {
  mPrimitive.PutTo(out);
  mStartTime.PutTo(out);
}

