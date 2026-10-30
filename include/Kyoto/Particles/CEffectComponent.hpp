#ifndef _CEFFECTCOMPONENT
#define _CEFFECTCOMPONENT
#include "rstl/RstlVersions.h"

#include "Kyoto/Particles/CParticleData.hpp"
#include "rstl/string.hpp"

class CEffectComponent {
public:
  explicit CEffectComponent(CInputStream& in);

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const rstl::string& GetComponentName() const { return mName; }
#else
  uint GetComponentNameHash() const { return mNameHash; }
#endif

  const SObjectTag& GetParticleTag() const { return mTag; }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const rstl::string& GetSegmentName() const { return mBoneName; }
#else
  const CSegId& GetSegmentId() const { return mBone; }
#endif

  float GetScale() const { return mScale; }
  CParticleData::EParentedMode GetParentedMode() const { return mParentedMode; }
  uint GetFlags() const { return mFlags; }

private:
  SObjectTag GetSObjectTagFromStream(CInputStream& in);

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  rstl::string mName;
#else
  // Echoes keeps a CPOINode string hash and segment ID instead of Prime's names.
  uint mNameHash;
#endif

  SObjectTag mTag;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  rstl::string mBoneName;
#else
  CSegId mBone;
#endif

  float mScale;
  CParticleData::EParentedMode mParentedMode;
  uint mFlags;
};
#if !RSTL_PRIME1
CHECK_SIZEOF(CEffectComponent, 0x1c)
#endif

#endif // _CEFFECTCOMPONENT
