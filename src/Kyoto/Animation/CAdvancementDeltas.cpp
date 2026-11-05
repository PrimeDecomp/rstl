#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CAdvancementDeltas.hpp"

#include "Kyoto/Animation/CAnimMathUtils.hpp"

CAdvancementDeltas CAdvancementDeltas::Interpolate(const CAdvancementDeltas& a,
                                                   const CAdvancementDeltas& b,
                                                   const float startWeight,
                                                   const float endWeight) {
  return CAdvancementDeltas((startWeight + endWeight) * b.GetOffsetDelta() * 0.5f -
                                a.GetOffsetDelta() * ((startWeight + endWeight) - 2.f) * 0.5f,
                            CAnimMathUtils::Slerp(a.GetOrientationDelta(), b.GetOrientationDelta(),
                                                  (startWeight + endWeight) * 0.5f));
}

#if RSTL_KYOTO_ADVANCEMENT_PARTIAL_BLENDS
CAdvancementDeltas CAdvancementDeltas::InterpolateTranslationOnly(const CAdvancementDeltas& a,
                                                                  const CAdvancementDeltas& b,
                                                                  const float startWeight,
                                                                  const float endWeight) {
  return CAdvancementDeltas((startWeight + endWeight) * b.GetOffsetDelta() * 0.5f -
                                a.GetOffsetDelta() * ((startWeight + endWeight) - 2.f) * 0.5f,
                            b.GetOrientationDelta());
}

CAdvancementDeltas CAdvancementDeltas::InterpolateRotationOnly(const CAdvancementDeltas& a,
                                                               const CAdvancementDeltas& b,
                                                               const float startWeight,
                                                               const float endWeight) {
  return CAdvancementDeltas(b.GetOffsetDelta(),
                            CAnimMathUtils::Slerp(a.GetOrientationDelta(), b.GetOrientationDelta(),
                                                  (startWeight + endWeight) * 0.5f));
}
#endif

CAdvancementDeltas CAdvancementDeltas::Blend(const CAdvancementDeltas& a,
                                             const CAdvancementDeltas& b, const float t) {

  return CAdvancementDeltas(
      CVector3f::Lerp(a.GetOffsetDelta(), b.GetOffsetDelta(), t),
      CAnimMathUtils::Slerp(a.GetOrientationDelta(), b.GetOrientationDelta(), t));
}

#if RSTL_KYOTO_ADVANCEMENT_PARTIAL_BLENDS
CAdvancementDeltas CAdvancementDeltas::BlendTranslationOnly(const CAdvancementDeltas& a,
                                                            const CAdvancementDeltas& b,
                                                            const float t) {
  return CAdvancementDeltas(CVector3f::Lerp(a.GetOffsetDelta(), b.GetOffsetDelta(), t),
                            b.GetOrientationDelta());
}

CAdvancementDeltas CAdvancementDeltas::BlendRotationOnly(const CAdvancementDeltas& a,
                                                         const CAdvancementDeltas& b,
                                                         const float t) {
  return CAdvancementDeltas(
      b.GetOffsetDelta(),
      CAnimMathUtils::Slerp(a.GetOrientationDelta(), b.GetOrientationDelta(), t));
}
#endif
