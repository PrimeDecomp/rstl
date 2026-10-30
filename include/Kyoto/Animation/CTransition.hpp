#ifndef _CTRANSITION
#define _CTRANSITION
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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  uint GetFromAnimIndex() const { return mAnimA; }
  uint GetToAnimIndex() const { return mAnimB; }
#else
  uint GetFromAnimIndex() const { return mAnimPair.first; }
  uint GetToAnimIndex() const { return mAnimPair.second; }
  uint GetAnimA() const { return mAnimPair.first; }
  uint GetAnimB() const { return mAnimPair.second; }
  rstl::pair< uint, uint > GetAnimPair() const { return mAnimPair; }
#endif

  const rstl::rc_ptr< IMetaTrans >& GetMetaTrans() const { return mTrans; }
};
CHECK_SIZEOF(CTransition, RSTL_PRIME1 ? 0x10 : 0x14)

#endif // _CTRANSITION
