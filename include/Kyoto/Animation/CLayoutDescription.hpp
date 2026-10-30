#ifndef _CLAYOUTDESCRIPTION
#define _CLAYOUTDESCRIPTION
#include "rstl/RstlVersions.h"

#include "Kyoto/TToken.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/pair.hpp"

class CCharLayoutInfo;
class CLayoutScaleInfo;

class CLayoutDescription {
public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CLayoutDescription(const TLockedToken< CCharLayoutInfo >& layout) : mLayoutToken(layout) {}
#else
  explicit CLayoutDescription(const TLockedToken< CCharLayoutInfo >& layout)
  : mLayoutToken(layout) {}
#endif

  class CScaledLayoutDescription {
  public:
#if !RSTL_PRIME1
    typedef rstl::pair< rstl::pair< const CLayoutScaleInfo*, const CLayoutScaleInfo* >, float >
        ScaleInfo;
#endif

    const TLockedToken< CCharLayoutInfo >& ScaledLayout() const { return mLayoutToken; }
#if RSTL_PRIME1
    float GlobalScale() const { return mScale; }
#endif

  private:
    TLockedToken< CCharLayoutInfo > mLayoutToken;
    float mScale;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    rstl::optional_object<
        rstl::pair< rstl::pair< const CLayoutScaleInfo*, const CLayoutScaleInfo* >, float > >
        mScaleInfo;
#else
    rstl::optional_object< ScaleInfo > mScaleInfo;
#endif

  };

  uchar GetNumSegments() const;
  bool UsesScale() const { return mScaled.valid(); }
#if RSTL_PRIME1
  float GlobalScale() const { return UsesScale() ? mScaled->GlobalScale() : 1.f; }
#endif

  const TLockedToken< CCharLayoutInfo >& ScaledLayout() const {
    return UsesScale() ? mScaled->ScaledLayout() : mLayoutToken;
  }

private:
  TLockedToken< CCharLayoutInfo > mLayoutToken;
  rstl::optional_object< CScaledLayoutDescription > mScaled;
};
CHECK_SIZEOF(CLayoutDescription, 0x30)
NESTED_CHECK_SIZEOF(CLayoutDescription, CScaledLayoutDescription, 0x20)

#endif // _CLAYOUTDESCRIPTION
