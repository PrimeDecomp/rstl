#ifndef _CLOSEENOUGH
#define _CLOSEENOUGH
#include "rstl/RstlVersions.h"

#include "rstl/types.h"

#include "Kyoto/Math/CMath.hpp"

#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "math.h"

struct Real32 {
  static inline float Epsilon() { return 1.e-5f; }
};
struct Double {
  static inline double Epsilon() {
#if RSTL_PRIME1
    return 1.0 / 60000.0;
#else
    return 1.e-5; // TODO check
#endif
  }
};
static inline float vector3_epsilon() { return 1.e-4f; }
static inline float vector2_epsilon() { return 1.e-4f; }

#if !RSTL_PRIME1
class CQuaternion;

bool close_enough(const CQuaternion& a, const CQuaternion& b, float epsilon);
bool operator==(const CQuaternion& a, const CQuaternion& b);
#endif

bool close_enough(const CVector2f& a, const CVector2f& b, float epsilon = vector2_epsilon());
bool close_enough(const CVector3f& a, const CVector3f& b, float epsilon = vector3_epsilon());
inline bool close_enough(float a, RSTL_ECHOES_CONST float b, float epsilon = Real32::Epsilon()) {
#if !RSTL_PRIME1
  return CMath::AbsF(a - b) < epsilon;
#elif RSTL_VERSION >= RSTL_R3IJ
  return fabsf(a - b) < epsilon;
#else
  return fabs(a - b) < epsilon;
#endif
}
inline bool close_enough(double a, double b, double epsilon = Double::Epsilon()) {
  return fabs(a - b) < epsilon;
}

#endif // _CLOSEENOUGH
