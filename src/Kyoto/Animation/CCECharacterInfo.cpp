#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CCECharacterInfo.hpp"

#if RSTL_CORRUPTION
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/RAssertDolphin.hpp"
#include "Kyoto/CFactoryMgr.hpp"
#include "Kyoto/CVParamTransfer.hpp"
#endif
#include "Kyoto/Streams/CInputStream.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/Streams/CMemoryInStream.hpp"
#endif

#if RSTL_CORRUPTION
#include "rstl/algorithm.hpp"
#include "rstl/set.hpp"
#else
CCECharacterInfo::CParticleResData::CParticleResData(CInputStream& in, ushort tableCount)
: mPart(in), mSwhc(in), mElscB(in) {
  if (tableCount > 5) {
    const rstl::vector< CAssetId > resources(in);
    mElscA = rstl::vector< CAssetId >(resources.begin(), resources.end());
  }
#endif

#if RSTL_CORRUPTION


CCECharacterInfo::CCECharacterInfo(CInputStream& in)
: mVersion(in.Get< uchar >())
, mCharacterIndex(in.Get< uchar >())
, mName(in)
, mModelId(in.Get< CAssetId >())
, mSkinRulesId(in.Get< CAssetId >())
, mCharLayoutInfoId(in.Get< CAssetId >())
, mAnimSetId(in.Get< CAssetId >())
, mPasDatabase(in.Get(TGetType(mPasDatabase)))
, mParticleRes(in, mVersion)
, mNamedPOIEntries(in)
, mAnimations(in)
, mAssetBounds(in)
, mFlag(in.Get< bool >())
, mAssetIdList(rstl::set< CAssetId >()) {
  if (mVersion != 0) {
    mFlags = rstl::vector< bool >(in);
  }
  if (mVersion > 1) {
    mCollisionDescs = rstl::vector< TCollisionDescList >(in);
  }
  if (mVersion > 2) {
    mAssetIdList = CAssetIdList(in);
#else
  if (tableCount > 8) {
    const rstl::vector< CAssetId > spawnResources(in);
    mSpsc = rstl::vector< CAssetId >(spawnResources.begin(), spawnResources.end());
    const rstl::vector< CAssetId > sortedResources(in);
    mSrsc = rstl::vector< CAssetId >(sortedResources.begin(), sortedResources.end());
#endif
  }
}

#if RSTL_CORRUPTION
void CCECharacterInfo::AppendAnimationIndices(rstl::vector< uint >& out) const {
  const uint count = mAnimations.size();
  out.reserve(out.size() + count);
  for (uint i = 0; i < count; ++i) {
    out.push_back(i);
#else
CCECharacterInfo::CCECharacterInfo(CInputStream& in)
: mTableCount(in.Get< ushort >())
, mName(in)
, mCmdl(in.Get< CAssetId >())
, mCksr(in.Get< CAssetId >())
, mCinf(in.Get< CAssetId >())
, mAnimInfo(in)
, mPasDatabase(in.Get(TGetType(mPasDatabase)))
, mPartRes(in, mTableCount)
, mDefaultAnimation(in.Get< uint >())
, mCmdlOverlay(kInvalidAssetId)
, mCksrOverlay(kInvalidAssetId)
, mSpatialPrimitiveId(kInvalidAssetId)
, mAnimatedScale(false) {
  if (mTableCount > 1) {
    mAabbs = rstl::vector< rstl::pair< rstl::string, CAABox > >(in);
  }
  if (mTableCount > 2) {
    mEffects = TEffectList(in);
  }
  if (mTableCount > 3) {
    mCmdlOverlay = in.Get< CAssetId >();
    mCksrOverlay = in.Get< CAssetId >();
  } else {
    mCmdlOverlay = 0;
    mCksrOverlay = 0;
  }
  if (mTableCount > 4) {
    mAnimIdxs = rstl::vector< uint >(in);
  }
  if (mTableCount > 6) {
    mSpatialPrimitiveId = in.Get< CAssetId >();
  }
  if (mTableCount > 7) {
    mAnimatedScale = in.Get< bool >();
  }
  if (mTableCount > 9) {
    mAnimBoundsById = rstl::vector< rstl::pair< uint, CAABox > >(in);
#endif
  }
}

#if RSTL_CORRUPTION
CAABox CCECharacterInfo::GetBoundsForAssetKey(const CAssetId& assetId) const {
  const CAssetId id = assetId;
  rstl::vector< TAssetBounds >::const_iterator it = rstl::binary_find(
      mAssetBounds.begin(), mAssetBounds.end(), id,
      rstl::pair_sorter_finder< TAssetBounds, rstl::less< CAssetId > >(rstl::less< CAssetId >()));
  if (it != mAssetBounds.end()) {
    return it->second;
#else
// Guessed; stands in for a function stripped from the link.
static void CopyAnimBBoxes(const rstl::vector< rstl::pair< rstl::string, CAABox > >& bboxes) {
  rstl::vector< rstl::pair< rstl::string, CAABox > > copy(bboxes);
#endif
}

#if RSTL_CORRUPTION
  rs_debugger_printf("WARNING: Bounding box not found for 0x%x\n", id);
  return CAABox::MakeNullBox();
#else
// Guessed; stands in for a function stripped from the link.
static void CopyCharacterEffects(const CCECharacterInfo::TEffectList& effects) {
  CCECharacterInfo::TEffectList copy(effects);
#endif
}
#if RSTL_CORRUPTION

const rstl::vector< CJointCollisionDescription >*
CCECharacterInfo::FindCollisionDescriptions(const rstl::string& name) const {
  rstl::vector< TCollisionDescList >::const_iterator it =
      rstl::binary_find(
          mCollisionDescs.begin(), mCollisionDescs.end(), name,
          rstl::pair_sorter_finder< TCollisionDescList, rstl::less< rstl::string > >(
              rstl::less< rstl::string >()));
  if (it != mCollisionDescs.end()) {
    return &it->second;
  }
  return nullptr;
}

CAABox CCECharacterInfo::GetCombinedAssetBounds() const {
  CAABox bounds = CAABox::MakeMaxInvertedBox();
  const uint count = mAssetBounds.size();
  for (uint i = 0; i < count; ++i) {
    bounds.Include(mAssetBounds[i].second);
  }
  return bounds;
}

rstl::string CCECharacterInfo::GetAnimationNameByIndex(uint index) const {
  if (index < mAnimations.size()) {
    return mAnimations[index].GetName();
  }
  return rstl::string("");
}

bool CCECharacterInfo::GetFlagByIndex(uint index) const {
  if (index < mFlags.size()) {
    return mFlags[index];
  }
  return false;
}

CFactoryFnReturn FCECharacterInfoFactory(const SObjectTag& tag,
                                         const rstl::auto_ptr< uchar >& data, int length,
                                         const CVParamTransfer& xfer) {
  CMemoryInStream in(data.get(), length);
  return rs_new_line(211) CCECharacterInfo(in);
}

CCECharacterInfo::~CCECharacterInfo() {}
#endif
