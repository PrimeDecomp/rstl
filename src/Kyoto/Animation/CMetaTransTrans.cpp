#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CMetaTransTrans.hpp"
#include "Kyoto/Animation/CAnimTreeTransition.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/COutputStream.hpp"

CMetaTransTrans::CMetaTransTrans(CInputStream& in)
: mTransDur(CCharAnimTime(in))
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
, xc_(in.ReadBool())
#else
, mCharacterSpaceBlend(in.ReadBool())
#endif
, mRunA(in.ReadBool())
, mFlags(
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      in.ReadLong()
#else
      in.ReadInt32()
#endif
      ) {}

rstl::ncrc_ptr< CAnimTreeNode >
CMetaTransTrans::VGetTransitionTree(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                    const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                    const CAnimSysContext& animSys) const {
  return rs_new CAnimTreeTransition(
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      xc_,
#else
      mCharacterSpaceBlend,
#endif
      a, b, mTransDur, mRunA, mFlags,
      CAnimTreeTransition::CreatePrimitiveName(a, b, mTransDur.GetSeconds()));
}

void CMetaTransTrans::WriteTransData(COutputStream& out) const {
  mTransDur.PutTo(out);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  out.WriteChar(bool(xc_));
#else
  out.WriteChar(bool(mCharacterSpaceBlend));
#endif
  out.WriteChar(bool(mRunA));
  out.WriteLong(mFlags);
}

