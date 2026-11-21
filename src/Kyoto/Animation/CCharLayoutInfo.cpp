#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/RstlVersions.h"
#include "rstl/macros.hpp"
#endif
#include "Kyoto/Animation/CCharLayoutInfo.hpp"

#if !RSTL_CORRUPTION
#include "Kyoto/CFactoryFnReturn.hpp"
#endif
#include "Kyoto/CFactoryMgr.hpp"

#include "Kyoto/Streams/CInputStream.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/Streams/CMemoryInStream.hpp"
#endif
#include "rstl/StringExtras.hpp"

#if RSTL_CORRUPTION


CFactoryFnReturn FCharLayoutInfo(const SObjectTag& tag, const rstl::auto_ptr< uchar >& buffer,
                                 int size, const CVParamTransfer&) {
  CMemoryInStream in(buffer.get(), size);
  return rs_new_line(25) CCharLayoutInfo(in);
#else
RSTL_PRIME1_CONST CFactoryFnReturn FCharLayoutInfo(const SObjectTag& tag, CInputStream& in,
                                                   const CVParamTransfer&) {
  return rs_new CCharLayoutInfo(in);
#endif
}

CCharLayoutNode::CCharLayoutNode(CInputStream& in)
: mParent(in)
, mReferenceStanceOffset(in)
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
, mRotation(in)
, mLocalRotation(in)
#endif
, mConnectedParts(in) {}

CCharLayoutInfo::CCharLayoutInfo(CInputStream& in)
: mNodes(rstl::ownership_transfer< TSegIdMap< CCharLayoutNode > >(
      rs_new_line(72) TSegIdMap< CCharLayoutNode >(in)))
, mSegIdList(in)
, mNameMap(in) {
#if RSTL_CORRUPTION
  InitializeLinearData();
}

void CCharLayoutInfo::InitializeLinearData() {
  const int count = mSegIdList.size();
  mLinearParents.reserve(count);
  mLinearReferenceStanceOffsets.reserve(count);
  mLinearParentOffsets.reserve(count);
  mLinearRotations.reserve(count);
  mLinearRotationMatrices.reserve(count);

  for (int i = 0; i < count; ++i) {
    CSegId id(i);
    const CCharLayoutNode& node = GetSegmentData(id);
    mLinearParents.push_back_unsafe(node.GetParent());
    mLinearReferenceStanceOffsets.push_back_unsafe(node.GetReferenceStanceOffset());
    mLinearParentOffsets.push_back_unsafe(GetFromParentUnrotated(id));
    mLinearRotations.push_back_unsafe(node.GetRotation());
    mLinearRotationMatrices.push_back_unsafe(mLinearRotations[i].BuildTransform());
  }
}
#else
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
  InitializeLinearData();
#endif
}

#if !RSTL_PRIME1
// Stripped by the linker; they keep the original template emission order.
static void DeleteSegIdList(CSegIdList* list) { delete list; }

static void CopyNameMap(const rstl::map< rstl::string, CSegId >& names) {
  rstl::map< rstl::string, CSegId > copy(names);
}

void CCharLayoutInfo::InitializeLinearData() {
  const int count = mSegIdList.size();
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
#endif

CSegId CCharLayoutInfo::GetSegIdFromString(const rstl::string& bone) const {
#if RSTL_CORRUPTION
  for (rstl::map< rstl::string, CSegId >::const_iterator it = mNameMap.begin();
       it != mNameMap.end(); ++it) {
    if (CStringExtras::CompareCaseInsensitive(it->first, bone) == 0) {
      return it->second;
    }
  }
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  AUTO(it, mNameMap.find(bone));
  if (it != mNameMap.end()) {
    return it->second;
  }
#else
  rstl::map< rstl::string, CSegId >::const_iterator it = mNameMap.begin();
  const rstl::map< rstl::string, CSegId >::const_iterator end = mNameMap.end();
  for (; it != end; ++it) {
    if (CStringExtras::CompareCaseInsensitive(it->first, bone) == 0) {
      return it->second;
    }
  }
#endif
#endif
  return CSegId::Invalid();
}
#if !RSTL_CORRUPTION

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
// Stripped by the linker; preserves template helper emission order.
static void UnusedLayoutOperation(CSegIdList* list) {
  TSegIdMap< CCharLayoutNode > map(uchar(0));
  delete list;
}
#else
// Stripped by the linker; they keep the original template emission order.
static void InsertLayoutNode(const CCharLayoutNode& node) {
  TSegIdMap< CCharLayoutNode > map(uchar(1));
  map.insert(CSegId(0), node);
}

static void CopyConnectedParts(const CCharLayoutNode& node) {
  rstl::vector< CSegId > parts(node.GetConnectedParts());
}

static void DeleteLayoutNode(CCharLayoutNode* node) { delete node; }
#endif
#endif
