#ifndef _CADVANCEMENTDELTAS_HPP
#define _CADVANCEMENTDELTAS_HPP
#include "rstl/RstlVersions.h"

#include "Kyoto/Math/CQuaternion.hpp"

struct CAdvancementDeltas {
public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CAdvancementDeltas() : mPosDelta(CVector3f::Zero()), mRotDelta(CQuaternion::NoRotation()) {}
#else
  CAdvancementDeltas() {}
#endif

  CAdvancementDeltas(const CVector3f& posDelta, const CQuaternion& rotDelta)
  : mPosDelta(posDelta), mRotDelta(rotDelta) {}

  const CVector3f& GetOffsetDelta() const { return mPosDelta; }
  const CQuaternion& GetOrientationDelta() const { return mRotDelta; }

  static CAdvancementDeltas Interpolate(const CAdvancementDeltas& a, const CAdvancementDeltas& b,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                                        const float c, const float d);
#else
                                        const float startWeight, const float endWeight);
#endif

  static CAdvancementDeltas Blend(const CAdvancementDeltas& a, const CAdvancementDeltas& b,
                                  const float t);

  CVector3f mPosDelta;
  CQuaternion mRotDelta;
};
CHECK_SIZEOF(CAdvancementDeltas, 0x1c)

#endif // _CADVANCEMENTDELTAS_HPP
