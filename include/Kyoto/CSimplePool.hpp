#ifndef _CSIMPLEPOOL
#define _CSIMPLEPOOL
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"

#if RSTL_PRIME1
#include "Kyoto/CVParamTransfer.hpp"
#endif

#include "rstl/types.h"

#include "rstl/map.hpp"
#include "rstl/vector.hpp"

#include "Kyoto/CToken.hpp"
#include "Kyoto/IObjectStore.hpp"

class IFactory;
class CSimplePool : public IObjectStore {
  struct TagIdLess {
    bool operator()(const SObjectTag& a, const SObjectTag& b) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      return a.GetId() < b.GetId();
#elif RSTL_KYOTO_POOL_TAG_TYPE_TIEBREAK
      if (a.id < b.id) {
        return true;
      }
      if (a.id > b.id) {
        return false;
      }
      return a.type < b.type;
#else
      return a.id < b.id;
#endif
    }
  };
  typedef rstl::map< SObjectTag, CObjectReference*, TagIdLess > ResourceMap;

public:
  CSimplePool(IFactory& factory);
  ~CSimplePool();
  void DebugDumpPool() const;
  // IObjectStore
  CToken GetObj(const SObjectTag& tag, const CVParamTransfer& xfer) override;
  CToken GetObj(const SObjectTag& tag) override;
  CToken GetObj(const char* name) override;
  CToken GetObj(const char* name, const CVParamTransfer& xfer) override;
  bool HasObject(const SObjectTag& tag) const override;
  bool ObjectIsLive(const SObjectTag& tag) const override;
  IFactory& GetFactory() RSTL_PRIME1_CONST override {
#if RSTL_PRIME1
    return mFactory;
#else
    return *mFactory;
#endif
  }
  void Flush() override;
  void ObjectUnreferenced(const SObjectTag& tag) override;

  rstl::vector< SObjectTag > GetReferencedTags() const;

private:
  ResourceMap mResources;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  IFactory& mFactory;
#else
  IFactory* mFactory;
#endif

  CVParamTransfer mParamXfr;
};
CHECK_SIZEOF(CSimplePool, (RSTL_PRIME1 && RSTL_VERSION < RSTL_R3IJ) ? 0x20 : 0x24)

extern CSimplePool* gpSimplePool;

#endif // _CSIMPLEPOOL
