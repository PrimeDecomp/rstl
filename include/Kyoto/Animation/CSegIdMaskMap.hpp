#ifndef _CSEGIDMASKMAP
#define _CSEGIDMASKMAP

#include "rstl/static_assert.hpp"
#include "rstl/types.h"

#include "Kyoto/Animation/CSegId.hpp"

#include "rstl/vector.hpp"



class CSegIdMaskMap { // Guessed name
public:
  CSegIdMaskMap(int count, int mask);

  int GetMask(const CSegId& id) const; // Guessed name
  void SetAll(int mask); // Guessed name

private:
  rstl::vector< int > mMasks;
};
CHECK_SIZEOF(CSegIdMaskMap, 0x10)


#endif // _CSEGIDMASKMAP
