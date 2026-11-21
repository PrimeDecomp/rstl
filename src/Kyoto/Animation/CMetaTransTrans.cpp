#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/RstlVersions.h"
#endif
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Animation/CMetaTransTrans.hpp"
#include "Kyoto/Animation/CAnimTreeTransition.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/COutputStream.hpp"

#if !RSTL_CORRUPTION
CMetaTransTrans::CMetaTransTrans(CInputStream& in)
: mTransDur(CCharAnimTime(in))
#endif
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_CORRUPTION

CMetaTransTrans::CMetaTransTrans(CInputStream& in, int type)
: mTransDur(CCharAnimTime(in))
, mCharacterSpaceBlend(in.ReadBool())
#else
#if RSTL_PRIME1
, xc_(in.ReadBool())
#else
, mCharacterSpaceBlend(in.ReadBool())
#endif
#endif
, mRunA(in.ReadBool())
#if RSTL_CORRUPTION
, mFlags(in.ReadInt32())
, mBlendCurve(type == kMTT_Trans ? in.ReadInt8() : 0) {}
#else
, mFlags(
      in.ReadLong()
      ) {}
#endif

rstl::ncrc_ptr< CAnimTreeNode >
CMetaTransTrans::VGetTransitionTree(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                    const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                    const CAnimSysContext& animSys) const {
#if RSTL_CORRUPTION
  return rs_new_line(54)
      CAnimTreeTransition(mCharacterSpaceBlend, a, b, mTransDur, mBlendCurve, mRunA, mFlags,
#else
  return rs_new CAnimTreeTransition(
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      xc_,
#else
      mCharacterSpaceBlend,
#endif
      a, b, mTransDur, mRunA, mFlags,
#endif
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
#if RSTL_CORRUPTION
  out.WriteChar(mBlendCurve);
#endif
}
