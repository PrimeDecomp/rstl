#ifndef _CADVANCEMENTDELTAS_HPP
#define _CADVANCEMENTDELTAS_HPP
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"

#include "Kyoto/Math/CQuaternion.hpp"
#include "rstl/construct.hpp"

struct CAdvancementDeltas {
public:
  CAdvancementDeltas()
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  : mPosDelta(CVector3f::Zero())
  , mRotDelta(CQuaternion::NoRotation())
#endif
  {}

  CAdvancementDeltas(const CVector3f& posDelta, const CQuaternion& rotDelta)
  : mPosDelta(posDelta), mRotDelta(rotDelta) {}

  const CVector3f& GetOffsetDelta() const { return mPosDelta; }
  const CQuaternion& GetOrientationDelta() const { return mRotDelta; }

  static CAdvancementDeltas Interpolate(const CAdvancementDeltas& a, const CAdvancementDeltas& b,
                                        const float startWeight, const float endWeight);

  static CAdvancementDeltas Blend(const CAdvancementDeltas& a, const CAdvancementDeltas& b,
                                  const float t);
  // Guessed names
  static CAdvancementDeltas InterpolateTranslationOnly(const CAdvancementDeltas& a,
                                                       const CAdvancementDeltas& b,
                                                       const float startWeight,
                                                       const float endWeight);
  static CAdvancementDeltas InterpolateRotationOnly(const CAdvancementDeltas& a,
                                                    const CAdvancementDeltas& b,
                                                    const float startWeight,
                                                    const float endWeight);
  static CAdvancementDeltas BlendTranslationOnly(const CAdvancementDeltas& a,
                                                 const CAdvancementDeltas& b, const float t);
  static CAdvancementDeltas BlendRotationOnly(const CAdvancementDeltas& a,
                                              const CAdvancementDeltas& b, const float t);

  CVector3f mPosDelta;
  CQuaternion mRotDelta;
};
CHECK_SIZEOF(CAdvancementDeltas, 0x1c)

namespace rstl {
RSTL_DECLARE_COPY_CONSTRUCTION(CAdvancementDeltas)
}

#endif // _CADVANCEMENTDELTAS_HPP
