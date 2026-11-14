#ifndef _IOBJECTSTORE
#define _IOBJECTSTORE
#include "rstl/RstlVersions.h"

#include "rstl/types.h"

#include "Kyoto/CToken.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/rc_ptr.hpp"

class IFactory;
class IObjectStore {
public:
  IObjectStore() {}
#if RSTL_KYOTO_OBJECT_STORE_VIRTUAL_DTOR
  virtual ~IObjectStore() = 0;
#endif
  virtual CToken GetObj(const SObjectTag& tag, const CVParamTransfer& xfer) = 0;
  virtual CToken GetObj(const SObjectTag& tag) = 0;
  virtual CToken GetObj(const char* name) = 0;
  virtual CToken GetObj(const char* name, const CVParamTransfer& xfer) = 0;
  virtual bool HasObject(const SObjectTag& tag) const = 0;
  virtual bool ObjectIsLive(const SObjectTag& tag) const = 0;
  virtual IFactory& GetFactory() RSTL_PRIME1_CONST = 0;

  virtual void Flush() = 0;
  virtual void ObjectUnreferenced(const SObjectTag& tag) = 0;
};
#if RSTL_KYOTO_OBJECT_STORE_VIRTUAL_DTOR
inline IObjectStore::~IObjectStore() {}
#endif

#endif // _IOBJECTSTORE
