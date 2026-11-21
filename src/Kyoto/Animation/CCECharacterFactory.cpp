#include "rstl/RstlVersions.h"
#if RSTL_KYOTO_PROTO_GAME_HEADERS
#define RS_TTOKEN_FILE "TToken.H"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Animation/CCECharacterFactory.hpp"

#include "Kyoto/Alloc/Assert.hpp"
#include "Kyoto/Animation/CAnimPOIDatabase.hpp"
#include "Kyoto/Animation/CAnimationDatabase.hpp"
#include "Kyoto/Animation/CCharLayoutInfo.hpp"
#include "Kyoto/Animation/CSkinnedModel.hpp"
#include "Kyoto/CFactoryMgr.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/CVParamTransfer.hpp"
#include "Corruption/MetroidPrime/CAnimData.hpp"
#include "Corruption/MetroidPrime/CTransitionDatabaseGame.hpp"



CCECharacterFactory::CDummyFactory::CDummyFactory(IObjectStore& store) : mStore(&store) {}

rstl::auto_ptr< IObj > CCECharacterFactory::CDummyFactory::Build(const SObjectTag& tag,
                                                                 const CVParamTransfer& params) {
  const int factoryOption = tag.GetType();
  const CVParamTransfer paramCopy(params);
  const CCECharacterInfo& charInfo =
      *static_cast< const TObjOwnerParam< const CCECharacterInfo* >& >(*paramCopy).GetData();
  RS_VERIFY_THROW(
      47, factoryOption >= kFactoryOption_FirstValid && factoryOption <= kFactoryOption_LastValid,
      "Invalid character factory build option!", NULL);
  switch (factoryOption) {
  case kFactoryOption_SkinnedModel:
    return CFactoryFnReturn(rs_new_line(52) CSkinnedModel(
                                mStore->GetObj(SObjectTag('CMDL', charInfo.GetModelId())),
                                mStore->GetObj(SObjectTag('CSKR', charInfo.GetSkinRulesId())),
                                mStore->GetObj(SObjectTag('CINF', charInfo.GetCharLayoutInfoId()))))
        .GetObjForTransfer();
  }
  return rstl::auto_ptr< IObj >();
}

void CCECharacterFactory::CDummyFactory::BuildAsync(const SObjectTag& tag,
                                                    const CVParamTransfer& params, IObj** out) {
  *out = Build(tag, params).release();
}

void CCECharacterFactory::CDummyFactory::CancelBuild(const SObjectTag&) {}

CCECharacterFactory::CCECharacterFactory(IObjectStore& store,
                                         const TLockedToken< CCECharacterInfo >& charInfo,
                                         const TLockedToken< CCEAnimationSet >& animSet)
: mStore(store)
, mLayoutInfo(store.GetObj(SObjectTag('CINF', charInfo->GetCharLayoutInfoId())))
, mDummyFactory(store)
, mCacheResPool(mDummyFactory)
, mCharInfo(charInfo)
, mAnimSet(animSet) {
  const rstl::vector< CAnimation >& animations = charInfo->GetAnimations();
  const CCEAnimationSet::TransitionList& transitions = animSet->GetTransitions();
  const CCEAnimationSet::HalfTransitionList& halfTransitions = animSet->GetHalfTransitions();
  const rstl::rc_ptr< IMetaTrans > defaultTrans = animSet->GetDefaultTransition();
  const rstl::rc_ptr< CAnimationDatabase > animDB(rs_new_line(96) CAnimationDatabase(animations));
  const rstl::rc_ptr< CTransitionDatabase > transDB(rs_new_line(97) CTransitionDatabaseGame(
      transitions, halfTransitions, animSet->GetAdditiveAnimInfoList(), defaultTrans,
      animSet->GetDefaultAdditiveAnimInfo()));
  const rstl::ncrc_ptr< CRandom16 > random(rs_new_line(105) CRandom16(2334));
  const rstl::rc_ptr< IAnimPOIDatabase > poiDB(rs_new_line(108)
                                                   CAnimPOIDatabase(animSet->GetPOIDatabase()));
  mSysContext = rstl::ncrc_ptr< CAnimSysContext >(
      rs_new_line(109) CAnimSysContext(transDB, random, store, poiDB));
  mAnimMgr =
      rstl::ncrc_ptr< CAnimationManager >(rs_new_line(112) CAnimationManager(animDB, *mSysContext));
  mTransMgr =
      rstl::ncrc_ptr< CTransitionManager >(rs_new_line(115) CTransitionManager(*mSysContext));
}

rstl::auto_ptr< CAnimData >
CCECharacterFactory::CreateCharacter(TLockedToken< CCECharacterFactory > factory, bool loop,
                                     int defaultAnim) {
  const CCECharacterInfo* charInfo = *mCharInfo;
  const SObjectTag modelTag(kFactoryOption_SkinnedModel, charInfo->GetModelId());
  TToken< CSkinnedModel > skinnedModel = mCacheResPool.GetObj(
      modelTag,
      CVParamTransfer(rs_new_line(134) TObjOwnerParam< const CCECharacterInfo* >(charInfo)));
  rstl::optional_object< TLockedToken< CSkinnedModel > > iceModel;
  return rs_new_line(167) CAnimData(factory, &mStore, charInfo, defaultAnim,
                                    charInfo->GetCharacterIndex(), loop, mLayoutInfo, skinnedModel,
                                    iceModel, mSysContext, mAnimMgr, charInfo->GetAnimatedScale());
}

const CCECharacterInfo& CCECharacterFactory::GetCharacterInfo() const { return **mCharInfo; }

CCECharacterFactory::~CCECharacterFactory() {}
#endif
