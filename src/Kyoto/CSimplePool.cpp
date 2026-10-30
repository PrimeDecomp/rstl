#include "rstl/RstlVersions.h"
#include "Kyoto/CSimplePool.hpp"
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#include "Kyoto/IFactory.hpp"
#else
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#endif

CSimplePool::CSimplePool(IFactory& factory)
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
: mFactory(factory), mParamXfr(CVParamTransfer::Null()) {
#else
: mFactory(&factory), mParamXfr(CVParamTransfer::Null()) {
#endif

  mParamXfr = CVParamTransfer(rs_new TObjOwnerParam< IObjectStore* >(this));
}

CSimplePool::~CSimplePool() {
  Flush();
  if (mResources.size() > 0) {
    DebugDumpPool();
  }
}

void CSimplePool::ObjectUnreferenced(const SObjectTag& tag) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  AUTO(it, mResources.find(tag));
#else
  ResourceMap::iterator it = mResources.find(tag);
#endif

  mResources.erase(it);
}

CToken CSimplePool::GetObj(const SObjectTag& tag, const CVParamTransfer& xfer) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  AUTO(it, mResources.find(tag));
#else
  ResourceMap::iterator it = mResources.find(tag);
#endif

  if (it != mResources.end()) {
    return CToken(it->second);
  }

  CObjectReference* ref =
      rs_new CObjectReference(*this, rstl::auto_ptr< IObj >(nullptr), tag, xfer);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const ResourceMap::value_type item(tag, ref);
#else
  ResourceMap::value_type item(tag, ref);
#endif

  mResources.insert(item);
  return CToken(ref);
}

CToken CSimplePool::GetObj(const SObjectTag& tag) { return CSimplePool::GetObj(tag, mParamXfr); }

CToken CSimplePool::GetObj(const char* name) { return CSimplePool::GetObj(name, mParamXfr); }

CToken CSimplePool::GetObj(const char* name, const CVParamTransfer& xfer) {
  const SObjectTag* tag = CSimplePool::GetFactory().GetResourceIdByName(name);
  return CSimplePool::GetObj(*tag, xfer);
}

bool CSimplePool::HasObject(const SObjectTag& tag) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  AUTO(it, mResources.find(tag));
  bool result = true;
  if (!(it != mResources.end())) {
#if NONMATCHING
    const bool canBuild = mFactory.CanBuild(tag);
#else
    const bool canBuild = &mFactory != nullptr && mFactory.CanBuild(tag);
#endif
    if (!canBuild) {
      result = false;
    }
  }
  return result;
#else
  ResourceMap::const_iterator it = mResources.find(tag);
  return it != mResources.end() || (mFactory != nullptr && mFactory->CanBuild(tag));
#endif

}

bool CSimplePool::ObjectIsLive(const SObjectTag& tag) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  AUTO(it, mResources.find(tag));
#else
  ResourceMap::const_iterator it = mResources.find(tag);
#endif

  if (it == mResources.end()) {
    return false;
  }
  return it->second->IsLoaded();
}

void CSimplePool::Flush() {}

void CSimplePool::DebugDumpPool() const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  AUTO(it, mResources.begin());
#else
  ResourceMap::const_iterator it = mResources.begin();
#endif

  for (; it != mResources.end(); ++it) {
#if RSTL_PRIME1
    SObjectTag::Type2Text(it->first.GetType());
#endif

  }
}

rstl::vector< SObjectTag > CSimplePool::GetReferencedTags() const {
  rstl::vector< SObjectTag > tags;
  tags.reserve(mResources.size());
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  AUTO(it, mResources.begin());
  AUTO(end, mResources.end());
#else
  ResourceMap::const_iterator it = mResources.begin();
  ResourceMap::const_iterator end = mResources.end();
#endif

  for (; it != end; ++it) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    tags.push_back(it->first);
#else
    tags.push_back_unsafe(it->first);
#endif

  }
  return tags;
}

