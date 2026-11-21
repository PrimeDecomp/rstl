#ifndef _CCECHARACTERINFO
#define _CCECHARACTERINFO
#include "rstl/RstlVersions.h"

#include "rstl/static_assert.hpp"
#include "rstl/types.h"

#if RSTL_CORRUPTION
#include "Kyoto/Animation/CAnimation.hpp"
#include "Kyoto/Animation/CNamedAnimPOIData.hpp"
#else
#include "Kyoto/Particles/CEffectComponent.hpp"

#endif
#include "Kyoto/Animation/CPASDatabase.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/Animation/CParticleResData.hpp"
#include "Kyoto/CAssetId.hpp"
#include "Kyoto/CAssetIdList.hpp"
#else
#include "Kyoto/IObjectStore.hpp"
#endif
#include "Kyoto/Math/CAABox.hpp"
#if RSTL_CORRUPTION

#include "Corruption/MetroidPrime/CJointCollisionDescription.hpp"
#endif

#include "rstl/pair.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

#if !RSTL_CORRUPTION
class CCECharacterInfo {
public:
  typedef rstl::vector< CEffectComponent > CEffectComponents;
  class CParticleResData {
  public:
    CParticleResData(CInputStream& in, ushort tableCount);
    CParticleResData(const rstl::vector< CAssetId >& part, const rstl::vector< CAssetId >& swhc,
                     const rstl::vector< CAssetId >& elscA, const rstl::vector< CAssetId >& spsc,
                     const rstl::vector< CAssetId >& srsc, const rstl::vector< CAssetId >& elscB)
    : mPart(part), mSwhc(swhc), mElscA(elscA), mSpsc(spsc), mSrsc(srsc), mElscB(elscB) {}
    const rstl::vector< CAssetId >& GetParts() const { return mPart; }
    const rstl::vector< CAssetId >& GetSwooshes() const { return mSwhc; }
    const rstl::vector< CAssetId >& GetElectrics() const { return mElscA; }
    const rstl::vector< CAssetId >& GetSpawnSystems() const { return mSpsc; }
    const rstl::vector< CAssetId >& GetSortedSystems() const { return mSrsc; }
#endif

#if RSTL_CORRUPTION

class CInputStream;

// Guessed name
class ICECharacterInfo {
public:
  virtual ~ICECharacterInfo() {}
  virtual const rstl::vector< CNamedAnimPOIData >& GetNamedPOIEntries() const = 0;
  virtual int MapAnimationIndex(int index) const = 0; // Guessed name
  virtual void AppendAnimationIndices(rstl::vector< uint >& out) const = 0;
  virtual const rstl::string& GetName() const = 0;
  virtual const CPASDatabase& GetPASDatabase() const = 0;
  virtual const CParticleResData& GetParticleResData() const = 0;
  virtual const CAssetIdList& GetAssetIdList() const = 0;
  virtual CAABox GetBoundsForAssetKey(const CAssetId& id) const = 0;
  virtual CAABox GetCombinedAssetBounds() const = 0;
  virtual rstl::string GetAnimationNameByIndex(uint index) const = 0;
  virtual CAssetId GetModelId() const = 0;
  virtual CAssetId GetSkinRulesId() const = 0;
  virtual bool GetFlagByIndex(uint index) const = 0;
  virtual const rstl::vector< CJointCollisionDescription >*
  FindCollisionDescriptions(const rstl::string& name) const = 0;
#else
  private:
    rstl::vector< CAssetId > mPart;
    rstl::vector< CAssetId > mSwhc;
    rstl::vector< CAssetId > mElscA;
    rstl::vector< CAssetId > mSpsc;
    rstl::vector< CAssetId > mSrsc;
    rstl::vector< CAssetId > mElscB;
#endif
  };

#if RSTL_CORRUPTION
class CCECharacterInfo : public ICECharacterInfo {
public:
  typedef ::CParticleResData CParticleResData;
#else
  explicit CCECharacterInfo(CInputStream& in);
  CCECharacterInfo(const CCECharacterInfo& other);
#endif

#if !RSTL_CORRUPTION
  CAssetId GetModelId() const { return mCmdl; }
  CAssetId GetSkinRulesId() const { return mCksr; }
  CAssetId GetCharLayoutInfoId() const { return mCinf; }
  CAssetId GetIceModelId() const { return mCmdlOverlay; }
  CAssetId GetIceSkinRulesId() const { return mCksrOverlay; }
  CAssetId GetSpatialPrimitiveId() const { return mSpatialPrimitiveId; }
  bool GetAnimatedScale() const { return mAnimatedScale; }
  uint GetDefaultAnimation() const { return mDefaultAnimation; }
  const CPASDatabase& GetPASDatabase() const { return mPasDatabase; }
  const CParticleResData& GetParticleResData() const;
  const rstl::vector< uint >& GetAnimationIndexList() const;
#endif
  // Guessed name
#if RSTL_CORRUPTION
  typedef rstl::pair< CAssetId, CAABox > TAssetBounds;

