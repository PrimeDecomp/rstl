#include "rstl/RstlVersions.h"
#define RS_TTOKEN_FILE "TToken.H"

#include "Kyoto/Animation/CCEAnimationSet.hpp"

#if RSTL_CORRUPTION
#include "Kyoto/Alloc/CMemory.hpp"
#endif
#include "Kyoto/Animation/CMetaTransFactory.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/CFactoryMgr.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"


#endif

CCEAnimationSet::CCEAnimationSet(CInputStream& in)
: mTableCount(in.Get< ushort >())
#if !RSTL_CORRUPTION
, mAnimations(in)
#endif
, mTransitions(in)
#if RSTL_CORRUPTION
, mHalfTransitions(in)
#endif
, mDefaultTransition(CMetaTransFactory::CreateMetaTrans(in))
#if RSTL_CORRUPTION
, mAdditiveAnimations(in)
, mDefaultAdditiveAnimation(in)
, mPOIDatabase(in) {}
#else
, mAdditiveAnimations(StreamAdditiveAnimInfoList(mTableCount, in))
, mDefaultAdditiveAnimation(StreamDefaultAdditiveAnimInfo(mTableCount, in))
, mHalfTransitions(StreamHalfTransitions(mTableCount, in))
, mEventSets(StreamEventSetList(mTableCount, in)) {}
#endif

#if RSTL_CORRUPTION
CFactoryFnReturn FCEAnimationSetFactory(const SObjectTag& tag,
                                        const rstl::auto_ptr< uchar >& buffer, int size,
                                        const CVParamTransfer& xfer) {
  CMemoryInStream in(buffer.get(), size);
  return rs_new_line(35) CCEAnimationSet(in);
#else
CCEAnimationSet::AdditiveAnimationList
CCEAnimationSet::StreamAdditiveAnimInfoList(ushort tableCount, CInputStream& in) {
  if (tableCount > 1) {
    return AdditiveAnimationList(in);
#endif
  }
#if !RSTL_CORRUPTION

  return AdditiveAnimationList();
}

CAdditiveAnimationInfo CCEAnimationSet::StreamDefaultAdditiveAnimInfo(ushort tableCount,
                                                                      CInputStream& in) {
  if (tableCount > 1) {
    return CAdditiveAnimationInfo(in);
  }

  return CAdditiveAnimationInfo(0.f, 0.f);
}

CCEAnimationSet::HalfTransitionList CCEAnimationSet::StreamHalfTransitions(ushort tableCount,
                                                                           CInputStream& in) {
  if (tableCount > 2) {
    return HalfTransitionList(in);
  }

  return HalfTransitionList();
}

// Guessed name.
CCEAnimationSet::EventSetList CCEAnimationSet::StreamEventSetList(ushort tableCount,
                                                                  CInputStream& in) {
  if (tableCount > 3) {
    return EventSetList(in);
  }

  return EventSetList();
}
#endif
