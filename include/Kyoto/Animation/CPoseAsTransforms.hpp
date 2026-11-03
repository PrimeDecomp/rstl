#ifndef _CPOSEASTRANSFORMS
#define _CPOSEASTRANSFORMS

#include "types.h"

#include "Kyoto/Animation/TSegIdMapVariableSize.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Math/CTransform4f.hpp"

#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/RstlVersions.h"

class CPoseAsTransforms {
public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  class CElementType {
  public:
    CElementType(const CMatrix3f& rotation, const CVector3f& offset)
    : mRotation(rotation), mOffset(offset) {}

    const CMatrix3f& GetRotation() const { return mRotation; }
    const CVector3f& GetOffset() const { return mOffset; }

  private:
    CMatrix3f mRotation;
    CVector3f mOffset;
  };

  CPoseAsTransforms(uchar count);

  void Insert(const CSegId& seg, const CMatrix3f& rotation, const CVector3f& offset);
#endif
  const CMatrix3f& GetRotation(const CSegId& id) const;
#if RSTL_PRIME1
  const CMatrix3f& GetTransformMinusOffset(const CSegId& id) const;
  const CVector3f& GetOffset(const CSegId& id) const;
  void Clear();

  void AccumulateScaledTransform(const CSegId& seg, CMatrix3f& transform, float scale) const;
  bool ContainsDataFor(const CSegId& seg) const;
  const TSegIdMapVariableSize< CElementType >& GetTransforms() const { return mSegIdMap; }
#endif

private:
#if RSTL_PRIME1
  TSegIdMapVariableSize< CElementType > mSegIdMap;
#else
  CSegId mNextId;
  CSegId mCount;
  // TODO TSegIdMapVariableSize<CPoseAsTransforms::CElementType>
  rstl::reserved_vector< rstl::pair< CSegId, CSegId >, 100 > mLinks;
  rstl::single_ptr< CTransform4f > mTransformArr;
  CSegId mLastInserted;
#endif
};
CHECK_SIZEOF(CPoseAsTransforms, 0xd8)

#endif // _CPOSEASTRANSFORMS
