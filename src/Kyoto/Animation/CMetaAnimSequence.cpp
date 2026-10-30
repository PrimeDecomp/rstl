#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CMetaAnimSequence.hpp"
#include "Kyoto/Animation/CAnimTreeSequence.hpp"
#include "Kyoto/Animation/CMetaAnimFactory.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/COutputStream.hpp"

CMetaAnimSequence::CMetaAnimSequence(CInputStream& in)
: mSequence(CMetaAnimSequence::CreateSequence(in)) {}

rstl::ncrc_ptr< CAnimTreeNode >
CMetaAnimSequence::VGetAnimationTree(const CAnimSysContext& animSys,
                                     const CMetaAnimTreeBuildOrders& orders) const {
  if (orders.mRecursiveAdvance) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    return GetAnimationTree(
        animSys, CMetaAnimTreeBuildOrders::PreAdvanceForAll(*orders.mRecursiveAdvance));
#else
    return GetAnimationTree(animSys,
                            CMetaAnimTreeBuildOrders::PreAdvanceForAll(*orders.mRecursiveAdvance));
#endif

  }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  AUTO(it, mSequence.begin());
  AUTO(end, mSequence.end());
#else
  rstl::vector< rstl::rc_ptr< IMetaAnim > >::const_iterator it = mSequence.begin();
  rstl::vector< rstl::rc_ptr< IMetaAnim > >::const_iterator end = mSequence.end();
#endif

  rstl::vector< rstl::string > names;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  AUTO(len, end - it);
  names.reserve(len++);
#else
  names.reserve(end - it);
#endif

  while (it != end) {
    rstl::rc_ptr< IMetaAnim > anim = *it;
    rstl::ncrc_ptr< CAnimTreeNode > tree = anim->GetAnimationTree(animSys, orders);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    names.push_back(tree->GetPrimitiveName());
#else
    names.push_back_unsafe(tree->GetPrimitiveName());
#endif

    ++it;
  }
  return rs_new CAnimTreeSequence(mSequence, animSys,
                                  CAnimTreeSequence::CreatePrimitiveName(names));
}

void CMetaAnimSequence::GetUniquePrimitives(rstl::set< CPrimitive >& primsOut) const {
  rstl::vector< rstl::rc_ptr< IMetaAnim > >::const_iterator it = mSequence.begin();
  rstl::vector< rstl::rc_ptr< IMetaAnim > >::const_iterator end = mSequence.end();
  while (it != end) {
    it->GetPtr()->GetUniquePrimitives(primsOut);
    ++it;
  }
}

void CMetaAnimSequence::WriteAnimData(COutputStream& out) const {
  rstl::vector< rstl::rc_ptr< IMetaAnim > >::const_iterator it = mSequence.begin();
  rstl::vector< rstl::rc_ptr< IMetaAnim > >::const_iterator end = mSequence.end();
  out.WriteLong(mSequence.size());
  while (it != end) {
    it->GetPtr()->PutTo(out);
    ++it;
  }
}

rstl::vector< rstl::rc_ptr< IMetaAnim > > CMetaAnimSequence::CreateSequence(CInputStream& in) {
  rstl::vector< rstl::rc_ptr< IMetaAnim > > sequence;
  int size = in.Get< int >();
  sequence.reserve(size);
  for (int i = 0; i < size; i++) {
    rstl::rc_ptr< IMetaAnim > anim = CMetaAnimFactory::CreateMetaAnim(in);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    sequence.push_back(anim);
#else
    sequence.push_back_unsafe(anim);
#endif

  }
  return sequence;
}

