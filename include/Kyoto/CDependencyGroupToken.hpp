#ifndef _CDEPENDENCYGROUPTOKEN
#define _CDEPENDENCYGROUPTOKEN

#include "Kyoto/CDependencyGroup.hpp"
#include "Kyoto/TToken.hpp"
#include "rstl/RstlVersions.h"
#include "rstl/static_assert.hpp"
#include "rstl/vector.hpp"

class IObjectStore;

// Guessed name. Owns the group token and its dependency tokens.
class CDependencyGroupToken {
public:
  CDependencyGroupToken(const TToken< CDependencyGroup >& group, IObjectStore& store);

  void Lock();
  void Unlock();
  bool IsLocked() const;
  bool IsLoaded() const;

private:
  TToken< CDependencyGroup > mGroup;
  rstl::vector< CToken > mDependencies;
  uint mLockCount;
  mutable bool mLoaded : 1;
};
CHECK_SIZEOF(CDependencyGroupToken, 0x20)

#endif // _CDEPENDENCYGROUPTOKEN
