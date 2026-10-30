#ifndef _CTRANSITIONDATABASE
#define _CTRANSITIONDATABASE
#include "rstl/RstlVersions.h"

#include "Kyoto/TToken.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/rc_ptr.hpp"

class IMetaTrans;

class CTransitionDatabase {
public:
#if !RSTL_PRIME1
  TRILOGY_VIRTUAL ~CTransitionDatabase() {}
#endif

  virtual const rstl::rc_ptr< IMetaTrans >& GetMetaTrans(uint from, uint to) const = 0;

  explicit CTransitionDatabase(const rstl::optional_object< TToken< CTransitionDatabase > >& token =
                                   rstl::optional_object_null())
  : mToken(token) {}
#if RSTL_PRIME1
  ~CTransitionDatabase() {}
#endif

private:
  rstl::optional_object< TToken< CTransitionDatabase > > mToken;
};
DECLARE_FULL_SIZE_FOR(CTransitionDatabase, 0x10)
CHECK_SIZEOF(CTransitionDatabase, CTransitionDatabase_FULL_SIZE)

#endif // _CTRANSITIONDATABASE
