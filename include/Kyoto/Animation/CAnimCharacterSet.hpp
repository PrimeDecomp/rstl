#ifndef _CANIMCHARACTERSET
#define _CANIMCHARACTERSET
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#include "Kyoto/Animation/CAnimationSet.hpp"
#else
#include "Kyoto/Animation/CCEAnimationSet.hpp"
#endif

#include "Kyoto/Animation/CCharacterSet.hpp"
#include "rstl/types.h"

struct SObjectTag;

class CFactoryFnReturn;
class CVParamTransfer;

class CAnimCharacterSet {
public:
  explicit CAnimCharacterSet(CInputStream& in);

  const CCharacterSet& GetCharacterSet() const { return mCharacterSet; }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const CAnimationSet& GetAnimationSet() const { return mAnimationSet; }
#else
  const CCEAnimationSet& GetAnimationSet() const { return mAnimationSet; }
#endif

private:
  ushort mVersion;
  CCharacterSet mCharacterSet;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CAnimationSet mAnimationSet;
#else
  CCEAnimationSet mAnimationSet;
#endif
};
CHECK_SIZEOF(CAnimCharacterSet, RSTL_PRIME1 ? 0x78 : 0x7c)

RSTL_PRIME1_CONST CFactoryFnReturn FAnimCharacterSet(const SObjectTag& tag, CInputStream& in,
                                                     const CVParamTransfer& xfer);

#endif // _CANIMCHARACTERSET
