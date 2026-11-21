#include "rstl/RstlVersions.h"
#if RSTL_KYOTO_PROTO_UNIT
#include "Kyoto/Animation/CAnimSysContext.hpp"

#include "Kyoto/Animation/CAnimPOIDatabase.hpp"
#include "Kyoto/Animation/CTransitionDatabase.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/IObjectStore.hpp"



CAnimSysContext::CAnimSysContext(const rstl::rc_ptr< CTransitionDatabase >& transDb,
                                 const rstl::ncrc_ptr< CRandom16 >& random, IObjectStore& store,
                                 const rstl::rc_ptr< IAnimPOIDatabase >& poiDb)
: mTransDb(transDb), mRandom(random), mStore(store), mPOIDatabase(poiDb) {}

const rstl::rc_ptr< CTransitionDatabase >& CAnimSysContext::GetTransitionDatabase() const {
  return mTransDb;
}

CRandom16& CAnimSysContext::GetRandomNumberGenerator() const { return *mRandom; }

IObjectStore& CAnimSysContext::GetObjectStore() const { return mStore; }

const rstl::rc_ptr< IAnimPOIDatabase >& CAnimSysContext::GetPOIDatabase() const {
  return mPOIDatabase;
}
#endif
