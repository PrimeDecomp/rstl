#include "rstl/RstlVersions.h"
#include "Kyoto/Graphics/CCubeSurface.hpp"

#if RSTL_CORRUPTION


const CVector3f CCubeSurface::skDefaultNormal(1.f, 0.f, 0.f);
#else
const CVector3f CCubeSurface::skDefaultNormal = CUnitVector3f(1.f, 0.f, 0.f);
#endif

CAABox CCubeSurface::GetBounds() const {
#if RSTL_CORRUPTION
  if (mData->mExtraSize >= sizeof(CAABox)) {
#else
  if (mData->mExtraSize != 0) {
#endif
    return mData->mBounds;
  }

  return CAABox(mData->mCenter, mData->mCenter);
}
#if RSTL_CORRUPTION

uchar CCubeSurface::GetDrawMaskBitIndex() const {
  const uchar* extra = reinterpret_cast< const uchar* >(&mData->mBounds);
  switch (mData->mExtraSize) {
  case 1:
    return extra[0];
  case sizeof(CAABox) + 1:
    return extra[sizeof(CAABox)];
  default:
    return 0xff;
  }
}
#endif
