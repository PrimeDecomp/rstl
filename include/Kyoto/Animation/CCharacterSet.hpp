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

class CInputStream;
#else
#include "rstl/pair.hpp"
#include "rstl/vector.hpp"
#endif

class CCharacterSet {
public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CCharacterSet(CInputStream& in);
  const rstl::vector< rstl::pair< int, CCharacterInfo > >& GetCharacterList() const {
#else
  explicit CCharacterSet(CInputStream& in);

  const rstl::vector< rstl::pair< int, CCECharacterInfo > >& GetCharacterList() const {
#endif

    return mCharacters;
  }

private:
  ushort mTableCount;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  rstl::vector< rstl::pair< int, CCharacterInfo > > mCharacters;
#else
  rstl::vector< rstl::pair< int, CCECharacterInfo > > mCharacters;
#endif

};
CHECK_SIZEOF(CCharacterSet, 0x14)

#endif // _CCHARACTERSET
