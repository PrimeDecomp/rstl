#ifndef _CTOKEN
#define _CTOKEN
#include "rstl/RstlVersions.h"

#include "types.h"

#include "Kyoto/CObjectReference.hpp"
#include "Kyoto/IObj.hpp"

class CObjectReference;

class CToken {
public:
  CToken()
#if RSTL_PRIME1
  : mObjRef(NULL)
  , mLockHeld_init(0)
#endif
  {
  }

  CToken(CObjectReference* ref);
  CToken(IObj* obj); // : x0_objRef(new CObjectReference(obj)), x4_lockHeld(false) {}
  CToken(const CToken& other);
  ~CToken();

  CObjOwnerDerivedFromIObjUntyped* GetObj();
  void Lock();
  void Unlock();
  bool IsLocked() RSTL_PRIME1_CONST { return mLockHeld; }

  bool IsLoaded() const { return mObjRef->IsLoaded(); }
  void RemoveRef();
  CToken& operator=(const CToken&);
  bool HasReference() const { return mObjRef != nullptr; }
  const SObjectTag& GetTag() const { return mObjRef->GetObjectTag(); }
  FourCC GetReferenceType() RSTL_PRIME1_CONST { return GetTag().GetType(); }

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
