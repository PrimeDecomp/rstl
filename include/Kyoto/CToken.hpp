#ifndef _CTOKEN
#define _CTOKEN
#include "rstl/RstlVersions.h"

#include "types.h"

#include "Kyoto/CObjectReference.hpp"
#include "Kyoto/IObj.hpp"

class CObjectReference;

class CToken {
public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CToken() : mObjRef(NULL), mLockHeld_init(0) {}
#else
  CToken() {}
#endif

  CToken(CObjectReference* ref);
  CToken(IObj* obj); // : x0_objRef(new CObjectReference(obj)), x4_lockHeld(false) {}
  CToken(const CToken& other);
  ~CToken();

  CObjOwnerDerivedFromIObjUntyped* GetObj();
  void Lock();
  void Unlock();
#if RSTL_PRIME1
  bool IsLocked() const { return mLockHeld; }
#endif

  bool IsLoaded() const { return mObjRef->IsLoaded(); }
  void RemoveRef();
  CToken& operator=(const CToken&);
  bool HasReference() const { return mObjRef != nullptr; }
  const SObjectTag& GetTag() const { return mObjRef->GetObjectTag(); }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  FourCC GetReferenceType() const { return GetTag().GetType(); }
#else
  FourCC GetReferenceType() { return GetTag().type; }

  bool IsLocked() { return mLockHeld; }
#endif

  const CObjectReference* GetRef() const { return mObjRef; }

private:
  CObjectReference* mObjRef;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  union {
    bool mLockHeld;
    int mLockHeld_init;
  };
#else
  bool mLockHeld;
#endif
};

#endif // _CTOKEN
