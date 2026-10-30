#ifndef _CCHARACTERSET
#define _CCHARACTERSET
#include "rstl/RstlVersions.h"

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#include "Kyoto/Animation/CCharacterInfo.hpp"
#else
#include "Kyoto/Animation/CCECharacterInfo.hpp"
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#include "rstl/map.hpp"

#include "types.h"
#else
#include "rstl/pair.hpp"
#include "rstl/vector.hpp"
#endif

class CInputStream;

class CCharacterSet {
public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CCharacterSet(CInputStream& in);
#else
  explicit CCharacterSet(CInputStream& in);
#endif

  const rstl::vector< rstl::pair< int,
#if RSTL_PRIME1
                                  CCharacterInfo
#else
                                  CCECharacterInfo
#endif
                                  > >&
  GetCharacterList() const {
    return mCharacters;
  }

private:
  ushort mTableCount;
  rstl::vector< rstl::pair< int,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                            CCharacterInfo
#else
                            CCECharacterInfo
#endif
                            > >
      mCharacters;
};
CHECK_SIZEOF(CCharacterSet, 0x14)

#endif // _CCHARACTERSET
