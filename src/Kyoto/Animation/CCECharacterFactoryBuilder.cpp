#include "rstl/RstlVersions.h"
#if RSTL_KYOTO_PROTO_GAME_HEADERS
#define RS_TTOKEN_FILE "TToken.H"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Animation/CCECharacterFactoryBuilder.hpp"

#include "Kyoto/Animation/CCEAnimationSet.hpp"
#include "Kyoto/Animation/CCECharacterInfo.hpp"
#include "Kyoto/CFactoryMgr.hpp"
#include "Kyoto/CVParamTransfer.hpp"
#include "Corruption/MetroidPrime/CAnimRes.hpp"



inline CCECharacterFactoryBuilder::CDummyFactory::~CDummyFactory() {}

const SObjectTag*
CCECharacterFactoryBuilder::CDummyFactory::GetResourceIdByName(const char*) const {
  return nullptr;
}

bool CCECharacterFactoryBuilder::CDummyFactory::CanBuild(const SObjectTag&) { return true; }

CCECharacterFactoryBuilder::CDummyFactory::CDummyFactory(IObjectStore& store) : mStore(&store) {}

rstl::auto_ptr< IObj >
CCECharacterFactoryBuilder::CDummyFactory::Build(const SObjectTag& tag,
                                                 const CVParamTransfer& params) {
  CAssetId id = tag.GetId();
  TToken< CCECharacterInfo > charInfo = mStore->GetObj(SObjectTag('CHAR', id));
  TToken< CCEAnimationSet > animSet = mStore->GetObj(SObjectTag('SAND', charInfo->GetAnimSetId()));
  return CFactoryFnReturn(rs_new_line(38) CCECharacterFactory(
                              *mStore, TLockedToken< CCECharacterInfo >(charInfo),
                              TLockedToken< CCEAnimationSet >(animSet)))
      .GetObjForTransfer();
}

void CCECharacterFactoryBuilder::CDummyFactory::BuildAsync(const SObjectTag& tag,
                                                           const CVParamTransfer& params,
                                                           IObj** out) {
  *out = Build(tag, params).release();
}

void CCECharacterFactoryBuilder::CDummyFactory::CancelBuild(const SObjectTag&) {}

CCECharacterFactoryBuilder::CCECharacterFactoryBuilder(IObjectStore& store)
: mDummyFactory(store), mDummyStore(mDummyFactory) {}

CCECharacterFactoryBuilder::~CCECharacterFactoryBuilder() {}

TToken< CCECharacterFactory > CCECharacterFactoryBuilder::GetFactory(const CAnimRes& res) {
  return mDummyStore.GetObj(SObjectTag('CHAR', res.GetId()), CVParamTransfer::Null());
}
#endif
