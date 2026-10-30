#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CAnimMathUtils.hpp"
#include "Kyoto/Math/CMath.hpp"

#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/math.hpp"
#include "rstl/vector.hpp"
#if !RSTL_PRIME1
#include "math.h"
#endif

const float CAnimMathUtils::kInterpolationThreshold = 0.0001f;
#if !RSTL_PRIME1
bool CAnimMathUtils::sUseFastSlerp; // Guessed name.

// Guessed name; stands in for a stripped function that read quaternion lists.
rstl::vector< CQuaternion > LoadQuaternions(CInputStream& in) {
  return rstl::vector< CQuaternion >(in);
}
#endif

CQuaternion CAnimMathUtils::Slerp(const CQuaternion& start, const CQuaternion& end, float t) {
#if RSTL_PRIME1
  if (t <= 0.00001f)
    return start;
  if (t >= 0.99999f)
    return end;
#endif

  const float dot = CQuaternion::Dot(start, end);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (dot >= 1.f)
    return start;
  if (dot > 0.95f) {
#else
  if (dot < -0.9999999f) {
    return t < 0.5f ? start : end;
  }

  if (sUseFastSlerp) {
    if (dot >= 0.866f) {
#endif

    const float scalar = start.GetScalar() + t * (end.GetScalar() - start.GetScalar());
    const CVector3f vector = start.GetVector() + t * (end.GetVector() - start.GetVector());
    const float invMag = CMath::InvSqrtF(scalar * scalar + vector.MagSquared());
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    return CQuaternion(invMag * scalar, invMag * vector);
  } else {
    const float angle = CMath::FastArcCosR(dot);
    const float a = CMath::FastSinR(angle * (1.f - t));
    const float b = CMath::FastSinR(angle * t);
    const float scalar = a * start.GetScalar() + b * end.GetScalar();
    const CVector3f vector = a * start.GetVector() + b * end.GetVector();
    const float invMag = CMath::InvSqrtF(scalar * scalar + vector.MagSquared());
    return CQuaternion(invMag * scalar, invMag * vector);
#else
      return CQuaternion(scalar * invMag, invMag * vector);
#endif

  }
#if !RSTL_PRIME1
    const double angle = acosf(dot);
    const double sineAngle = sin(angle);
    const double a = sin(angle * (1.f - t));
    const double b = sin(angle * t);
    const double invSine = 1.0 / sineAngle;
    const float aScale = a * invSine;
    const float bScale = b * invSine;
    return CQuaternion(aScale * start.GetScalar() + bScale * end.GetScalar(),
                       aScale * start.GetVector() + bScale * end.GetVector());
  }

  double w = start.GetScalar();
  double x = start.AxisX();
  double y = start.AxisY();
  double z = start.AxisZ();
  const double endW = end.GetScalar();
  const double endX = end.AxisX();
  const double endY = end.AxisY();
  const double endZ = end.AxisZ();
  const double product = w * endW + x * endX + y * endY + z * endZ;
  const double angle = acos(rstl::min_val(product, 1.0));
  const double sineAngle = sin(angle);
  if (sineAngle < 1e-99) {
    return start;
  }

  const double a = sin(angle * (1.f - t));
  const double b = sin(angle * t);
  const double invSine = 1.0 / sineAngle;
  const double bScale = b * invSine;
  const double aScale = a * invSine;
  w = w * aScale + endW * bScale;
  x = x * aScale + endX * bScale;
  y = y * aScale + endY * bScale;
  z = z * aScale + endZ * bScale;
  return CQuaternion(w, x, y, z);
#endif

}

CQuaternion CAnimMathUtils::SlerpLocal(const CQuaternion& start, const CQuaternion& end, float t) {
  return CQuaternion::Dot(start, end) >= 0.f ? Slerp(start, end, t)
                                          : Slerp(start, end.BuildEquivalent(), t);
}

