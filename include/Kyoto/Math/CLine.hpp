#ifndef _CLINE
#define _CLINE
#include "rstl/RstlVersions.h"

#include "types.h"

#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CVector3f.hpp"

class CLine {
public:
  CLine(const CVector3f& origin, const CUnitVector3f& dir) : mOrigin(origin), mDir(dir) {}
  CLine(const CVector3f& origin, const CVector3f& dir)
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  : mOrigin(origin), mDir(*reinterpret_cast< const CUnitVector3f* >(&dir)) {}
#else
  : mOrigin(origin), mDir(dir, CUnitVector3f::kN_No) {}
#endif

#if !RSTL_PRIME1
  // Guessed names
  void SetRefPoint(const CVector3f& origin) { mOrigin = origin; }
  void SetNormal(const CUnitVector3f& dir) { mDir = dir; }
#endif

  const CVector3f& GetRefPoint() const { return mOrigin; }
  const CUnitVector3f& GetNormal() const { return mDir; }
#if !RSTL_PRIME1
  // Guessed name (target-derived: projects a point onto the line).
  CVector3f GetClosestPoint(const CVector3f& point) const;
#endif

private:
  CVector3f mOrigin;
  CUnitVector3f mDir;
};

#endif // _CLINE
