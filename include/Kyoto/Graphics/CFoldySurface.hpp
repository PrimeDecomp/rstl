#ifndef _CFOLDYSURFACE
#define _CFOLDYSURFACE
#include "rstl/RstlVersions.h"

#include "rstl/static_assert.hpp"
#include "rstl/types.h"

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/single_ptr.hpp"
#if RSTL_CORRUPTION
#include "rstl/reserved_vector.hpp"

#include <dolphin/gx/GXEnum.h>


#endif

class CModel;
class CModelFlags;
class CTransform4f;

// Guessed name for the model owner that supplies three independently transformed X segments.
class CFoldySurface {
public:
  CFoldySurface(const TToken< CModel >& model, float lowerX, float upperX,
                const CVector3f& lowerOffset, const CVector3f& middleOffset,
                const CVector3f& upperOffset);
  ~CFoldySurface();

  void ResetRenderState() const;
  void SetMaterialCurrent(const CModelFlags& flags) const;
  void DrawDisplayList(int index) const;
  void SetSegmentTransforms(int matrixGroup, const CTransform4f& lower, const CTransform4f& middle,
                            const CTransform4f& upper) const;

private:
#if RSTL_CORRUPTION
  void BuildMatrixIndexAttributes(); // Guessed name

#endif
  // Guessed names for the three supported spatial segments.
  enum ESegment {
    kS_LowerX,
    kS_MiddleX,
    kS_UpperX,
    kS_Count,
  };

  TCachedToken< CModel > mModel;
  rstl::single_ptr< CVector3f > mPositions;
  int mVertexCount;
#if RSTL_CORRUPTION
  rstl::single_ptr< uchar > mDisplayLists[kS_Count];
#else
  rstl::single_ptr< void > mDisplayLists[kS_Count];
#endif
  uint mDisplayListSize;
#if RSTL_CORRUPTION
  rstl::reserved_vector< GXAttr, 9 > mMatrixIndexAttributes; // Guessed name
#endif
};
#if RSTL_CORRUPTION
CHECK_SIZEOF(CFoldySurface, 0x4c)
#else
CHECK_SIZEOF(CFoldySurface, 0x24)
#endif

#endif // _CFOLDYSURFACE
