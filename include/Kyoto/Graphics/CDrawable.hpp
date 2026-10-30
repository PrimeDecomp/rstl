#ifndef _CDRAWABLE
#define _CDRAWABLE
#include "rstl/RstlVersions.h"
#include "types.h"

#include "Kyoto/Math/CAABox.hpp"

enum EDrawableType {
  kDT_WorldSurface,
  kDT_Particle,
  kDT_Actor,
  kDT_SimpleShadow,
  kDT_Decal,
#if RSTL_PRIME1
  kDT_Invalid = 0xFFFF,
#endif

};

class CDrawable {
public:
  CDrawable(EDrawableType type, ushort extraSort, float distance, const CAABox& bounds,
            const void* data
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
            , bool alpha
#endif
            )
  : mType(type)
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
  , mAlpha(alpha)
#endif
  , mExtraSort(extraSort)
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  , mData(const_cast< void* >(data))
#else
  , mData(data)
#endif
  , mBounds(bounds)
  , mViewDistance(distance) {}
  // CDrawable(const CDrawable& other)
  // : x0_type(other.x0_type)
  // , x2_extraSort(other.x2_extraSort)
  // , x4_data(other.x4_data)
  // , x8_aabb(other.x8_aabb)
  // , x20_viewDist(other.x20_viewDist) {}

  EDrawableType GetType() const { return static_cast< EDrawableType >(mType); }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const CAABox& GetBounds() const { return mBounds; }
  float GetDistance() const { return mViewDistance; }
  void* GetData() { return mData; }
#else
  bool IsAlpha() const { return mAlpha; }
  ushort GetExtraSort() const { return mExtraSort; }
#endif

  const void* GetData() const { return mData; }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  u16 GetExtraSort() const { return mExtraSort; }
#else
  const CAABox& GetBounds() const { return mBounds; }
  float GetDistance() const { return mViewDistance; }
#endif

private:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  ushort mType;
#else
  ushort mType : 8;
  ushort mAlpha : 1;
#endif

  ushort mExtraSort;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  void* mData;
#else
  const void* mData;
#endif
  CAABox mBounds;
  float mViewDistance;

};
#if !RSTL_PRIME1
DECLARE_FULL_SIZE_FOR(CDrawable, 0x24)
CHECK_SIZEOF(CDrawable, CDrawable_FULL_SIZE)
#endif

#endif // _CDRAWABLE
