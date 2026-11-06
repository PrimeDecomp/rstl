#include "rstl/RstlVersions.h"
#include "rstl/macros.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Alloc/Assert.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/IFactory.hpp"

CSimplePool::CSimplePool(IFactory& factory)
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
: mFactory(factory)
#else
: mFactory(&factory)
#endif
, mParamXfr(CVParamTransfer::Null()) {
  mParamXfr = CVParamTransfer(rs_new_line(32) TObjOwnerParam< IObjectStore* >(this));
}

CSimplePool::~CSimplePool() {
  Flush();
  if (mResources.size() > 0) {
    RS_DEBUGGER_PRINT("Error: Object store %x still contains referenced objects:\n");
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
    CObjectReference* ref = it->second;
    RS_VERIFY_THROW(
        83, tag.GetType() == ref->GetObjectTag().GetType(), false,
        CBasics::Stringize("Object tag types don't match in CSimplePool::GetObj. Type:%s\tID:%s",
                           SObjectTag::Type2Text(tag.GetType()), tag.GetId().ToHexString().data()));
    return CToken(ref);
  }

  RS_VERIFY_THROW(91, mFactory->CanBuild(tag), false,
                  CBasics::Stringize("Asset doesn't exist in .pak files. Type:%s\tID:%s",
                                     SObjectTag::Type2Text(tag.GetType()),
                                     tag.GetId().ToHexString().data()));
  RS_VERIFY_THROW(95, mFactory != NULL, false,
                  CBasics::Stringize("Missing factory for type:%s\tID:%s",
                                     SObjectTag::Type2Text(tag.GetType()),
                                     tag.GetId().ToHexString().data()));
  CObjectReference* ref =
      rs_new_line(97) CObjectReference(*this, rstl::auto_ptr< IObj >(nullptr), tag, xfer);
  RSTL_PRIME1_CONST ResourceMap::value_type item(tag, ref);

  mResources.insert(item);
  return CToken(ref);
}

CToken CSimplePool::GetObj(const SObjectTag& tag) { return CSimplePool::GetObj(tag, mParamXfr); }

CToken CSimplePool::GetObj(const char* name) { return CSimplePool::GetObj(name, mParamXfr); }

CToken CSimplePool::GetObj(const char* name, const CVParamTransfer& xfer) {
  const SObjectTag* tag = CSimplePool::GetFactory().GetResourceIdByName(name);
  RS_VERIFY_THROW(121, tag != NULL, false,
                  CBasics::Stringize("Can't find resource id for: %s\n", name));
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
  RS_DEBUGGER_PRINT("Object Store contents:\n");
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  AUTO(it, mResources.begin());
#else
  ResourceMap::const_iterator it = mResources.begin();
#endif
  for (; it != mResources.end(); ++it) {
#if RSTL_PRIME1
    SObjectTag::Type2Text(it->first.GetType());
#else
    RS_DEBUGGER_PRINT("   Asset %s type %s\n", it->first.GetId().ToHexString().data(),
                      SObjectTag::Type2Text(it->first.GetType()));
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

