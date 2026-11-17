#ifndef _CAREABSPTREE
#define _CAREABSPTREE

#include "rstl/static_assert.hpp"
#include "rstl/types.h"

class CInputStream;
class CTransform4f;

class CAreaBspTree {
public:
  CAreaBspTree(CInputStream& in, const CTransform4f& transform);
};
CHECK_SIZEOF(CAreaBspTree, 1)

#endif // _CAREABSPTREE
