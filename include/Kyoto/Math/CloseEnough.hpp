#ifndef _CLOSEENOUGH
#define _CLOSEENOUGH
#include "rstl/RstlVersions.h"

#include "types.h"

#if !RSTL_PRIME1
#include "Kyoto/Math/CMath.hpp"
#endif

#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "math.h"

struct Real32 {
  static inline float Epsilon() { return 1.e-5f; }
};
struct Double {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static inline double Epsilon() { return 1.0 / 60000.0; }
#else
  static inline double Epsilon() { return 1.e-5; } // TODO check
#endif

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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
inline bool close_enough(float a, float b, float epsilon = Real32::Epsilon()) {
#if RSTL_VERSION >= RSTL_R3IJ
  return fabsf(a - b) < epsilon;
#else
  return fabs(a - b) < epsilon;
#endif
#else
inline bool close_enough(float a, const float b, float epsilon = Real32::Epsilon()) {
  return CMath::AbsF(a - b) < epsilon;
#endif

}
inline bool close_enough(double a, double b, double epsilon = Double::Epsilon()) {
  return fabs(a - b) < epsilon;
}

#endif // _CLOSEENOUGH
