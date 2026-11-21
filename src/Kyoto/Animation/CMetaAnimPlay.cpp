#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/RstlVersions.h"
#endif
#include "Kyoto/Animation/CMetaAnimPlay.hpp"
#include "Kyoto/Animation/CAllFormatsAnimSource.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/Alloc/Assert.hpp"
#include "Kyoto/Animation/CAnimPOIDatabase.hpp"
#endif
#include "Kyoto/Animation/CAnimSysContext.hpp"
#include "Kyoto/Animation/CAnimTreeAnimReaderContainer.hpp"
#if !RSTL_CORRUPTION
#include "Kyoto/IObjectStore.hpp"
#endif
#include "Kyoto/CSimplePool.hpp"

#include "Kyoto/Streams/COutputStream.hpp"

rstl::ncrc_ptr< CAnimTreeNode >
CMetaAnimPlay::VGetAnimationTree(const CAnimSysContext& animSys,
                                 const CMetaAnimTreeBuildOrders& orders) const {
  if (orders.mRecursiveAdvance) {
    return GetAnimationTree(animSys,
                            CMetaAnimTreeBuildOrders::PreAdvanceForAll(*orders.mRecursiveAdvance));
  }

#if RSTL_CORRUPTION
  const SObjectTag tag('ANIM', CAssetId(mPrimitive.GetResourceId()));
  if (!animSys.GetObjectStore().ObjectIsLive(tag)) {
    rs_debugger_printf("Requested animation '%s' is not currently instanced -- LOADING\n",
                       mPrimitive.GetName().data());
  }
  TLockedToken< CAllFormatsAnimSource > anim = animSys.GetObjectStore().GetObj(tag);
  const CAnimPOIData* pAnimPOIData =
      animSys.GetPOIDatabase()->FindPOIDataByAssetId(mPrimitive.GetResourceId());
  RS_VERIFY_THROW(63, pAnimPOIData != NULL, 0,
                  "CMetaAnimPlay::VGetAnimationTree -- NULL Anim POI Data!");
#else
  IObjectStore& pool = animSys.GetSimplePool();
  const SObjectTag tag('ANIM', mPrimitive.GetResourceId());
  TLockedToken< CAllFormatsAnimSource > anim = pool.GetObj(tag);
#if !RSTL_PRIME1
  const int animIdx = mPrimitive.GetAnimDatabaseIndex();
#endif
#endif
  rstl::ownership_transfer< IAnimReader > reader =
#if RSTL_CORRUPTION
      CAllFormatsAnimSource::GetNewReader(anim, mStartTime, pAnimPOIData);
  return rs_new_line(68) CAnimTreeAnimReaderContainer(reader, mPrimitive);
#else
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
#endif
}

void CMetaAnimPlay::GetUniquePrimitives(rstl::set< CPrimitive >& primsOut) const {
  primsOut.insert(mPrimitive);
}

void CMetaAnimPlay::WriteAnimData(COutputStream& out) const {
  mPrimitive.PutTo(out);
  mStartTime.PutTo(out);
}
