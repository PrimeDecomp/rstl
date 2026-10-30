#ifndef _CSIMPLEPOOL
#define _CSIMPLEPOOL
#include "rstl/RstlVersions.h"

#if RSTL_PRIME1
#include "Kyoto/CVParamTransfer.hpp"
#endif

#include "types.h"

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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  IFactory& GetFactory() const override { return mFactory; }
#else
  IFactory& GetFactory() override { return *mFactory; }
#endif
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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CHECK_SIZEOF(CSimplePool, (RSTL_VERSION >= RSTL_R3IJ ? 0x24 : 0x20))
#else
CHECK_SIZEOF(CSimplePool, 0x24)
#endif

extern CSimplePool* gpSimplePool;

#endif // _CSIMPLEPOOL
