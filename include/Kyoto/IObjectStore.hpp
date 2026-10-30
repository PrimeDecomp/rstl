#ifndef _IOBJECTSTORE
#define _IOBJECTSTORE
#include "rstl/RstlVersions.h"

#include "types.h"

#include "Kyoto/CToken.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/rc_ptr.hpp"

class IFactory;
class IObjectStore {
public:
  IObjectStore() {}
  virtual CToken GetObj(const SObjectTag& tag, const CVParamTransfer& xfer) = 0;
  virtual CToken GetObj(const SObjectTag& tag) = 0;
  virtual CToken GetObj(const char* name) = 0;
  virtual CToken GetObj(const char* name, const CVParamTransfer& xfer) = 0;
  virtual bool HasObject(const SObjectTag& tag) const = 0;
  virtual bool ObjectIsLive(const SObjectTag& tag) const = 0;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  virtual IFactory& GetFactory() const = 0;
#else
  virtual IFactory& GetFactory() = 0;
#endif

  virtual void Flush() = 0;
  virtual void ObjectUnreferenced(const SObjectTag& tag) = 0;
};

#endif // _IOBJECTSTORE
