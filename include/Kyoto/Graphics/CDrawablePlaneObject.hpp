#ifndef _CDRAWABLEPLANEOBJECT
#define _CDRAWABLEPLANEOBJECT
#include "rstl/RstlVersions.h"
#include "types.h"

#include "Kyoto/Graphics/CDrawable.hpp"
#include "Kyoto/Math/CAABox.hpp"

#include "Kyoto/Math/CPlane.hpp"

class CDrawablePlaneObject : public CDrawable {
public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CDrawablePlaneObject(EDrawableType dtype, float closeDist, float farDist, const CAABox& aabb,
                       bool invertTest, const CPlane& plane, bool zOnly, const void* data)
  : CDrawable(dtype, 0, closeDist, aabb, data)
#else
  CDrawablePlaneObject(EDrawableType type, float closeDistance, float farDistance,
                       const CAABox& bounds, bool invertTest, const CPlane& plane, bool zOnly,
                       const void* data)
  : CDrawable(type, 0, closeDistance, bounds, data, false)
#endif

  , mTargetBucket(0)
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  , mFarDist(farDist)
#else
  , mFarDistance(farDistance)
#endif

  , mPlane(plane)
  , mInvertTest(invertTest)
  , mZOnly(zOnly) {}

  ushort GetBucketIndex() const { return mTargetBucket; }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  void SetBucketIndex(ushort idx) { mTargetBucket = idx; }
#else
  void SetBucketIndex(ushort index) { mTargetBucket = index; }
#endif

  const CPlane& GetPlane() const { return mPlane; }
  bool IsViewInFront() const { return mInvertTest; }
  bool IsOptimalPlane() const { return mZOnly; }

private:
  ushort mTargetBucket;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  float mFarDist;
#else
  float mFarDistance;
#endif

  CPlane mPlane;
  bool mInvertTest : 1;
  bool mZOnly : 1;
};
#if !RSTL_PRIME1
CHECK_CHILD_SIZEOF(CDrawablePlaneObject, CDrawable, 0x1c)
#endif

#endif // _CDRAWABLEPLANEOBJECT
