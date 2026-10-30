#ifndef _CPARTICLEDATA
#define _CPARTICLEDATA
#include "rstl/RstlVersions.h"

#include "types.h"

#if !RSTL_PRIME1
#include "Kyoto/Animation/CSegId.hpp"
#endif

#include "Kyoto/IObjectStore.hpp"
#include "Kyoto/Math/CVector3f.hpp"
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#include "rstl/string.hpp"
#else
#include "rstl/construct.hpp"
#endif

class CInputStream;

class CParticleData {
public:
  enum EParentedMode {
    kPM_Initial,
    kPM_ContinuousEmitter,
    kPM_ContinuousSystem,
  };

  CParticleData(int duration = 0, const SObjectTag& tag = SObjectTag(0, 0),
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                const rstl::string& boneName = rstl::string_l("root"), float scale = 1.f,
                EParentedMode mode = kPM_Initial)
  : mDuration(duration)
  , mParticle(tag)
  , mBoneName(boneName)
  , mScale(scale)
  , mParentMode(mode) {}
#else
                const CSegId& bone = CSegId(0), float scale = 1.f, EParentedMode mode = kPM_Initial)
  : mDuration(duration), mParticle(tag), mBone(bone), mScale(scale), mParentMode(mode) {}
#endif

  CParticleData(CInputStream& in);

  int GetDuration() const { return mDuration; }
  SObjectTag GetParticleAssetInfo() const {
    return SObjectTag(mParticle.GetType(), mParticle.GetId());
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const rstl::string& GetLocatorName() const { return mBoneName; }
#else
  CSegId GetSegmentId() const { return mBone; }
#endif

  float GetScale() const { return mScale; }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  EParentedMode GetParentedMode() const { return static_cast< EParentedMode >(mParentMode); }
#else
  EParentedMode GetParentedMode() const { return mParentMode; }
#endif

private:
  int mDuration;
  SObjectTag mParticle;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  rstl::string mBoneName;
#else
  // Echoes stores a segment ID where Prime stored the bone name.
  CSegId mBone;
#endif

  float mScale;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  int mParentMode;
#else
  EParentedMode mParentMode;
#endif

};
#if !RSTL_PRIME1
CHECK_SIZEOF(CParticleData, 0x18)
#endif

class CAuxiliaryParticleData {
#if RSTL_PRIME1
public:
  CAuxiliaryParticleData(uint duration, const SObjectTag& tag, const CVector3f& translation,
                         float scale)
  : mDuration(duration), mParticle(tag), mTranslation(translation), mScale(scale) {}

  uint GetDuration() const { return mDuration; }
  SObjectTag GetParticleAssetInfo() const {
    return SObjectTag(mParticle.GetType(), mParticle.GetId());
  }
  const CVector3f& GetTranslation() const { return mTranslation; }
  float GetScale() const { return mScale; }
#endif

private:
  uint mDuration;
  SObjectTag mParticle;
  CVector3f mTranslation;
  float mScale;
};

#if !RSTL_PRIME1
namespace rstl {
template <>
struct is_scalar_type< CParticleData::EParentedMode > {
  enum { value = true };
};
} // namespace rstl
#endif

#endif // _CPARTICLEDATA
