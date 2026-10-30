#ifndef _CVIRTUALBONE
#define _CVIRTUALBONE
#include "rstl/RstlVersions.h"

#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Math/CTransform4f.hpp"

#include "rstl/reserved_vector.hpp"

#if !RSTL_PRIME1
#include "dolphin/mtx.h"
#endif

struct SSkinWeighting {
  CSegId mId;
  float mWeight;
  explicit SSkinWeighting(CInputStream& in)
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  : mId((uchar)in.ReadLong())
  , mWeight(in.ReadFloat())
#else
  : mId(in.Get< int >())
  , mWeight(in.Get< float >())
#endif
  {}
};
#if !RSTL_PRIME1
CHECK_SIZEOF(SSkinWeighting, 0x8)
#endif

class CPoseAsTransforms;
#if !RSTL_PRIME1
// Guessed name. GPU matrix records occupy three cache lines.
struct SSkinningMatrices {
  ATTRIBUTE_ALIGN_DECL(32, Mtx mPosition);
  float mNormal[3][3];
};
CHECK_SIZEOF(SSkinningMatrices, 0x60)

class CPoseAsTransforms_Linear;
#endif

class CVirtualBone {
public:
  explicit CVirtualBone(CInputStream& in);
  const rstl::reserved_vector< SSkinWeighting, 3 >& GetWeights() const { return mWeights; }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  int GetNumIndices() const { return mVertexCount; }
  const CTransform4f& GetTransform() const { return mXf; }
#else
  uint GetVertexCount() const { return mVertexCount; }
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  void BuildPoints(const ushort*, volatile void*, int) const;
  void BuildNormals(const ushort*, volatile void*, int) const;
  void BuildNormals(const CVector3f*, CVector3f*, int) const;
  void BuildAccumulatedTransform(const CPoseAsTransforms& pose, const CVector3f* points) const;
  void BuildFinalPosMatrix(const CPoseAsTransforms& pose, const CVector3f* points) const;
#else
  void BuildAccumulatedTransform(const CPoseAsTransforms_Linear& pose, const CVector3f* points,
                                 CTransform4f& out) const;
  void BuildFinalPosMatrix(const CPoseAsTransforms_Linear& pose, const CVector3f* points,
                           CTransform4f& out) const;
  // Guessed name.
  void BuildSkinningMatrices(const CTransform4f& xf, SSkinningMatrices& out,
                             const bool uniformScale) const;
#endif

private:
  rstl::reserved_vector< SSkinWeighting, 3 > mWeights;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  int mVertexCount;
  mutable CTransform4f mXf;
  mutable CMatrix3f mRotation;
#else
  uint mVertexCount;
#endif
};
CHECK_SIZEOF(CVirtualBone, RSTL_PRIME1 ? 0x74 : 0x20)

#endif // _CVIRTUALBONE
