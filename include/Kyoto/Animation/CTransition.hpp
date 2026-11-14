#ifndef _CTRANSITION
#define _CTRANSITION
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"

#include "Kyoto/Animation/IMetaTrans.hpp"
#include "rstl/pair.hpp"

#include "rstl/rc_ptr.hpp"

class CInputStream;

class CTransition {
  uint mId;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  uint mAnimA;
  uint mAnimB;
#else
  rstl::pair< uint, uint > mAnimPair;
#endif

  rstl::rc_ptr< IMetaTrans > mTrans;

public:
  explicit CTransition(CInputStream& in);
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

  const rstl::rc_ptr< IMetaTrans >& GetMetaTrans() const { return mTrans; }
};
CHECK_SIZEOF(CTransition, RSTL_PRIME1 ? 0x10 : 0x14)

#endif // _CTRANSITION
