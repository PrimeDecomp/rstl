#ifndef _CPLANE
#define _CPLANE
#include "rstl/RstlVersions.h"

#include "types.h"

#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#if !RSTL_PRIME1
class CInputStream;
#endif

class CPlane {
public:
#if !RSTL_PRIME1
  CPlane(CInputStream& in);
#endif

  CPlane(const CVector3f& vec, const CUnitVector3f& normal)
  : mNormal(normal), mConstant(CVector3f::Dot(vec, normal)) {}
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CPlane(const float constant, const CUnitVector3f& normal)
  : mNormal(normal), mConstant(constant) {}
#else
  CPlane(const float constant, const CUnitVector3f& normal);
#endif

  CPlane(const CVector3f&, const CVector3f&, const CVector3f&);
  // TODO

  const CUnitVector3f& GetNormal() const { return mNormal; }
  float GetConstant() const { return mConstant; }
  float GetHeight(const CVector3f& pos) const {
    return CVector3f::Dot(GetNormal(), pos) - GetConstant();
  }
  bool IsFacing(const CVector3f& vec) const {
    return CVector3f::Dot(mNormal, vec) >= GetConstant();
  }
#if !RSTL_PRIME1
  CVector3f GetClosestPoint(const CVector3f& point) const;
  float ClipLineSegment(const CVector3f& start, const CVector3f& end) const;
  // Guessed names, corroborated by later-engine metadata.
  void SetCenter(CVector3f point) { mConstant = CVector3f::Dot(point, mNormal); }
  void SetNormal(const CUnitVector3f& normal) { mNormal = normal; }
#endif

#if RSTL_PRIME1
  float ClipLineSegment(const CVector3f& start, const CVector3f& end) const;

  // TODO: Fake?
#endif

  void SetFrom(const float d, const CVector3f& vec) {
    mNormal.SetX(vec.GetX());
    mNormal.SetY(vec.GetY());
    mNormal.SetZ(vec.GetZ());
    mConstant = d;
  }

private:
  CUnitVector3f mNormal;
  float mConstant;
};
CHECK_SIZEOF(CPlane, 0x10)

#if !RSTL_PRIME1
#ifdef __MWERKS__
#pragma cpp_extensions on
#endif
inline CPlane::CPlane(const float constant, const CUnitVector3f& normal)
: mNormal(normal), mConstant(constant) {}
#ifdef __MWERKS__
#pragma cpp_extensions reset
#endif
#endif

#if !RSTL_PRIME1
namespace rstl {
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(CPlane)
} // namespace rstl
#endif

#endif // _CPLANE
