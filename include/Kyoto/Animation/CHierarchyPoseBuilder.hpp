#ifndef _CHIERARCHYPOSEBUILDER
#define _CHIERARCHYPOSEBUILDER

#include "Kyoto/Animation/CLayoutDescription.hpp"
#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Animation/TSegIdMap.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "rstl/RstlVersions.h"
#include "rstl/auto_ptr.hpp"
#include "rstl/construction_deferred.hpp"
#include "rstl/static_assert.hpp"

class CCharLayoutInfo;
class CPoseAsTransforms;
class CTransform4f;

class CHierarchyPoseBuilder {
public:
  typedef TSegIdMap< CVector3f > ScaleMap;

  // Prime 1
  CHierarchyPoseBuilder(const CLayoutDescription& layout);

  // Prime 2
  CHierarchyPoseBuilder(const CLayoutDescription& layout, bool animatedScale);

  // Prime 1 only
  void BuildNoScale(CPoseAsTransforms& pose);
  void BuildTransform(const CSegId& seg, CTransform4f& transform) const;

  class CTreeNode {
  public:
    CTreeNode(const CSegId& sibling, const CSegId& child, const CVector3f& offset)
    : mChild(child), mSibling(sibling), mRotation(CQuaternion::NoRotation()), mOffset(offset) {}

    CSegId GetFirstChildSegment() const { return mChild; }
    CSegId GetNextSiblingSegment() const { return mSibling; }
    const CQuaternion& GetRotation() const { return mRotation; }
    const CVector3f& GetOffset() const { return mOffset; }

    void SetRotation(const CQuaternion& rot) { mRotation = CQuaternion(rot); }
    void SetOffset(const CVector3f& offset) { mOffset = offset; }

    CTreeNode NodeForNextChildInserted(const CSegId& child, const CSegId& nullId,
                                       const CVector3f& offset) {
      CSegId sibling = mChild;
      mChild = child;
      return CTreeNode(sibling, nullId, offset);
    }

  private:
    CSegId mChild;
    CSegId mSibling;
    CQuaternion mRotation;
    CVector3f mOffset;
  };

  void Insert(const CSegId& id, const CQuaternion& rot) { 
    // FIXME: this function needs it's inline weight increased by at +28 to cause it to be outlined
    mTreeMap[id].SetRotation(rot); 
  }
  void Insert(const CSegId& id, const CVector3f& offset) { mTreeMap[id].SetOffset(offset); }
  CQuaternion GetSegRotation(const CSegId& id) const { return mTreeMap[id].GetRotation(); }
  const TLockedToken< CCharLayoutInfo >& CharLayoutInfo() const {
    return mLayoutDesc.ScaledLayout();
  }

private:
  void BuildIntoHeirarchy(const CCharLayoutInfo& layout, const CSegId& seg, const CSegId& root);
  void RecursivelyBuildNoScale(const CSegId& seg, const CTreeNode& node, CPoseAsTransforms& pose,
                               const CQuaternion& rotation, const CMatrix3f& matrix,
                               const CVector3f& offset) const;
  void RecursivelyBuild(const CSegId& seg, const CTreeNode& node, CPoseAsTransforms& pose,
                        const CQuaternion& rotation, const CMatrix3f& matrix,
                        const CVector3f& offset) const;

  CLayoutDescription mLayoutDesc;
  rstl::construction_deferred< CSegId > mRootId;
  TSegIdMap< CTreeNode > mTreeMap;
#if !RSTL_PRIME1
  rstl::auto_ptr< ScaleMap > mScales;
#endif
};
CHECK_SIZEOF(CHierarchyPoseBuilder, RSTL_PRIME1 ? 0x110 : 0x118)
NESTED_CHECK_SIZEOF(CHierarchyPoseBuilder, CTreeNode, 0x20)

#endif // _CHIERARCHYPOSEBUILDER
