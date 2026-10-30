#ifndef _CANIMCHARACTERSET
#define _CANIMCHARACTERSET
#include "rstl/RstlVersions.h"

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#include "Kyoto/Animation/CAnimationSet.hpp"
#else
#include "Kyoto/Animation/CCEAnimationSet.hpp"
#endif

#include "Kyoto/Animation/CCharacterSet.hpp"
#include "types.h"

#if !RSTL_PRIME1
struct SObjectTag;

class CFactoryFnReturn;
class CVParamTransfer;
#endif

class CAnimCharacterSet {
public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CAnimCharacterSet(CInputStream& in);
#else
  explicit CAnimCharacterSet(CInputStream& in);
#endif

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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CHECK_SIZEOF(CAnimCharacterSet, 0x78)
#else
CHECK_SIZEOF(CAnimCharacterSet, 0x7c)

CFactoryFnReturn FAnimCharacterSet(const SObjectTag& tag, CInputStream& in,
                                   const CVParamTransfer& xfer);
#endif

#endif // _CANIMCHARACTERSET
