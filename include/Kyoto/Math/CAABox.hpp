#ifndef _CAABOX
#define _CAABOX
#include "rstl/RstlVersions.h"

#include "Kyoto/Math/CVector3f.hpp"
#include "rstl/pair.hpp"

class CInputStream;

class CLineSeg;
class CPlane;
class CQuad;

class CTransform4f;
class CTri;

class CAABox {
public:
  enum EBoxEdgeId {
    kE_Z0,
    kE_X0,
    kE_Z1,
    kE_X1,
    kE_Z2,
    kE_X2,
    kE_Z3,
    kE_X3,
    kE_Y0,
    kE_Y1,
    kE_Y2,
    kE_Y3,
  };
  enum EBoxFaceId {
    kF_YMin,
    kF_YMax,
    kF_XMin,
    kF_XMax,
    kF_ZMax,
    kF_ZMin,
  };

#if RSTL_PRIME1
  CAABox() : min(CVector3f::Zero()), max(CVector3f::Zero()) {}
#endif

  CAABox(const CVector3f& min, const CVector3f& max);
  CAABox(RSTL_PRIME1_CONST float minX, RSTL_PRIME1_CONST float minY, RSTL_PRIME1_CONST float minZ,
         RSTL_PRIME1_CONST float maxX, RSTL_PRIME1_CONST float maxY, RSTL_PRIME1_CONST float maxZ)
  : min(minX, minY, minZ), max(maxX, maxY, maxZ) {}
  CAABox(CInputStream& in);
  // CAABox(const CAABox& other)
  // : min(other.min)
  // , max(other.max) {}

  // CAABox& operator=(const CAABox& other) {
  //   min = other.min;
  //   max = other.max;
  //   return *this;
  // }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CLineSeg GetEdge(EBoxEdgeId edge) const;
#else
  // Guessed name; the G2ME01 target returns two corner indices.
  static rstl::pair< int, int > GetEdge(EBoxEdgeId edge);
#endif

  CTri GetTri(EBoxFaceId face, int windOffset) const;
#if !RSTL_PRIME1
  // Guessed name for the G2ME01 four-corner face method.
  CQuad GetQuad(EBoxFaceId face) const;
#endif

  CVector3f ClosestPointAlongVector(const CVector3f& vec) const;
  CVector3f FurthestPointAlongVector(const CVector3f& vec) const;
  CVector3f GetCenterPoint() const;
#if RSTL_PRIME1
  CVector3f CenterPoint() const { return (min + max) * 0.5f; }
#endif

  CVector3f GetPoint(int) const;
#if !RSTL_PRIME1
  // Include__6CAABoxFRC9CVector3f weak
  // Include__6CAABoxFRC6CAABox weak
  void AccumulateBounds(const CVector3f&);
#endif

  void Include(const CAABox& box) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    AccumulateBounds(box.GetMinPoint());
    AccumulateBounds(box.GetMaxPoint());
#else
    AccumulateBounds(box.min);
    AccumulateBounds(box.max);
#endif
  }
#if RSTL_PRIME1
  void Include(const CVector3f& vec) { AccumulateBounds(vec); }
  void AccumulateBounds(const CVector3f&);
#endif

  bool Invalid() const;
  bool PointInside(const CVector3f& vec) const;
  bool DoBoundsOverlap(const CAABox&) const;
  float GetVolume() const;
  CAABox GetBooleanIntersection(const CAABox& other) const;
  bool Inside(const CAABox& other) const;
  bool InsidePlane(const CPlane& plane) const;
  CVector3f ClampToBox(const CVector3f& vec) const;
  CAABox GetTransformedAABox(const CTransform4f& xf) const;

  inline CVector3f GetHalfExtent() const {
    // Name is a guess
    return (max - min) * 0.5f;
  }

  const CVector3f& GetMinPoint() const { return min; }
  const CVector3f& GetMaxPoint() const { return max; }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CVector3f GetPointA() const { return CVector3f(min.GetX(), min.GetY(), min.GetZ()); }
  CVector3f GetPointB() const { return CVector3f(max.GetX(), min.GetY(), min.GetZ()); }
  CVector3f GetPointC() const { return CVector3f(max.GetX(), min.GetY(), max.GetZ()); }
  CVector3f GetPointD() const { return CVector3f(min.GetX(), min.GetY(), max.GetZ()); }
  CVector3f GetPointE() const { return CVector3f(max.GetX(), max.GetY(), min.GetZ()); }
  CVector3f GetPointF() const { return CVector3f(min.GetX(), max.GetY(), max.GetZ()); }
  CVector3f GetPointG() const { return CVector3f(max.GetX(), max.GetY(), max.GetZ()); }
  CVector3f GetPointH() const { return CVector3f(max.GetX(), max.GetY(), min.GetZ()); }
#else
  // GetPointA__6CAABoxCFv weak
  // GetPointB__6CAABoxCFv weak
  // GetPointC__6CAABoxCFv weak
  // GetPointD__6CAABoxCFv weak
  // GetPointE__6CAABoxCFv weak
  // Corner order guessed to follow GetPoint(int): bit 0 = X, bit 1 = Y, bit 2 = Z.
  CVector3f GetPointE() const { return CVector3f(min.GetX(), min.GetY(), max.GetZ()); }
  // GetPointF__6CAABoxCFv weak
  // GetPointG__6CAABoxCFv weak
  // GetPointH__6CAABoxCFv weak
#endif

  float GetWidth() const { return max.GetX() - min.GetX(); }
  float GetHeight() const { return max.GetY() - min.GetY(); }
  float GetDepth() const { return max.GetZ() - min.GetZ(); }
  // GetTri__6CAABoxCFii weak
  // GetEdge__6CAABoxCFi weak

  static const CAABox& Identity() { return mskNullBox; }
  static const CAABox& MakeMaxInvertedBox() { return mskInvertedBox; }
  static const CAABox& MakeNullBox() { return mskNullBox; }
  static float DistanceBetween(const CAABox& a, const CAABox& b);

private:
  CVector3f min;
  CVector3f max;

  static CAABox mskInvertedBox;
  static CAABox mskNullBox;
};
CHECK_SIZEOF(CAABox, 0x18)

#if !RSTL_PRIME1
namespace rstl {
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(CAABox)
RSTL_DECLARE_DIRECT_CONSTRUCTION(CAABox)
} // namespace rstl
#endif

#endif // _CAABOX
