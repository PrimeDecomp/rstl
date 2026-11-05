#ifndef _CPARTICLEDATA
#define _CPARTICLEDATA
#include "rstl/RstlVersions.h"

#include "types.h"

#include "Kyoto/Animation/CSegId.hpp"

#include "Kyoto/IObjectStore.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "rstl/string.hpp"
#include "rstl/construct.hpp"

class CInputStream;

class CParticleData {
public:
  enum EParentedMode {
    kPM_Invalid = -1, // Guessed name
    kPM_Initial,
    kPM_ContinuousEmitter,
    kPM_ContinuousSystem,
  };

#if RSTL_KYOTO_PARTICLE_DATA_PROTOTYPE
  CParticleData(const SObjectTag& tag = SObjectTag(0, CAssetId(0ULL)), float scale = 1.f,
                EParentedMode mode = kPM_Initial)
  : mParticle(tag), mScale(scale), mParentMode(mode) {}
#else
  CParticleData(int duration = 0, const SObjectTag& tag = SObjectTag(0, 0),
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                const rstl::string& boneName = rstl::string_l("root"),
#else
                const CSegId& bone = CSegId(0),
#endif
                float scale = 1.f, EParentedMode mode = kPM_Initial)
  : mDuration(duration)
  , mParticle(tag)
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  , mBoneName(boneName)
#else
  , mBone(bone)
#endif
  , mScale(scale)
  , mParentMode(mode) {}
#endif

  CParticleData(CInputStream& in);

  SObjectTag GetParticleAssetInfo() const {
    return SObjectTag(mParticle.GetType(), mParticle.GetId());
  }
#if !RSTL_KYOTO_PARTICLE_DATA_PROTOTYPE
  int GetDuration() const { return mDuration; }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const rstl::string& GetLocatorName() const { return mBoneName; }
#else
  CSegId GetSegmentId() const { return mBone; }
#endif
#endif

  float GetScale() const { return mScale; }
  EParentedMode GetParentedMode() const {
#if RSTL_PRIME1
    return static_cast< EParentedMode >(mParentMode);
#else
    return mParentMode;
#endif
  }

private:
#if !RSTL_KYOTO_PARTICLE_DATA_PROTOTYPE
  int mDuration;
#endif
  SObjectTag mParticle;
#if !RSTL_KYOTO_PARTICLE_DATA_PROTOTYPE
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  rstl::string mBoneName;
#else
  // Echoes stores a segment ID where Prime stored the bone name.
  CSegId mBone;
#endif
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

#if RSTL_KYOTO_PARTICLE_DATA_PROTOTYPE
#include "Kyoto/Streams/CInputStream.hpp"
inline CParticleData::CParticleData(CInputStream& in)
: mParticle(in)
, mScale(in.ReadFloat())
, mParentMode(static_cast< EParentedMode >(in.Get< int >())) {}
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
