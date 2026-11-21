#include "Kyoto/Graphics/PortalPlane.hpp"

#include "rstl/RstlVersions.h"

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CVector3f.hpp"

namespace PortalPlane {
CTransform4f sTextureTransform = CTransform4f::Identity();
CPlane sWorldSpacePlane(0.f, CVector3f::Forward());
} // namespace PortalPlane

namespace {
bool sHasPlane = false;
} // namespace

#if RSTL_KYOTO_PORTAL_PLANE_MATERIAL_GLOBALS
CTransform4f PortalPlane::sMaterialProjectionTransform = CTransform4f::Identity();
// Guessed name
static CColor sColor8079B5DC(CColor::White());
CColor PortalPlane::sOptionalINCAColor(CColor::Black());
#endif

const CTransform4f& PortalPlane::GetTextureTransform() { return sTextureTransform; }

void PortalPlane::SetCurrentPlane(const CPlane& plane) {
  SetWorldSpacePlane(plane, CGraphics::GetGXModelView(), CGraphics::GetModelMatrix());
}

void PortalPlane::SetWorldSpacePlane(const CPlane& plane, const CTransform4f& modelView,
                                     const CTransform4f& model) {
  sWorldSpacePlane = plane;
  sHasPlane = true;

  const CTransform4f inverseModelView = modelView.GetInverse();
  const CTransform4f inverseModel = model.GetInverse();
  const CPlane localPlane(inverseModel * (plane.GetConstant() * plane.GetNormal()),
                          CUnitVector3f(inverseModel.Rotate(plane.GetNormal())));

  const CTransform4f projection = CTransform4f::FromRows(
      CVector3f(0.f, 0.f, 0.f), localPlane.GetNormal(), CVector3f(0.f, 0.f, 0.f),
      CVector3f(0.5f, 0.5f - localPlane.GetConstant(), 1.f));
  sTextureTransform = projection * inverseModelView;
}

