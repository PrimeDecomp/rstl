#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CHierarchyPoseBuilder.hpp"

#include "Kyoto/Animation/CCharLayoutInfo.hpp"
#if RSTL_PRIME1
#include "Kyoto/Animation/CPoseAsTransforms.hpp"
#include "rstl/reserved_vector.hpp"
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CHierarchyPoseBuilder::CHierarchyPoseBuilder(const CLayoutDescription& layout)
: mLayoutDesc(layout), mTreeMap(layout.GetNumSegments()) {
#else
CHierarchyPoseBuilder::CHierarchyPoseBuilder(const CLayoutDescription& layout, bool animatedScale)
: mLayoutDesc(layout)
, mTreeMap(layout.GetNumSegments())
, mScales(animatedScale ? rstl::auto_ptr< ScaleMap >(rs_new ScaleMap(layout.GetNumSegments()))
                        : rstl::auto_ptr< ScaleMap >()) {
#endif

  TToken< CCharLayoutInfo > layoutToken = layout.ScaledLayout();
  const CCharLayoutInfo& layoutInfo = **layoutToken;
  const CSegIdList& segments = layoutInfo.GetBodyPartSegIds();
  for (CSegIdList::const_iterator it = segments.begin(), end = segments.end(); it != end; ++it) {
    CSegId seg = *it;
    BuildIntoHeirarchy(layoutInfo, seg, CSegId::Character());
#if !RSTL_PRIME1
    if (mScales.get()) {
      mScales->insert(seg, CVector3f::One());
    }
#endif

  }
}

void CHierarchyPoseBuilder::BuildIntoHeirarchy(const CCharLayoutInfo& layout, const CSegId& seg,
                                               const CSegId& root) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const CSegId id = seg;
  if (!mTreeMap.ContainsDataFor(id)) {
#else
  if (!mTreeMap.ContainsDataFor(seg)) {
#endif

    CSegId parent = layout.GetOriginalParent(seg);
    if (parent == root) {
      mRootId.build(seg);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      mTreeMap.insert(seg, CTreeNode(CSegId::Null(), CSegId::Null(),
                                     layout.GetFromParentUnrotated(seg)));
#else
      mTreeMap.insert(
          seg, CTreeNode(CSegId::Null(), CSegId::Null(), layout.GetFromParentUnrotated(seg)));
#endif

    } else {
      BuildIntoHeirarchy(layout, parent, root);
      mTreeMap.insert(seg, mTreeMap[parent].NodeForNextChildInserted(
                                  seg, CSegId::Null(), layout.GetFromParentUnrotated(seg)));
    }
  }
}

#if RSTL_PRIME1
void CHierarchyPoseBuilder::BuildNoScale(CPoseAsTransforms& pose) {
  CSegId root = *mRootId;
  pose.Clear();
  RecursivelyBuildNoScale(root, mTreeMap[root], pose, CQuaternion::NoRotation(),
                          CMatrix3f::Identity(), CVector3f::Zero());
}

void CHierarchyPoseBuilder::RecursivelyBuild(const CSegId& seg, const CTreeNode& node,
                                             CPoseAsTransforms& pose, const CQuaternion& rotation,
                                             const CMatrix3f& matrix,
                                             const CVector3f& offset) const {
  const CQuaternion& nodeRotation = node.GetRotation();
  CQuaternion childRotation = rotation * nodeRotation;
  float scale = mLayoutDesc.GlobalScale();
  CMatrix3f childMatrix = scale == 1.f
                              ? childRotation.BuildTransform()
                              : matrix * (nodeRotation.BuildTransform() * CMatrix3f::Scale(scale));
  CVector3f childOffset = offset + matrix * node.GetOffset();
  pose.Insert(seg, childMatrix, childOffset);

  CSegId child = node.GetFirstChildSegment();
  while (child != CSegId::Null()) {
    const CSegId childId = child;
    const CTreeNode& childNode = mTreeMap[childId];
    RecursivelyBuild(childId, childNode, pose, childRotation, childRotation.BuildTransform(),
                     childOffset);
    child = childNode.GetNextSiblingSegment();
  }
}

void CHierarchyPoseBuilder::RecursivelyBuildNoScale(const CSegId& seg, const CTreeNode& node,
                                                    CPoseAsTransforms& pose,
                                                    const CQuaternion& rotation,
                                                    const CMatrix3f& matrix,
                                                    const CVector3f& offset) const {
  CQuaternion childRotation = rotation * node.GetRotation();
  CMatrix3f childMatrix = childRotation.BuildTransform();
  CVector3f childOffset = offset + matrix * node.GetOffset();
  pose.Insert(seg, childMatrix, childOffset);

  CSegId child = node.GetFirstChildSegment();
  while (child != CSegId::Null()) {
    const CSegId childId = child;
    const CTreeNode& childNode = mTreeMap[childId];
    RecursivelyBuild(childId, childNode, pose, childRotation, childMatrix, childOffset);
    child = childNode.GetNextSiblingSegment();
  }
}

void CHierarchyPoseBuilder::BuildTransform(const CSegId& seg, CTransform4f& transform) const {
  rstl::reserved_vector< CSegId, 100 > segments;
  CSegId current = seg;
  const CCharLayoutInfo& layout = **CharLayoutInfo();
  float scale = mLayoutDesc.GlobalScale();
  while (current != CSegId::Character()) {
    segments.push_back(current);
    current = layout.GetOriginalParent(current);
  }

  CQuaternion rotation = CQuaternion::NoRotation();
  CVector3f offset(0.f, 0.f, 0.f);
  CMatrix3f matrix = CMatrix3f::Identity();
  for (AUTO(it, segments.end()); it != segments.begin(); --it) {
    const CTreeNode& node = mTreeMap[*(it - 1)];
    const CQuaternion& nodeRotation = node.GetRotation();
    rotation = rotation * nodeRotation;
    offset += matrix * node.GetOffset();
    matrix = scale == 1.f ? rotation.BuildTransform()
                          : matrix * (nodeRotation.BuildTransform() * CMatrix3f::Scale(scale));
  }
  transform.SetRotation(matrix);
  transform.SetTranslation(offset);
}
#endif

uchar CLayoutDescription::GetNumSegments() const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return mLayoutToken->GetBodyPartSegIds().size();
#else
  return mLayoutToken->GetBodyPartSegIds().Size();
#endif

}

