#ifndef _CANIMMATHUTILS
#define _CANIMMATHUTILS
#include "rstl/RstlVersions.h"

#include "Kyoto/Math/CMath.hpp"
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
#include "EchoesNamespace.h"
#endif

class CQuaternion;

class CAnimMathUtils {
public:
  static const float kInterpolationThreshold;
#if !RSTL_PRIME1
  static bool sUseFastSlerp; // Guessed name.
#endif

  static CQuaternion Slerp(const CQuaternion& start, const CQuaternion& end, float t);
  static CQuaternion SlerpLocal(const CQuaternion& start, const CQuaternion& end, float t);
};

#endif // _CANIMMATHUTILS
