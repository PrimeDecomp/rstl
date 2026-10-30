#include "Kyoto/CDependencyGroup.hpp"

#include "rstl/RstlVersions.h"

#if RSTL_PRIME1
#include "Kyoto/CResFactory.hpp"
#define RSTL_CONST_FACTORY_FN_RETURN const
#elif RSTL_KYOTO_CORRUPTION
#include "Corruption/Kyoto/CFactoryMgr.hpp"
#include "Corruption/Kyoto/IObjectStore.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/CDependencyGroupToken.hpp"
#define RSTL_CONST_FACTORY_FN_RETURN
#else
#include "Kyoto/CDependencyGroupToken.hpp"
#include "Kyoto/CFactoryMgr.hpp"
#include "Kyoto/IObjectStore.hpp"
#define RSTL_CONST_FACTORY_FN_RETURN
#endif
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"

CDependencyGroup::CDependencyGroup(CInputStream& in) { ReadFromStream(in); }

void CDependencyGroup::ReadFromStream(CInputStream& in) {
  int numTags = in.ReadInt32();
  mObjectTags.reserve(numTags);

  for (int i = 0; i < numTags; ++i) {
#if RSTL_KYOTO_ECHOES_DEPENDENCY_GROUP
    FourCC type = in.Get< FourCC >();
#else
    FourCC type = in.ReadInt32();
#endif
#if RSTL_KYOTO_ASSET_ID_CLASS
    CAssetId id(in);
    mObjectTags.push_back_unsafe(SObjectTag(type, id));
#elif RSTL_KYOTO_DEPENDENCY_PUSH_UNSAFE
    CAssetId id = in.ReadInt32();
    mObjectTags.push_back_unsafe(SObjectTag(type, id));
#else
    CAssetId id = in.ReadInt32();
    mObjectTags.push_back(SObjectTag(type, id));
#endif
  }
}

int CDependencyGroup::GetCountForResType(FourCC type) const {
  int ret = 0;
  for (rstl::vector< SObjectTag >::const_iterator it = mObjectTags.begin(); it != mObjectTags.end();
       ++it) {
    if (it->type == type) {
      ++ret;
    }
  }

  return ret;
}

KYOTO_START_GAME_NAMESPACE

#if RSTL_KYOTO_BUFFER_FACTORIES
CFactoryFnReturn FDependencyGroupFactory(const SObjectTag& tag,
                                         const rstl::auto_ptr< uchar >& buffer, int size,
                                         const CVParamTransfer& xfer) {
  CMemoryInStream in(buffer.get(), size);
  return rs_new_line(87) CDependencyGroup(in);
}
#else
RSTL_CONST_FACTORY_FN_RETURN CFactoryFnReturn FDependencyGroupFactory(const SObjectTag& tag, CInputStream& in,
                                         const CVParamTransfer& xfer) {
  return rs_new CDependencyGroup(in);
}
#endif

#if RSTL_KYOTO_DEPENDENCY_GROUP_TOKEN
CDependencyGroupToken::CDependencyGroupToken(const TToken< CDependencyGroup >& group,
                                             IObjectStore& store)
: mGroup(group), mLockCount(0), mLoaded(false) {
  const rstl::vector< SObjectTag >& tags = mGroup->GetObjectTagVector();
  mDependencies.reserve(tags.size());
  for (int i = 0; i < tags.size(); ++i) {
    mDependencies.push_back_unsafe(store.GetObj(tags[i]));
  }
}

void CDependencyGroupToken::Lock() {
  ++mLockCount;
  if (mLockCount == 1) {
    for (int i = 0; i < mDependencies.size(); ++i) {
      mDependencies[i].Lock();
    }
    mLoaded = false;
  }
}

void CDependencyGroupToken::Unlock() {
  --mLockCount;
  if (mLockCount == 0) {
    for (int i = 0; i < mDependencies.size(); ++i) {
      mDependencies[i].Unlock();
    }
    mLoaded = false;
  }
}

bool CDependencyGroupToken::IsLocked() const { return mLockCount != 0; }

bool CDependencyGroupToken::IsLoaded() const {
  if (mLockCount == 0) {
    return false;
  }
  if (mLoaded) {
    return true;
  }
  bool loaded = true;
  for (int i = 0; i < mDependencies.size(); ++i) {
    if (!mDependencies[i].IsLoaded()) {
      loaded = false;
    }
  }
  mLoaded = loaded;
  return loaded;
}
#endif

KYOTO_END_GAME_NAMESPACE
