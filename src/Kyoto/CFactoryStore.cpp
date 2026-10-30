#include "rstl/RstlVersions.h"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/IObjectStore.hpp"

// Guessed name, following the Prime counterpart.
struct CDummyFactory : public IObjectStore {
public:
  CDummyFactory(IFactory& factory) : mFactory(factory) {}

  CToken GetObj(const SObjectTag& tag, const CVParamTransfer& xfer) override { return CToken(); }
  CToken GetObj(const SObjectTag& tag) override { return CToken(); }
  CToken GetObj(const char* name) override { return CToken(); }
  CToken GetObj(const char* name, const CVParamTransfer& xfer) override { return CToken(); }
  bool HasObject(const SObjectTag& tag) const override { return false; }
  bool ObjectIsLive(const SObjectTag& tag) const override { return false; }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  IFactory& GetFactory() const override { return mFactory; }
#else
  IFactory& GetFactory() override { return mFactory; }
#endif
  void Flush() override {}
  void ObjectUnreferenced(const SObjectTag& tag) override {}

private:
  IFactory& mFactory;
};

TObjOwnerParam< IObjectStore* > DumbTest() {
  CResFactory tmp;
  CDummyFactory pool(tmp);
  return TObjOwnerParam< IObjectStore* >(&pool);
}

