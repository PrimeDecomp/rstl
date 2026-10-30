#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CAdvancementDeltas.hpp"

#include "Kyoto/Animation/CAnimMathUtils.hpp"

CAdvancementDeltas CAdvancementDeltas::Interpolate(const CAdvancementDeltas& a,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                                                   const CAdvancementDeltas& b, const float c,
                                                   const float d) {
  return CAdvancementDeltas(
      (c + d) * b.GetOffsetDelta() * 0.5f - a.GetOffsetDelta() * ((c + d) - 2.f) * 0.5f,
      CAnimMathUtils::Slerp(a.GetOrientationDelta(), b.GetOrientationDelta(), (c + d) * 0.5f));
#else
                                                   const CAdvancementDeltas& b,
                                                   const float startWeight, const float endWeight) {
  return CAdvancementDeltas((startWeight + endWeight) * b.GetOffsetDelta() * 0.5f -
                                a.GetOffsetDelta() * ((startWeight + endWeight) - 2.f) * 0.5f,
                            CAnimMathUtils::Slerp(a.GetOrientationDelta(), b.GetOrientationDelta(),
                                                  (startWeight + endWeight) * 0.5f));
#endif

}
CAdvancementDeltas CAdvancementDeltas::Blend(const CAdvancementDeltas& a,
                                             const CAdvancementDeltas& b, const float t) {

  return CAdvancementDeltas(
      CVector3f::Lerp(a.GetOffsetDelta(), b.GetOffsetDelta(), t),
      CAnimMathUtils::Slerp(a.GetOrientationDelta(), b.GetOrientationDelta(), t));
}
