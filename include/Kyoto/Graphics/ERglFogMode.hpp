#ifndef _ERGLFOGMODE
#define _ERGLFOGMODE

#include "dolphin/gx.h"

enum ERglFogMode {
  kRFM_None = GX_FOG_NONE,

  kRFM_PerspLin = GX_FOG_PERSP_LIN,
  kRFM_PerspExp = GX_FOG_PERSP_EXP,
  kRFM_PerspExp2 = GX_FOG_PERSP_EXP2,
  kRFM_PerspRevExp = GX_FOG_PERSP_REVEXP,
  kRFM_PerspRevExp2 = GX_FOG_PERSP_REVEXP2,

  kRFM_OrthoLin = GX_FOG_ORTHO_LIN,
  kRFM_OrthoExp = GX_FOG_ORTHO_EXP,
  kRFM_OrthoExp2 = GX_FOG_ORTHO_EXP2,
  kRFM_OrthoRevExp = GX_FOG_ORTHO_REVEXP,
  kRFM_OrthoRevExp2 = GX_FOG_ORTHO_REVEXP2,
};

#endif // _ERGLFOGMODE
