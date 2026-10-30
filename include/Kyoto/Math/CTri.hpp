#ifndef _CTRI
#define _CTRI
#include "rstl/RstlVersions.h"

#include "types.h"

#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/Math/CVector3f.hpp"

class CTri {
public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CTri(const CVector3f& a, const CVector3f& b, const CVector3f& c)
  : mPlane(a, b, c), mA(a), mB(b), mC(c) {}
#else
  CTri(const CVector3f& a, const CVector3f& b, const CVector3f& c);
#endif

  const CPlane& GetPlane() const { return mPlane; }
  const CVector3f& GetPointA() const { return mA; }
  const CVector3f& GetPointB() const { return mB; }
  const CVector3f& GetPointC() const { return mC; }
#if RSTL_PRIME1
  // TODO: Verify, this should be right though
  CPlane GetState() const { return CPlane(mA, mB, mC); }
#endif

private:
  CPlane mPlane;
  CVector3f mA;
  CVector3f mB;
  CVector3f mC;
};
CHECK_SIZEOF(CTri, 0x34)

#endif // _CTRI
