#ifndef _CCECHARACTERFACTORY
#define _CCECHARACTERFACTORY

#include "rstl/static_assert.hpp"
#include "rstl/types.h"

#include "Kyoto/Animation/CAnimSysContext.hpp"
#include "Kyoto/Animation/CAnimationManager.hpp"
#include "Kyoto/Animation/CCEAnimationSet.hpp"
#include "Kyoto/Animation/CCECharacterInfo.hpp"
#include "Kyoto/Animation/CTransitionManager.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/IObjFactory.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/rc_ptr.hpp"



class CAnimData;
class CCharLayoutInfo;
class IObjectStore;

class CCECharacterFactory : public IObjFactory {
public:
  enum EFactoryOption {              // Guessed name
    kFactoryOption_SkinnedModel = 0, // Guessed name
    kFactoryOption_FirstValid = kFactoryOption_SkinnedModel,
    kFactoryOption_LastValid = 1
  };

  class CDummyFactory : public IFactory {
  public:
    explicit CDummyFactory(IObjectStore& store);
    ~CDummyFactory() override {}
    rstl::auto_ptr< IObj > Build(const SObjectTag& tag, const CVParamTransfer& params) override;
    void BuildAsync(const SObjectTag& tag, const CVParamTransfer& params, IObj** out) override;
    void CancelBuild(const SObjectTag& tag) override;
    bool CanBuild(const SObjectTag&) override { return true; }
    const SObjectTag* GetResourceIdByName(const char*) const override { return nullptr; }

  private:
    IObjectStore* mStore; // Guessed name
  };

  CCECharacterFactory(IObjectStore& store, const TLockedToken< CCECharacterInfo >& charInfo,
                      const TLockedToken< CCEAnimationSet >& animSet);
  ~CCECharacterFactory() override;

  rstl::auto_ptr< CAnimData > CreateCharacter(TLockedToken< CCECharacterFactory > factory,
                                              bool loop, int defaultAnim);
  const CCECharacterInfo& GetCharacterInfo() const;

private:
  IObjectStore& mStore;                           // Guessed name
  TLockedToken< CCharLayoutInfo > mLayoutInfo;    // Guessed name
  rstl::ncrc_ptr< CAnimSysContext > mSysContext;  // Guessed name
  rstl::ncrc_ptr< CAnimationManager > mAnimMgr;   // Guessed name
  rstl::ncrc_ptr< CTransitionManager > mTransMgr; // Guessed name
  CDummyFactory mDummyFactory;                    // Guessed name
  CSimplePool mCacheResPool;                      // Guessed name
  TLockedToken< CCECharacterInfo > mCharInfo;     // Guessed name
  TLockedToken< CCEAnimationSet > mAnimSet;       // Guessed name
};
CHECK_SIZEOF(CCECharacterFactory, 0x70)


#endif // _CCECHARACTERFACTORY
