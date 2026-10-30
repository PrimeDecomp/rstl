#ifndef _TTOKEN
#define _TTOKEN
#include "rstl/RstlVersions.h"

#include "types.h"

#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/CToken.hpp"
#include "Kyoto/IObj.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/construct.hpp"

template < typename T >
class TToken : public CToken {
public:
  TToken() {}
  TToken(const CToken& token) : CToken(token) {}
  TToken(T* obj) : CToken(TObjOwnerDerivedFromIObj< T >::GetNewDerivedObject(obj).release()) {}
  TToken(const rstl::auto_ptr< T >& obj) : CToken(GetIObjObjectFor(obj).release()) {}

  // TToken& operator=(const TToken& other) {

  // }
  T* GetT() { return reinterpret_cast< T* >(CToken::GetObj()->GetContents()); }
  T* operator*() { return GetT(); }
  T* operator->() { return GetT(); }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_R3IJ
  typedef rstl::auto_ptr< IObj > TOwnerPtr;
#else
  typedef rstl::auto_ptr< TObjOwnerDerivedFromIObj< T > > TOwnerPtr;
#endif

  static TOwnerPtr GetIObjObjectFor(const rstl::auto_ptr< T >& obj) {
#else
  static typename TObjOwnerDerivedFromIObj< T >::TOwnerPtr
  GetIObjObjectFor(const rstl::auto_ptr< T >& obj) {
#endif

    return TObjOwnerDerivedFromIObj< T >::GetNewDerivedObject(obj);
  }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  TToken< T > NonConstCopy() const { return *this; }
#else
  TToken< T > NonConstCopy() const { return *const_cast< TToken< T >* >(this); }
#endif

};

template < typename T >
class TCachedToken : public TToken< T > {
public:
  TCachedToken() {}
  TCachedToken(const CToken& token) : TToken< T >(token), mItem(nullptr) {}
#if !RSTL_PRIME1
  TCachedToken(const CToken& token, bool) : TToken< T >(token), mItem(TToken< T >::GetT()) {}
#endif

#if !RSTL_PRIME1
  T* operator*() { return mItem; }
#endif

  T* GetObject() const { return mItem; }
#if !RSTL_PRIME1
  const CToken& GetToken() const { return *this; }
#endif

#if !RSTL_PRIME1
  bool IsLoaded() const { return mItem != nullptr || CToken::IsLoaded(); }
#endif

  bool TryCache() {
    if (mItem != nullptr) {
      return true;
    }
    if (CToken::IsLocked() && CToken::IsLoaded()) {
      mItem = TToken< T >::GetT();
      return true;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    }
#else
    } else {
#endif

    return false;
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  void Lock() { TToken< T >::Lock(); }

  const CToken& GetToken() const { return *this; }
#else
  }
#endif

  void Unlock() {
    mItem = nullptr;
    TToken< T >::Unlock();
  }
#if RSTL_PRIME1
  bool IsLoaded() const { return mItem != nullptr || CToken::IsLoaded(); }
#endif

  void ForceCache() {
    if (mItem == nullptr) {
      mItem = TToken< T >::GetT();
    }
  }

private:
  T* mItem;
};

template < typename T >
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
class TLockedToken : public TToken< T > {
#else
class TLockedToken {
#endif

public:
  TLockedToken() {}
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  TLockedToken(T* item) : TToken< T >(item), mItem(item) { CToken::Lock(); }
  TLockedToken(const CToken& token) : TToken< T >(token), mItem(TToken< T >::GetT()) {}
  TLockedToken(const TLockedToken< T >& token) : TToken< T >(token), mItem(*token) {
    CToken::Lock();
#else
  TLockedToken(T* item) : mToken(item), mItem(item) { mToken.Lock(); }
  TLockedToken(const CToken& token) : mToken(token), mItem(*mToken) {}
  TLockedToken(const TLockedToken< T >& token) : mToken(token), mItem(*token) { mToken.Lock(); }

  TLockedToken& operator=(const TLockedToken< T >& token) {
    mToken = token.mToken;
    mItem = token.mItem;
    return *this;
#endif

  }

#if !RSTL_PRIME1
  operator const TToken< T >&() const { return mToken; }
#endif

  T* operator*() const { return mItem; }
  T* operator->() const { return mItem; }
#if !RSTL_PRIME1
  bool IsLoaded() const { return mToken.IsLoaded(); }
#endif

private:
#if !RSTL_PRIME1
  TToken< T > mToken;
#endif

  T* mItem;
};

#if !RSTL_PRIME1
namespace rstl {
template < typename T >
struct range_destruction_checks< TCachedToken< T > > {
  enum { value = true };
};
} // namespace rstl
#endif

#endif // _TTOKEN
