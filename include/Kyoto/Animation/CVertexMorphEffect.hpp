#ifndef _CVERTEXMORPHEFFECT
#define _CVERTEXMORPHEFFECT
#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"

#include "Kyoto/CRandom16.hpp"
#endif

#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#if !RSTL_CORRUPTION
#include "Kyoto/TToken.hpp"
#endif

#include "rstl/vector.hpp"

#if RSTL_CORRUPTION


class CRandom16;
#else
class CSkinRules;
class CPoseAsTransforms;
#endif

class CVertexMorphEffect {
public:
#if RSTL_CORRUPTION
  ~CVertexMorphEffect() {}
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CVertexMorphEffect(const CUnitVector3f& dir, const CVector3f& pos, float duration,
                     float diagExtent, CRandom16& random)
  : mDir(dir)
  , mPos(pos)
  , mDuration(duration)
  , mElapsed(0.f)
  , mDiagExtent(diagExtent)
  , mRandom(random) {}
  void Update(float dt);

  void MorphVertices(CVector3f* workspace, const CVector3f* averageNormals,
                     const TLockedToken< CSkinRules >& skinRules, const CPoseAsTransforms& pose,
                     uint vertexCount) const;
  void Reset(const CUnitVector3f& dir, const CVector3f& pos, float duration);
#else
  ~CVertexMorphEffect() {}
#endif
#endif

private:
  CUnitVector3f mDir;
  CVector3f mPos;
  float mDuration;
  float mElapsed;
  float mDiagExtent;
  CRandom16& mRandom;
#if RSTL_CORRUPTION
  rstl::vector< uint > mIndices;
  rstl::vector< float > mFloats;
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  mutable rstl::vector< uint > mIndices;
  mutable rstl::vector< float > mFloats;
#else
  rstl::vector< uint > mIndices;
  rstl::vector< float > mFloats;
#endif
#endif
};

#if !RSTL_CORRUPTION
#if RSTL_PRIME1
CHECK_SIZEOF(CVertexMorphEffect, 0x48)
#endif
#endif

#endif // _CVERTEXMORPHEFFECT
