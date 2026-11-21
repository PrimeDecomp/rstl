#ifndef _PORTALPLANE
#define _PORTALPLANE
#include "rstl/RstlVersions.h"

class CPlane;
class CTransform4f;
#if RSTL_CORRUPTION
class CColor;
#endif

// Guessed namespace and function names for the portal alpha-mask projection utility.
namespace PortalPlane {
void SetWorldSpacePlane(const CPlane& plane, const CTransform4f& modelView,
                        const CTransform4f& model);
void SetCurrentPlane(const CPlane& plane);
const CTransform4f& GetTextureTransform();
#if RSTL_CORRUPTION
// Guessed names; separate from the alpha-mask projection above.
extern CTransform4f sMaterialProjectionTransform;
inline const CTransform4f& GetMaterialProjectionTransform() { return sMaterialProjectionTransform; }
// Guessed names; the INCA color is set by actor rendering and read by CCubeMaterial.
extern CColor sOptionalINCAColor;
#endif
} // namespace PortalPlane

#endif // _PORTALPLANE
