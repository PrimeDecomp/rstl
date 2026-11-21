#ifndef _CTRANSITION
#define _CTRANSITION
#include "rstl/RstlVersions.h"
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"

#if RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#include "Kyoto/CAssetId.hpp"
#else
#include "Kyoto/Animation/IMetaTrans.hpp"
#include "rstl/pair.hpp"
#endif

#include "rstl/rc_ptr.hpp"

#if RSTL_CORRUPTION


class IMetaTrans;
#endif
class CInputStream;

#if !RSTL_CORRUPTION
class CTransition {
  uint mId;
#endif
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_CORRUPTION
class CTransition {
#else
#if RSTL_PRIME1
  uint mAnimA;
  uint mAnimB;
#else
  rstl::pair< uint, uint > mAnimPair;
#endif

  rstl::rc_ptr< IMetaTrans > mTrans;

#endif
public:
  explicit CTransition(CInputStream& in);
#if !RSTL_CORRUPTION
  uint GetFromAnimIndex() const {
#if RSTL_PRIME1
    return mAnimA;
#else
    return mAnimPair.first;
#endif
  }
  uint GetToAnimIndex() const {
#if RSTL_PRIME1
    return mAnimB;
#else
    return mAnimPair.second;
#endif
  }
#if !RSTL_PRIME1
  uint GetAnimA() const { return mAnimPair.first; }
  uint GetAnimB() const { return mAnimPair.second; }
  rstl::pair< uint, uint > GetAnimPair() const { return mAnimPair; }
#endif
#endif

#if !RSTL_CORRUPTION
  const rstl::rc_ptr< IMetaTrans >& GetMetaTrans() const { return mTrans; }
};
CHECK_SIZEOF(CTransition, RSTL_PRIME1 ? 0x10 : 0x14)
#endif
  // Guessed names. Emitted out of line in CTransition.cpp.
#if RSTL_CORRUPTION
  const CAssetId& GetToAnimId() const;   // 0x805608C0
  const CAssetId& GetFromAnimId() const; // 0x805608C8
  const rstl::rc_ptr< IMetaTrans >& GetMetaTrans() const; // 0x805608D0, Prime's name

private:
  uchar mVersion; // Guessed name: the first byte of the stream record
  CAssetId mFromAnim;
  CAssetId mToAnim;
  rstl::rc_ptr< IMetaTrans > mMetaTrans;
};
CHECK_SIZEOF(CTransition, 0x20)


#endif
#endif // _CTRANSITION
