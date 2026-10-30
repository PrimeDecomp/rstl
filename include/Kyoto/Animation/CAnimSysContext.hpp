#ifndef _CANIMSYSCONTEXT
#define _CANIMSYSCONTEXT
#include "rstl/RstlVersions.h"

#if !RSTL_PRIME1
#include "Kyoto/Animation/CCEAnimationSet.hpp"
#endif

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/rc_ptr.hpp"

class CTransitionDatabase;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
class IObjectStore;
#else
class CSimplePool;
#endif

class CAnimSysContext {
public:
  CAnimSysContext(const TToken< CTransitionDatabase >& transDb,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                  const rstl::ncrc_ptr< CRandom16 >& random, IObjectStore& store)
  : mTransDb(transDb), mRandom(random), mStore(store) {}
#else
                  const rstl::ncrc_ptr< CRandom16 >& random, CSimplePool& store,
                  const CCEAnimationSet::EventSetList& eventSets)
  : mTransDb(transDb), mRandom(random), mStore(store), mEventSets(eventSets) {}
#endif

#if RSTL_PRIME1
  IObjectStore& GetSimplePool() const { return mStore; }
#endif

  const TToken< CTransitionDatabase >& GetTransitionDatabase() const { return mTransDb; }
  CRandom16& GetRandomNumberGenerator() const { return *mRandom; }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CRandom16* Random() { return mRandom.GetPtr(); }
  const CRandom16* GetRandom() const { return mRandom.GetPtr(); }
#else
  CSimplePool& GetSimplePool() const { return mStore; }
  const CAnimPOIData* GetEventData(int animIdx) const {
    return &mEventSets[animIdx];
  } // Guessed name.
#endif

private:
  TToken< CTransitionDatabase > mTransDb;
  rstl::ncrc_ptr< CRandom16 > mRandom;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  IObjectStore& mStore;
#else
  CSimplePool& mStore;
  const CCEAnimationSet::EventSetList& mEventSets; // Guessed name.
#endif

};
#if !RSTL_PRIME1 || RSTL_VERSION < RSTL_R3IJ
CHECK_SIZEOF(CAnimSysContext, RSTL_PRIME1 ? 0x10 : 0x18)
#endif

#endif // _CANIMSYSCONTEXT
