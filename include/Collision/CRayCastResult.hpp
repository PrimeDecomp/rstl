#ifndef _CRAYCASTRESULT
#define _CRAYCASTRESULT

#include "rstl/RstlVersions.h"
#include "rstl/static_assert.hpp"
#include "rstl/types.h"

#include "Collision/CMaterialList.hpp"
#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector3f.hpp"

class CRayCastResult {
public:
  // Guessed enum names; the native invalid constructor takes a zero tag.
  enum EInvalid {
    kI_Invalid,
    kI_Valid,
  };

  CRayCastResult(float time, const CVector3f& point, const CPlane& plane,
                 const CMaterialList& material)
  : mTime(time)
  , mPoint(point)
  , mPlane(plane)
#if RSTL_PRIME1
  , mValid(kI_Valid)
  , mMaterial(material)
#else
  , mMaterial(material)
  , mValid(true)
#endif
  {
  }
  CRayCastResult(EInvalid = kI_Invalid)
  : mTime(0.f)
  , mPoint(0.f, 0.f, 0.f)
#if RSTL_PRIME1
  , mPlane(0.f, CUnitVector3f(1.f, 0.f, 0.f, CUnitVector3f::kN_Yes))
  , mValid(kI_Invalid)
#else
  , mPlane(0.f, CVector3f::Right())
  , mMaterial()
  , mValid(false)
#endif
  {
  }

  static CRayCastResult MakeInvalid() { return CRayCastResult(); }

  float GetTime() const { return mTime; }
  const CVector3f& GetPoint() const { return mPoint; }
  const CPlane& GetPlane() const { return mPlane; }
  const CMaterialList& GetMaterial() const { return mMaterial; }
  RSTL_PRIME1_CONST bool IsValid() const { return mValid; }
  bool GetValid() const { return mValid; }
  // TODO: figure out what's going on here
  bool IsInvalid() const { return mValid == kI_Invalid; }
  void Transform(const CTransform4f& xf);

private:
  float mTime;
  CVector3f mPoint;
  CPlane mPlane;
#if RSTL_PRIME1
  bool mValid;
  CMaterialList mMaterial;
#else
  CMaterialList mMaterial;
  bool mValid;
#endif
};
CHECK_SIZEOF(CRayCastResult, 0x30)

#if !RSTL_PRIME1
inline void CRayCastResult::Transform(const CTransform4f& xf) {
  mPoint = xf * mPoint;
  CVector3f normal = xf.Rotate(mPlane.GetNormal());
  mPlane = CPlane(mPoint, CUnitVector3f(normal.GetX(), normal.GetY(), normal.GetZ()));
}
#endif

#endif // _CRAYCASTRESULT
