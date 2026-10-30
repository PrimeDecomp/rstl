#ifndef _COBJECTREFERENCE
#define _COBJECTREFERENCE
#include "rstl/RstlVersions.h"

#if !RSTL_PRIME1
#include "Kyoto/CVParamTransfer.hpp"
#endif

#include "Kyoto/SObjectTag.hpp"
#if RSTL_PRIME1
#include "Kyoto/CVParamTransfer.hpp"
#endif

#include "rstl/auto_ptr.hpp"

class IObj;
class IObjectStore;
class CObjectReference {
public:
  CObjectReference(const rstl::auto_ptr< IObj >& obj);
  ~CObjectReference();
  CObjectReference(IObjectStore& store, const rstl::auto_ptr< IObj >& obj, const SObjectTag& tag,
                   CVParamTransfer xfer);

  bool IsLoaded() const { return mObject != nullptr; }

  void AddReference() { mRefCount++; }
  int RemoveReference();
  void Lock();
  void Unlock();
  IObj* GetObject();
  void Unload();
  void CancelLoad();
  bool IsLoading() const;
  const SObjectTag& GetObjectTag() const { return mObjTag; }

private:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  short mRefCount : 16;
  short mLockCount : 15;
  short mLoading : 1;
#else
  int mRefCount;
  int mLockCount;
  int mLoading;
#endif

  SObjectTag mObjTag;
  IObjectStore* mObjectStore;
  IObj* mObject;
  CVParamTransfer mParams;
};
#if !RSTL_PRIME1
CHECK_SIZEOF(CObjectReference, 0x24)
#endif

#endif // _COBJECTREFERENCE
