#ifndef _CDEPENDENCYGROUPTOKEN
#define _CDEPENDENCYGROUPTOKEN
#include "rstl/RstlVersions.h"
#if RSTL_KYOTO_CORRUPTION
#include "Corruption/Kyoto/TToken.hpp"
#else
#include "Kyoto/TToken.hpp"
#endif
#include "rstl/vector.hpp"

class CDependencyGroup;
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
