#include "rstl/RstlVersions.h"
#include "Kyoto/CToken.hpp"
#include "Kyoto/Alloc/CMemory.hpp"

CToken::CToken(CObjectReference* ref) : mObjRef(ref), mLockHeld(false) {
  mObjRef->AddReference();
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CToken::CToken(IObj* obj)
: mObjRef(rs_new CObjectReference(obj)), mLockHeld(false) {
#else
CToken::CToken(IObj* obj) : mObjRef(rs_new CObjectReference(obj)), mLockHeld(false) {
#endif

  mObjRef->AddReference();
  Lock();
}

CToken::CToken(const CToken& other) : mObjRef(other.mObjRef), mLockHeld(false) {
  mObjRef->AddReference();
  if (other.mLockHeld) {
    Lock();
  }
}

CToken::~CToken() {
  if (mLockHeld) {
    mObjRef->Unlock();
  }
  RemoveRef();
}

CObjOwnerDerivedFromIObjUntyped* CToken::GetObj() {
  Lock();

  return reinterpret_cast< CObjOwnerDerivedFromIObjUntyped* >(mObjRef->GetObject());
}

void CToken::RemoveRef() {
  if (mObjRef->RemoveReference() == 0) {
    delete mObjRef;
  }
}

void CToken::Lock() {
  if (!mLockHeld) {
    mObjRef->Lock();
    mLockHeld = true;
  }
}

void CToken::Unlock() {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (mLockHeld == 0) {
#else
  if (!mLockHeld) {
#endif

    return;
  }

  mObjRef->Unlock();
  mLockHeld = false;
}

CToken& CToken::operator=(const CToken& other) {
  if (&other == this) {
    return *this;
  }

  Unlock();
  RemoveRef();
  mObjRef = other.mObjRef;
  mObjRef->AddReference();
  if (other.mLockHeld) {
    Lock();
  }

  return *this;
}

