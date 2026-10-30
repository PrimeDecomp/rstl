#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CCharacterSet.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

CCharacterSet::CCharacterSet(CInputStream& in) : mTableCount(in.Get< ushort >()) {
  int count = in.Get< int >();
  mCharacters.reserve(count);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  for (int i = 0; i < count; i++) {
    mCharacters.push_back(in);
  }
#else
  for (int i = 0; i < count; ++i) {
    mCharacters.push_back_unsafe(in);
  }
#endif
}
