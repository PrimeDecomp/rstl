#ifndef _CHALFTRANSITION
#define _CHALFTRANSITION
#include "rstl/RstlVersions.h"
#include "rstl/static_assert.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/CAssetId.hpp"
#else
#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/IMetaTrans.hpp"
#endif

#include "rstl/rc_ptr.hpp"

#if RSTL_CORRUPTION


class IMetaTrans;
#endif
class CInputStream;

class CHalfTransition {
public:
  explicit CHalfTransition(CInputStream& in);
#if !RSTL_CORRUPTION
#if !RSTL_PRIME1
  ~CHalfTransition() {}
#endif
#endif

#if RSTL_CORRUPTION
  // Emitted out of line in CHalfTransition.cpp.
  const rstl::rc_ptr< IMetaTrans >& GetMetaTrans() const; // 0x8055DE78, Prime's name
  const CAssetId& GetToAnimId() const;                    // 0x8055DE80, Guessed name
#else
  uint GetPrimitiveIndex() const { return mId; }
  const rstl::rc_ptr< IMetaTrans >& GetMetaTrans() const { return mTrans; }
#endif

private:
#if RSTL_CORRUPTION
  uchar mVersion; // Guessed name: the first byte of the stream record
  CAssetId mToAnim;
  rstl::rc_ptr< IMetaTrans > mMetaTrans;
#else
  uint mId;
  rstl::rc_ptr< IMetaTrans > mTrans;
#endif
};
#if RSTL_CORRUPTION
CHECK_SIZEOF(CHalfTransition, 0x18)
#else
CHECK_SIZEOF(CHalfTransition, RSTL_PRIME1 ? 0x8 : 0xc)
#endif

#endif // _CHALFTRANSITION
