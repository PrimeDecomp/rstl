#ifndef _CHALFTRANSITION
#define _CHALFTRANSITION
#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/IMetaTrans.hpp"

#include "rstl/rc_ptr.hpp"

class CInputStream;

class CHalfTransition {
public:
  explicit CHalfTransition(CInputStream& in);
#if !RSTL_PRIME1
  ~CHalfTransition() {}
#endif

  uint GetPrimitiveIndex() const { return mId; }
  const rstl::rc_ptr< IMetaTrans >& GetMetaTrans() const { return mTrans; }

private:
  uint mId;
  rstl::rc_ptr< IMetaTrans > mTrans;
};
CHECK_SIZEOF(CHalfTransition, RSTL_PRIME1 ? 0x8 : 0xc)

#endif // _CHALFTRANSITION
