#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/RstlVersions.h"
#endif
#include "Kyoto/Animation/IMetaAnim.hpp"

#if !RSTL_CORRUPTION
#include "Kyoto/Animation/CAnimTreeNode.hpp"

#include "Kyoto/Animation/CBoolPOINode.hpp"
#endif
#include "Kyoto/Streams/COutputStream.hpp"
#if !RSTL_CORRUPTION
#if RSTL_PRIME1
#include "string.h"
#endif
#endif

#if !RSTL_CORRUPTION
#if RSTL_PRIME1
bool CPreAdvanceIndicator::IsTime() const { return mIsTime; }

const CCharAnimTime& CPreAdvanceIndicator::GetTime() const { return mTime; }

bool CPreAdvanceIndicator::IsString() const { return !IsTime(); }

const char* CPreAdvanceIndicator::GetString() const { return mString; }
#endif
#endif

CMetaAnimTreeBuildOrders CMetaAnimTreeBuildOrders::NoSpecialOrders() {
  return CMetaAnimTreeBuildOrders();
}
CMetaAnimTreeBuildOrders
CMetaAnimTreeBuildOrders::PreAdvanceForAll(const CPreAdvanceIndicator& ind) {
  CMetaAnimTreeBuildOrders ret;
  ret.mSingleAdvance = ind;
  return ret;
}

rstl::ncrc_ptr< CAnimTreeNode >
IMetaAnim::GetAnimationTree(const CAnimSysContext& animSys,
                            const CMetaAnimTreeBuildOrders& orders) const {
  if (orders.mSingleAdvance) {
    rstl::ncrc_ptr< CAnimTreeNode > tree =
        VGetAnimationTree(animSys, CMetaAnimTreeBuildOrders::NoSpecialOrders());
    if (orders.mSingleAdvance->IsTime() || orders.mSingleAdvance->IsString()) {
      AdvanceAnim(*tree, GetTime(*orders.mSingleAdvance, *tree));
    }
    return tree;
  }
  if (orders.mRecursiveAdvance) {
    rstl::ncrc_ptr< CAnimTreeNode > tree =
        VGetAnimationTree(animSys, CMetaAnimTreeBuildOrders::NoSpecialOrders());
    if (orders.mRecursiveAdvance->IsTime() || orders.mRecursiveAdvance->IsString()) {
      AdvanceAnim(*tree, GetTime(*orders.mRecursiveAdvance, *tree));
    }
    return tree;
  }
  return VGetAnimationTree(animSys, CMetaAnimTreeBuildOrders::NoSpecialOrders());
}

void IMetaAnim::PutTo(COutputStream& out) const {
  out.WriteInt32(GetType());
  WriteAnimData(out);
}

#if RSTL_CORRUPTION
CCharAnimTime IMetaAnim::GetTime(const CPreAdvanceIndicator& ind, const IAnimReader&) {
#else
CCharAnimTime IMetaAnim::GetTime(const CPreAdvanceIndicator& ind, const IAnimReader& anim) {
#endif
  if (ind.IsTime()) {
    return ind.GetTime();
  }

#if !RSTL_CORRUPTION
  CBoolPOINode nodes[64];
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const char* name = ind.GetString();
  const uint count = anim.GetBoolPOIList(anim.GetTimeRemaining(), nodes, 64, 0, 0);
#else
  const uint nameHash = ind.GetNameHash();
  const uint count = anim.GetBoolPOIList(anim.VGetTimeRemaining(), nodes, 64, 0, 0);
#endif
  for (uint i = 0; i < count; ++i) {
    const CBoolPOINode& node = nodes[i];
    if (
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
        strcmp(node.GetString().data(), name) == 0
#else
        node.GetNameHash() == nameHash
#endif
        && node.GetValue()) {
      return node.GetTime();
    }

  }
#endif
  return CCharAnimTime::ZeroFlat();
}

void IMetaAnim::AdvanceAnim(IAnimReader& anim, const CCharAnimTime& dt) {
  CCharAnimTime remaining = dt;
  while (remaining > CCharAnimTime::ZeroFlat()) {
#if RSTL_CORRUPTION
    SAdvancementResults result = anim.VAdvanceView(remaining);
    remaining = result.mRemTime;
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    SAdvancementResults result = anim.AdvanceView(remaining);
    remaining = result.GetRemainder();
#else
    SAdvancementResults result = anim.VAdvanceView(remaining);
    remaining = result.mRemTime;
#endif
#endif
  }
}
