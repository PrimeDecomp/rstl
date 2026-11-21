#ifndef _CCEANIMATIONSET
#define _CCEANIMATIONSET
#include "rstl/RstlVersions.h"

#include "rstl/static_assert.hpp"
#include "Kyoto/Animation/CAdditiveAnimationInfo.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/Animation/CAnimPOIDatabase.hpp"
#else
#include "Kyoto/Animation/CAnimPOIData.hpp"
#include "Kyoto/Animation/CAnimation.hpp"
#endif
#include "Kyoto/Animation/CHalfTransition.hpp"
#include "Kyoto/Animation/CTransition.hpp"
#if !RSTL_CORRUPTION
#include "Kyoto/Animation/IMetaAnim.hpp"
#endif
#include "Kyoto/Animation/IMetaTrans.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/CAssetId.hpp"
#endif

#if RSTL_CORRUPTION
#include "rstl/auto_ptr.hpp"
#else
#include "rstl/construct.hpp"
#endif
#include "rstl/pair.hpp"
#if RSTL_CORRUPTION
#include "rstl/rc_ptr.hpp"
#endif
#include "rstl/vector.hpp"
#if RSTL_CORRUPTION



class CFactoryFnReturn;
class CInputStream;
class CVParamTransfer;
struct SObjectTag;
#endif

class CCEAnimationSet {
public:
#if !RSTL_CORRUPTION
  typedef rstl::vector< CAnimation > AnimationList;
#endif
  typedef rstl::vector< CTransition > TransitionList;
  typedef rstl::vector< CHalfTransition > HalfTransitionList;
#if RSTL_CORRUPTION
  typedef rstl::vector< rstl::pair< CAssetId, CAdditiveAnimationInfo > > AdditiveAnimationList;
#else
  typedef rstl::vector< rstl::pair< uint, CAdditiveAnimationInfo > > AdditiveAnimationList;
  typedef rstl::vector< CAnimPOIData > EventSetList; // Guessed name.
#endif

  explicit CCEAnimationSet(CInputStream& in);
#if RSTL_CORRUPTION
  ~CCEAnimationSet() {}
#endif

#if !RSTL_CORRUPTION
  const AnimationList& GetAnimations() const { return mAnimations; }
#endif
  const TransitionList& GetTransitions() const { return mTransitions; }
  const HalfTransitionList& GetHalfTransitions() const { return mHalfTransitions; }
  const rstl::rc_ptr< IMetaTrans >& GetDefaultTransition() const { return mDefaultTransition; }
  const AdditiveAnimationList& GetAdditiveAnimInfoList() const { return mAdditiveAnimations; }
  const CAdditiveAnimationInfo& GetDefaultAdditiveAnimInfo() const {
    return mDefaultAdditiveAnimation;
  }
#if RSTL_CORRUPTION
  const CAnimPOIDatabase& GetPOIDatabase() const { return mPOIDatabase; } // Guessed name
#else
  const EventSetList& GetEventSets() const { return mEventSets; } // Guessed name.

  static AdditiveAnimationList StreamAdditiveAnimInfoList(ushort tableCount, CInputStream& in);
  static CAdditiveAnimationInfo StreamDefaultAdditiveAnimInfo(ushort tableCount, CInputStream& in);
  static HalfTransitionList StreamHalfTransitions(ushort tableCount, CInputStream& in);
  static EventSetList StreamEventSetList(ushort tableCount, CInputStream& in); // Guessed name.
#endif

private:
  ushort mTableCount;
#if !RSTL_CORRUPTION
  AnimationList mAnimations;
#endif
  TransitionList mTransitions;
#if RSTL_CORRUPTION
  HalfTransitionList mHalfTransitions;
#endif
  rstl::rc_ptr< IMetaTrans > mDefaultTransition;
  AdditiveAnimationList mAdditiveAnimations;
  CAdditiveAnimationInfo mDefaultAdditiveAnimation;
#if RSTL_CORRUPTION
  CAnimPOIDatabase mPOIDatabase;
#else
  HalfTransitionList mHalfTransitions;
  EventSetList mEventSets; // Guessed name: embedded animation event data.
#endif
};
#if RSTL_CORRUPTION
CHECK_SIZEOF(CCEAnimationSet, 0x5c)
#else
CHECK_SIZEOF(CCEAnimationSet, 0x64)
#endif

#if RSTL_CORRUPTION
CFactoryFnReturn FCEAnimationSetFactory(const SObjectTag& tag,
                                        const rstl::auto_ptr< uchar >& buffer, int size,
                                        const CVParamTransfer& xfer); // Guessed name


#endif
#endif // _CCEANIMATIONSET