  typedef rstl::pair< rstl::string, rstl::vector< CJointCollisionDescription > >
      TCollisionDescList;

  explicit CCECharacterInfo(CInputStream& in);

  // ICECharacterInfo
  ~CCECharacterInfo() override;

  const rstl::vector< CNamedAnimPOIData >& GetNamedPOIEntries() const override {
    return mNamedPOIEntries;
#else
  const rstl::vector< rstl::pair< int, rstl::string > >& GetAnimationInfo() const {
    return mAnimInfo;
#endif
  }
#if RSTL_CORRUPTION
  int MapAnimationIndex(int index) const override { return index; }
  void AppendAnimationIndices(rstl::vector< uint >& out) const override;
  const rstl::string& GetName() const override { return mName; }
  const CPASDatabase& GetPASDatabase() const override { return mPasDatabase; }
  const CParticleResData& GetParticleResData() const override { return mParticleRes; }
  const CAssetIdList& GetAssetIdList() const override { return mAssetIdList; }
  CAABox GetBoundsForAssetKey(const CAssetId& id) const override;
  CAABox GetCombinedAssetBounds() const override;
  rstl::string GetAnimationNameByIndex(uint index) const override;
  CAssetId GetModelId() const override { return mModelId; }
  CAssetId GetSkinRulesId() const override { return mSkinRulesId; }
  bool GetFlagByIndex(uint index) const override;
  const rstl::vector< CJointCollisionDescription >*
  FindCollisionDescriptions(const rstl::string& name) const override;

  CAssetId GetCharLayoutInfoId() const { return mCharLayoutInfoId; }
  CAssetId GetAnimSetId() const { return mAnimSetId; } // Guessed name
  const rstl::vector< CAnimation >& GetAnimations() const { return mAnimations; }
  int GetCharacterIndex() const { return mCharacterIndex; } // Guessed name
  bool GetAnimatedScale() const { return mFlag; }           // Guessed name
#else
  const rstl::vector< rstl::pair< rstl::string, CAABox > >& GetAnimBBoxList() const {
    return mAabbs;
  }
  const rstl::vector< rstl::pair< uint, CAABox > >& GetAnimBoundsById() const {
    return mAnimBoundsById;
  }
  typedef rstl::vector< rstl::pair< rstl::string, CEffectComponents > > TEffectList;
  const TEffectList& GetEffects() const { return mEffects; }
#endif

private:
#if RSTL_CORRUPTION
  uchar mVersion;
  uchar mCharacterIndex; // Guessed name
#else
  ushort mTableCount;
#endif
  rstl::string mName;
#if RSTL_CORRUPTION
  CAssetId mModelId;
  CAssetId mSkinRulesId;
  CAssetId mCharLayoutInfoId;
  CAssetId mAnimSetId; // Guessed name
#else
  CAssetId mCmdl;
  CAssetId mCksr;
  CAssetId mCinf;
  rstl::vector< rstl::pair< int, rstl::string > > mAnimInfo;
#endif
  CPASDatabase mPasDatabase;
#if RSTL_CORRUPTION
  CParticleResData mParticleRes;
  rstl::vector< CNamedAnimPOIData > mNamedPOIEntries;
  rstl::vector< CAnimation > mAnimations;
  rstl::vector< TAssetBounds > mAssetBounds; // Sorted by asset id.
  bool mFlag;                                // Guessed name
  rstl::vector< bool > mFlags;
  rstl::vector< TCollisionDescList > mCollisionDescs; // Sorted by name.
  CAssetIdList mAssetIdList;
#else
  CParticleResData mPartRes;
  uint mDefaultAnimation;
  rstl::vector< rstl::pair< rstl::string, CAABox > > mAabbs;
  TEffectList mEffects;
  CAssetId mCmdlOverlay;
  CAssetId mCksrOverlay;
  rstl::vector< uint > mAnimIdxs;
  CAssetId mSpatialPrimitiveId; // Guessed name: CSPP resource.
  bool mAnimatedScale;          // Guessed name.
  rstl::vector< rstl::pair< uint, CAABox > > mAnimBoundsById;
#endif
};
#if RSTL_CORRUPTION
CHECK_SIZEOF(CCECharacterInfo, 0x128)
#else
CHECK_SIZEOF(CCECharacterInfo, 0xf8)
NESTED_CHECK_SIZEOF(CCECharacterInfo, CParticleResData, 0x60)
#endif

namespace rstl {
#if RSTL_CORRUPTION
RSTL_DECLARE_BITWISE_CONSTRUCTION(CCECharacterInfo::TAssetBounds)
#else
template <> struct vector_copy_construction_policy< pair< uint, CAABox > > {
  enum { value = true };
  static void copy_n(pair< uint, CAABox >* src, int n, pair< uint, CAABox >* dest) {
    pair< uint, CAABox >* cur = dest;
    for (int remaining = n; remaining != 0; --remaining, ++src, ++cur) {
      *cur = *src;
    }
  }
};
#endif
} // namespace rstl

#endif // _CCECHARACTERINFO
