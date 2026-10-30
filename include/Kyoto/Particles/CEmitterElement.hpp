#ifndef _CEMITTERELEMENT
#define _CEMITTERELEMENT
#include "rstl/RstlVersions.h"

#if !RSTL_PRIME1
#include "Kyoto/Math/CTransform4f.hpp"
#endif

#include "Kyoto/Particles/IElement.hpp"

class CEESimpleEmitter : public CEmitterElement {
public:
  CEESimpleEmitter(CVectorElement* pos, CVectorElement* vel);
#if RSTL_PRIME1
  ~CEESimpleEmitter();
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  bool GetValue(int frame, CVector3f& pPos, CVector3f& pVel) const;
#else
  // IElement
  ~CEESimpleEmitter() override;

  // CEmitterElement
  bool GetValue(int frame, CVector3f& pPos, CVector3f& pVel) const override;
#endif

private:
  CVectorElement* mPos;
  CVectorElement* mVel;
};

class CVESphere : public CEmitterElement {
public:
  CVESphere(CVectorElement* origin, CRealElement* radius, CRealElement* velocityMag);
#if RSTL_PRIME1
  ~CVESphere();
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  bool GetValue(int frame, CVector3f& pPos, CVector3f& pVel) const;
#else
  // IElement
  ~CVESphere() override;

  // CEmitterElement
  bool GetValue(int frame, CVector3f& pPos, CVector3f& pVel) const override;
#endif

private:
  CVectorElement* mSphereOrigin;
  CRealElement* mSphereRadius;
  CRealElement* mVelocityMag;
};

#if !RSTL_PRIME1
// Echoes ELPS emitter: random point in (or on) a rotated ellipsoid.
// Guessed name
class CVEEllipsoid : public CEmitterElement {
public:
  CVEEllipsoid(CVectorElement* origin, CVectorElement* radii, CVectorElement* rotation,
               CRealElement* velocityMag, bool onSurface);

  // IElement
  ~CVEEllipsoid() override;

  // CEmitterElement
  bool GetValue(int frame, CVector3f& pPos, CVector3f& pVel) const override;

private:
  CVectorElement* mOrigin;
  CVectorElement* mRadii;
  CVectorElement* mRotation;
  CRealElement* mVelocityMag;
  bool mOnSurface;
};
#endif

class CVEAngleSphere : public CEmitterElement {
public:
  CVEAngleSphere(CVectorElement* origin, CRealElement* radius, CRealElement* velocityMag,
                 CRealElement* angleXBias, CRealElement* angleYBias, CRealElement* angleXRange,
                 CRealElement* angleYRange);
#if RSTL_PRIME1
  ~CVEAngleSphere();
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  bool GetValue(int frame, CVector3f& pPos, CVector3f& pVel) const;
#else
  // IElement
  ~CVEAngleSphere() override;

  // CEmitterElement
  bool GetValue(int frame, CVector3f& pPos, CVector3f& pVel) const override;
#endif

private:
  CVectorElement* mSphereOrigin;
  CRealElement* mSphereRadius;
  CRealElement* mVelocityMag;
  CRealElement* mAngleXBias;
  CRealElement* mAngleYBias;
  CRealElement* mAngleXRange;
  CRealElement* mAngleYRange;
};

#if !RSTL_PRIME1
// Echoes PLNE emitter: random point on a rotated rectangle, with an optional cone spread.
// Guessed name
class CEEPlaneEmitter : public CEmitterElement {
public:
  CEEPlaneEmitter(CVectorElement* translation, CVectorElement* rotation, CVectorElement* velocity,
                  CRealElement* xRange, CRealElement* yRange, CRealElement* coneAngle);
#endif

#if !RSTL_PRIME1
  // IElement
  ~CEEPlaneEmitter() override;

  // CEmitterElement
  bool GetValue(int frame, CVector3f& pPos, CVector3f& pVel) const override;

private:
  CVectorElement* mTranslation;
  CVectorElement* mRotation;
  CVectorElement* mVelocity;
  CRealElement* mXRange;
  CRealElement* mYRange;
  CRealElement* mConeAngleElement;
  mutable CTransform4f mTransform;
  mutable float mConeAngleRadians;
  mutable float mConeCos;
  mutable float mConeSin;
  bool mConstantTransform : 1;
  bool mHasCone : 1;
  bool mConstantCone : 1;
};

CHECK_SIZEOF(CVEEllipsoid, 0x18)
CHECK_SIZEOF(CEEPlaneEmitter, 0x5C)
#endif

#endif // _CEMITTERELEMENT
