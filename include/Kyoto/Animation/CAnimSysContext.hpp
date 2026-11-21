#ifndef _CANIMSYSCONTEXT
#define _CANIMSYSCONTEXT
#include "rstl/RstlVersions.h"
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"

#if RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#include "rstl/types.h"
#else
#if !RSTL_PRIME1
#include "Kyoto/Animation/CCEAnimationSet.hpp"
#endif
#endif

#if RSTL_CORRUPTION
#include "Kyoto/Animation/CAnimPOIDatabase.hpp"
#include "Kyoto/Animation/CTransitionDatabase.hpp"
#endif
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/rc_ptr.hpp"

#if !RSTL_CORRUPTION
class CTransitionDatabase;
#endif
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_CORRUPTION

class IObjectStore;
#else
#if RSTL_PRIME1
class IObjectStore;
#else
class CSimplePool;
#endif
#endif

class CAnimSysContext {
public:
#if RSTL_CORRUPTION
  CAnimSysContext(const rstl::rc_ptr< CTransitionDatabase >& transDb,
                  const rstl::ncrc_ptr< CRandom16 >& random, IObjectStore& store,
                  const rstl::rc_ptr< IAnimPOIDatabase >& poiDb);
#else
  CAnimSysContext(const TToken< CTransitionDatabase >& transDb,
#endif
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_CORRUPTION
  const rstl::rc_ptr< IAnimPOIDatabase >& GetPOIDatabase() const; // Guessed name
  IObjectStore& GetObjectStore() const;                           // Guessed name
  CRandom16& GetRandomNumberGenerator() const;
  const rstl::rc_ptr< CTransitionDatabase >& GetTransitionDatabase() const;
#else
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
#endif

private:
#if RSTL_CORRUPTION
  rstl::ncrc_ptr< CTransitionDatabase > mTransDb;
#else
  TToken< CTransitionDatabase > mTransDb;
#endif
  rstl::ncrc_ptr< CRandom16 > mRandom;
#if RSTL_CORRUPTION
  IObjectStore& mStore;
  rstl::ncrc_ptr< IAnimPOIDatabase > mPOIDatabase;
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  IObjectStore& mStore;
#else
  CSimplePool& mStore;
  const CCEAnimationSet::EventSetList& mEventSets; // Guessed name.
#endif

#endif
};
#if RSTL_CORRUPTION
CHECK_SIZEOF(CAnimSysContext, 0x1c)
#else
CHECK_SIZEOF(CAnimSysContext, RSTL_PRIME1 ? 0x10 : 0x18)
#endif

#endif // _CANIMSYSCONTEXT
