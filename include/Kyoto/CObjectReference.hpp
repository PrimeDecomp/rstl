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

  void AddReference();
  int RemoveReference();
  void Lock();
  void Unlock();
  IObj* GetObject();
  void Unload();
  void CancelLoad();
  bool IsLoading() const;
  const SObjectTag& GetObjectTag() const { return mObjTag; }
  int GetReferenceCount() const { return mRefCount; } // Guessed name

private:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_KYOTO_OBJECT_REFERENCE_PACKED_COUNTS
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
#if !RSTL_KYOTO_OBJECT_REFERENCE_ADD_OUT_OF_LINE
inline void CObjectReference::AddReference() { mRefCount++; }
#endif
#if !RSTL_PRIME1 && !RSTL_ECHOES_DEMO
CHECK_SIZEOF(CObjectReference, RSTL_KYOTO_ASSET_ID_CLASS ? 0x30 : 0x24)
#endif

#endif // _COBJECTREFERENCE
