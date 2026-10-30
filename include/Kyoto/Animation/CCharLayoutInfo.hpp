#ifndef _CCHARLAYOUTINFO
#define _CCHARLAYOUTINFO
#include "rstl/RstlVersions.h"
#if !RSTL_PRIME1
#include "types.h"
#endif

#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Animation/CSegIdList.hpp"
#include "Kyoto/Animation/TSegIdMap.hpp"
#if !RSTL_PRIME1
#include "Kyoto/Math/CQuaternion.hpp"
#endif

#include "Kyoto/Math/CVector3f.hpp"

#if !RSTL_PRIME1
#include "rstl/map.hpp"
#endif

#include "rstl/object_owner.hpp"
#if RSTL_PRIME1
#include "rstl/auto_ptr.hpp"
#include "rstl/map.hpp"
#endif

#include "rstl/string.hpp"
#if !RSTL_PRIME1
#include "rstl/vector.hpp"
#endif

class CCharLayoutNode {
public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CCharLayoutNode(CInputStream& in);
#else
  explicit CCharLayoutNode(CInputStream& in);
#endif

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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION < RSTL_R3IJ
CHECK_SIZEOF(CCharLayoutNode, 0x20)
#endif
#else
CHECK_SIZEOF(CCharLayoutNode, 0x40)
#endif

class CCharLayoutInfo {
public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CCharLayoutInfo(CInputStream& in);
#else
  explicit CCharLayoutInfo(CInputStream& in);
#endif

  CSegId GetSegIdFromString(const rstl::string& bone) const;
#if !RSTL_PRIME1
  const rstl::map< rstl::string, CSegId >& GetNameMap() const { return mNameMap; }

  const CCharLayoutNode& GetSegmentData(const CSegId& seg) const { return (*mNodes)[seg]; }

  int GetNumSegments() const { return mNodes->GetNumNodes(); }
#endif

  const CSegIdList& GetBodyPartSegIds() const { return mSegIdList; }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const rstl::map< rstl::string, CSegId >& GetNameMap() const { return mNameMap; }
  const CVector3f& GetReferenceStanceOffset(const CSegId& id) const {
    return id != CSegId::Root() ? GetSegmentData(id).GetReferenceStanceOffset() : CVector3f::Zero();
#else
  CSegId GetOriginalParent(const CSegId& seg) const { return GetSegmentData(seg).GetParent(); }

  const rstl::vector< CSegId >& GetLinearParents() const { return mLinearParents; }

  const rstl::vector< CVector3f >& GetLinearReferenceStanceOffsets() const {
    return mLinearReferenceStanceOffsets;
#endif

  }
#if !RSTL_PRIME1
  const rstl::vector< CVector3f >& GetLinearParentOffsets() const { return mLinearParentOffsets; }

  // Guessed name
  const rstl::vector< CQuaternion >& GetLinearRotations() const { return mLinearRotations; }
#endif

  CVector3f GetFromParentUnrotated(const CSegId& id) const {
    const CCharLayoutNode& node = GetSegmentData(id);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    return !mNodes->ContainsDataFor(node.GetParent())
#else
    const CSegId parent = node.GetParent();
    return !mNodes->ContainsDataFor(parent)
#endif

               ? node.GetReferenceStanceOffset()
               : node.GetReferenceStanceOffset() -
                     GetSegmentData(node.GetParent()).GetReferenceStanceOffset();
  }
#if RSTL_PRIME1
  const CCharLayoutNode& GetSegmentData(const CSegId& seg) const { return (*mNodes)[seg]; }
  CSegId GetOriginalParent(const CSegId& seg) const { return GetSegmentData(seg).GetParent(); }
#endif

private:
#if !RSTL_PRIME1
  void InitializeLinearData(); // Guessed name.
#endif

  rstl::object_owner< TSegIdMap< CCharLayoutNode > > mNodes;
  CSegIdList mSegIdList;
  rstl::map< rstl::string, CSegId > mNameMap;
#if !RSTL_PRIME1
  rstl::vector< CSegId > mLinearParents;
  rstl::vector< CVector3f > mLinearReferenceStanceOffsets;
  rstl::vector< CVector3f > mLinearParentOffsets;
  rstl::vector< CQuaternion > mLinearRotations;
  rstl::vector< CQuaternion > mLinearLocalRotations;
#endif

};
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION < RSTL_R3IJ
CHECK_SIZEOF(CCharLayoutInfo, 0x2c)
#endif
#else
CHECK_SIZEOF(CCharLayoutInfo, 0x7c)
#endif

#endif // _CCHARLAYOUTINFO
