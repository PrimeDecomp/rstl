#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CCharLayoutInfo.hpp"

#include "Kyoto/CFactoryFnReturn.hpp"
#include "Kyoto/CFactoryMgr.hpp"

#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/StringExtras.hpp"

// TODO: verify that this Prime/Echoes difference is real
RSTL_PRIME1_CONST CFactoryFnReturn FCharLayoutInfo(const SObjectTag& tag, CInputStream& in, const CVParamTransfer&) {

  return rs_new CCharLayoutInfo(in);
}

CCharLayoutNode::CCharLayoutNode(CInputStream& in)
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
: mParent(in), mReferenceStanceOffset(in), mConnectedParts(in) {}
#else
: mParent(in), mReferenceStanceOffset(in), mRotation(in), mLocalRotation(in), mConnectedParts(in) {}
#endif

CCharLayoutInfo::CCharLayoutInfo(CInputStream& in)
: mNodes(rstl::ownership_transfer< TSegIdMap< CCharLayoutNode > >(
      rs_new TSegIdMap< CCharLayoutNode >(in)))
, mSegIdList(in)
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
, mNameMap(in) {}
#else
, mNameMap(in) {
  InitializeLinearData();
}

// Stripped by the linker; they keep the original template emission order.
static void DeleteSegIdList(CSegIdList* list) { delete list; }

static void CopyNameMap(const rstl::map< rstl::string, CSegId >& names) {
  rstl::map< rstl::string, CSegId > copy(names);
}

void CCharLayoutInfo::InitializeLinearData() {
  const int count = mSegIdList.Size();
  mLinearParents.reserve(count);
  mLinearReferenceStanceOffsets.reserve(count);
  mLinearParentOffsets.reserve(count);
  mLinearRotations.reserve(count);
  mLinearLocalRotations.reserve(count);

  for (int i = 0; i < count; ++i) {
    CSegId id(i);
    const CCharLayoutNode& node = GetSegmentData(id);
    mLinearParents.push_back_unsafe(node.GetParent());
    mLinearReferenceStanceOffsets.push_back_unsafe(node.GetReferenceStanceOffset());
    mLinearParentOffsets.push_back_unsafe(GetFromParentUnrotated(id));
    mLinearRotations.push_back_unsafe(node.GetRotation());
    mLinearLocalRotations.push_back_unsafe(node.GetLocalRotation());
  }
}
#endif

CSegId CCharLayoutInfo::GetSegIdFromString(const rstl::string& bone) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  AUTO(it, mNameMap.find(bone));
  if (it != mNameMap.end()) {
#else
  for (rstl::map< rstl::string, CSegId >::const_iterator it = mNameMap.begin();
       it != mNameMap.end(); ++it) {
    if (CStringExtras::CompareCaseInsensitive(it->first, bone) == 0) {
#endif

    return it->second;
#if !RSTL_PRIME1
    }
#endif

  }
  return CSegId::Invalid();
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
// Stripped by the linker; preserves template helper emission order.
static void UnusedLayoutOperation(CSegIdList* list) {
  TSegIdMap< CCharLayoutNode > map(uchar(0));
  delete list;
#else
// Stripped by the linker; they keep the original template emission order.
static void InsertLayoutNode(const CCharLayoutNode& node) {
  TSegIdMap< CCharLayoutNode > map(uchar(1));
  map.insert(CSegId(0), node);
#endif

}

#if !RSTL_PRIME1
static void CopyConnectedParts(const CCharLayoutNode& node) {
  rstl::vector< CSegId > parts(node.GetConnectedParts());
}
#endif

#if !RSTL_PRIME1
static void DeleteLayoutNode(CCharLayoutNode* node) { delete node; }
#endif
