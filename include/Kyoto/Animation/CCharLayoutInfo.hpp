#ifndef _CCHARLAYOUTINFO
#define _CCHARLAYOUTINFO
#include "rstl/RstlVersions.h"
#include "rstl/static_assert.hpp"
#if !RSTL_CORRUPTION
#include "rstl/RstlVersions.h"
#endif
#include "rstl/types.h"

#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Animation/CSegIdList.hpp"
#include "Kyoto/Animation/TSegIdMap.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/Math/CMatrix3f.hpp"
#endif
#include "Kyoto/Math/CQuaternion.hpp"

#include "Kyoto/Math/CVector3f.hpp"

#include "rstl/map.hpp"

#include "rstl/object_owner.hpp"
#if !RSTL_CORRUPTION
#include "rstl/auto_ptr.hpp"

#endif
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

class CCharLayoutNode {
public:
  explicit CCharLayoutNode(CInputStream& in);

  ~CCharLayoutNode() {}
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CSegId GetParent() const { return mParent; }
#else
  const CSegId& GetParent() const { return mParent; }
#endif

  const CVector3f& GetReferenceStanceOffset() const { return mReferenceStanceOffset; }
#if !RSTL_PRIME1
  const CQuaternion& GetRotation() const { return mRotation; }

  const CQuaternion& GetLocalRotation() const { return mLocalRotation; }
#endif

  int GetNumConnectedParts() const { return mConnectedParts.size(); }
  const rstl::vector< CSegId >& GetConnectedParts() const { return mConnectedParts; }

private:
  CSegId mParent;
  CVector3f mReferenceStanceOffset;
#if !RSTL_PRIME1
  CQuaternion mRotation;
  CQuaternion mLocalRotation;
#endif

  rstl::vector< CSegId > mConnectedParts;
};
#if RSTL_CORRUPTION
CHECK_SIZEOF(CCharLayoutNode, 0x40)
#else
CHECK_SIZEOF(CCharLayoutNode, RSTL_PRIME1 ? 0x20 : 0x40)
#endif

class CCharLayoutInfo {
public:
  explicit CCharLayoutInfo(CInputStream& in);

  CSegId GetSegIdFromString(const rstl::string& bone) const;
#if !RSTL_CORRUPTION
  const rstl::map< rstl::string, CSegId >& GetNameMap() const { return mNameMap; }
#endif
  const CCharLayoutNode& GetSegmentData(const CSegId& seg) const { return (*mNodes)[seg]; }
#if !RSTL_PRIME1

  int GetNumSegments() const { return mNodes->GetNumNodes(); }
#endif

  const CSegIdList& GetBodyPartSegIds() const { return mSegIdList; }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const CVector3f& GetReferenceStanceOffset(const CSegId& id) const {
    return id != CSegId::Root() ? GetSegmentData(id).GetReferenceStanceOffset() : CVector3f::Zero();
  }
#endif
  CSegId GetOriginalParent(const CSegId& seg) const { return GetSegmentData(seg).GetParent(); }
#if RSTL_CORRUPTION

  const rstl::vector< CSegId >& GetLinearParents() const { return mLinearParents; }

  const rstl::vector< CVector3f >& GetLinearReferenceStanceOffsets() const {
    return mLinearReferenceStanceOffsets;
  }
  const rstl::vector< CVector3f >& GetLinearParentOffsets() const { return mLinearParentOffsets; }

  // Guessed name
  const rstl::vector< CQuaternion >& GetLinearRotations() const { return mLinearRotations; }

  // Guessed names
  const rstl::vector< CMatrix3f >& GetLinearRotationMatrices() const {
    return mLinearRotationMatrices;
  }
  const rstl::map< rstl::string, CSegId >& GetNameMap() const { return mNameMap; }
#else
#if !RSTL_PRIME1

  const rstl::vector< CSegId >& GetLinearParents() const { return mLinearParents; }

  const rstl::vector< CVector3f >& GetLinearReferenceStanceOffsets() const {
    return mLinearReferenceStanceOffsets;
  }
  const rstl::vector< CVector3f >& GetLinearParentOffsets() const { return mLinearParentOffsets; }

  // Guessed name
  const rstl::vector< CQuaternion >& GetLinearRotations() const { return mLinearRotations; }
#endif
#endif

  CVector3f GetFromParentUnrotated(const CSegId& id) const {
    const CCharLayoutNode& node = GetSegmentData(id);
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
    const CSegId parent = node.GetParent();
#endif
    return !mNodes->ContainsDataFor(
#if RSTL_PRIME1
               node.GetParent()
#else
               parent
#endif
               )
               ? node.GetReferenceStanceOffset()
               : node.GetReferenceStanceOffset() -
                     GetSegmentData(node.GetParent()).GetReferenceStanceOffset();
  }

private:
  void InitializeLinearData(); // Guessed name.

  rstl::object_owner< TSegIdMap< CCharLayoutNode > > mNodes;
  CSegIdList mSegIdList;
  rstl::map< rstl::string, CSegId > mNameMap;
#if RSTL_CORRUPTION
  rstl::vector< CSegId > mLinearParents;
  rstl::vector< CVector3f > mLinearReferenceStanceOffsets;
  rstl::vector< CVector3f > mLinearParentOffsets;
  rstl::vector< CQuaternion > mLinearRotations;
  rstl::vector< CMatrix3f > mLinearRotationMatrices; // Guessed name
#else
#if !RSTL_PRIME1
  rstl::vector< CSegId > mLinearParents;
  rstl::vector< CVector3f > mLinearReferenceStanceOffsets;
  rstl::vector< CVector3f > mLinearParentOffsets;
  rstl::vector< CQuaternion > mLinearRotations;
  rstl::vector< CQuaternion > mLinearLocalRotations;
#endif
#endif
};
#if RSTL_CORRUPTION
CHECK_SIZEOF(CCharLayoutInfo, 0x7c)
#else
CHECK_SIZEOF(CCharLayoutInfo, RSTL_PRIME1 ? 0x2c : 0x7c)
#endif

#endif // _CCHARLAYOUTINFO
