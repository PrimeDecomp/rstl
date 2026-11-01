#ifndef _CCECHARACTERINFO_INL
#define _CCECHARACTERINFO_INL
#include "Kyoto/Animation/CCECharacterInfo.hpp"

inline CCECharacterInfo::CCECharacterInfo(const CCECharacterInfo& other)
: mTableCount(other.mTableCount),
  mName(other.mName),
  mCmdl(other.mCmdl),
  mCksr(other.mCksr),
  mCinf(other.mCinf),
  mAnimInfo(other.mAnimInfo),
  mPasDatabase(other.mPasDatabase),
  mPartRes(other.mPartRes),
  mDefaultAnimation(other.mDefaultAnimation),
  mAabbs(other.mAabbs),
  mEffects(other.mEffects),
  mCmdlOverlay(other.mCmdlOverlay),
  mCksrOverlay(other.mCksrOverlay),
  mAnimIdxs(other.mAnimIdxs),
  mSpatialPrimitiveId(other.mSpatialPrimitiveId),
  mAnimatedScale(other.mAnimatedScale),
  mAnimBoundsById(other.mAnimBoundsById) {}
inline const CCECharacterInfo::CParticleResData& CCECharacterInfo::GetParticleResData() const {
  return mPartRes;
}
inline const rstl::vector< uint >& CCECharacterInfo::GetAnimationIndexList() const {
  return mAnimIdxs;
}

#endif // _CCECHARACTERINFO_INL
