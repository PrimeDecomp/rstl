#ifndef _CTRANSITIONDATABASE
#define _CTRANSITIONDATABASE
#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"
#endif

#if RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#else
#include "Kyoto/TToken.hpp"
#include "rstl/optional_object.hpp"
#endif
#include "rstl/rc_ptr.hpp"

#if RSTL_CORRUPTION


class CAdditiveAnimationInfo;
class CAssetId;
#endif
class IMetaTrans;

class CTransitionDatabase {
public:
#if RSTL_CORRUPTION
  virtual ~CTransitionDatabase() {}
  virtual const rstl::rc_ptr< IMetaTrans >& GetMetaTrans(const CAssetId& from, const CAssetId& to) const = 0;
#else
#if !RSTL_PRIME1
  RSTL_TRILOGY_VIRTUAL ~CTransitionDatabase() {}
#endif
#endif
  // Guessed name. CTransitionDatabaseGame answers true when neither a transition nor a half
#if !RSTL_CORRUPTION
  virtual const rstl::rc_ptr< IMetaTrans >& GetMetaTrans(uint from, uint to) const = 0;
#endif

#if RSTL_CORRUPTION
  virtual bool UsesDefaultTransition(const CAssetId& from, const CAssetId& to) const = 0;
#else
  explicit CTransitionDatabase(const rstl::optional_object< TToken< CTransitionDatabase > >& token =
                                   rstl::optional_object_null())
  : mToken(token) {}
#if RSTL_PRIME1
  ~CTransitionDatabase() {}
#endif
#endif
  // Guessed name, after Metaforce's CCharacterFactory::FindAdditiveInfo, which this lookup
#if !RSTL_CORRUPTION
private:
  rstl::optional_object< TToken< CTransitionDatabase > > mToken;
};
DECLARE_FULL_SIZE_FOR(CTransitionDatabase, 0x10)
CHECK_SIZEOF(CTransitionDatabase, CTransitionDatabase_FULL_SIZE)
#endif

#if RSTL_CORRUPTION
  virtual const CAdditiveAnimationInfo& FindAdditiveInfo(const CAssetId& anim) const = 0;
};
CHECK_SIZEOF(CTransitionDatabase, 0x4)


#endif
#endif // _CTRANSITIONDATABASE
