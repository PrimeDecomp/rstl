#ifndef _CCECHARACTERFACTORYBUILDER
#define _CCECHARACTERFACTORYBUILDER

#include "rstl/static_assert.hpp"
#include "rstl/types.h"

#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"



class IObjectStore;
class CAnimRes;
#include "Kyoto/Animation/CCECharacterFactory.hpp"

class CCECharacterFactoryBuilder {
public:
  class CDummyFactory : public IFactory {
  public:
    explicit CDummyFactory(IObjectStore& store);
    ~CDummyFactory();
    rstl::auto_ptr< IObj > Build(const SObjectTag& tag, const CVParamTransfer& params) override;
    void BuildAsync(const SObjectTag& tag, const CVParamTransfer& params, IObj** out) override;
    void CancelBuild(const SObjectTag& tag) override;
    bool CanBuild(const SObjectTag&) override;
    const SObjectTag* GetResourceIdByName(const char*) const override;

  private:
    IObjectStore* mStore; // Guessed name
  };

  CCECharacterFactoryBuilder(IObjectStore& store);
  ~CCECharacterFactoryBuilder();
  TToken< CCECharacterFactory > GetFactory(const CAnimRes& res);

private:
  CDummyFactory mDummyFactory;
  CSimplePool mDummyStore;
};
CHECK_SIZEOF(CCECharacterFactoryBuilder, 0x2c)

extern CCECharacterFactoryBuilder* gpCharacterFactoryBuilder;


#endif // _CCECHARACTERFACTORYBUILDER
