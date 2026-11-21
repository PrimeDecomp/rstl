#ifndef _CANIMATIONDATABASE
#define _CANIMATIONDATABASE
#include "rstl/RstlVersions.h"

#include "rstl/static_assert.hpp"
#if RSTL_CORRUPTION
#include "rstl/types.h"
#else
#include "rstl/RstlVersions.h"
#endif

#include "Kyoto/TToken.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/set.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

#if !RSTL_CORRUPTION
class CPrimitive;
class CAnimation;
class CAnimTreeNode;
class CAnimSysContext;
class CMetaAnimTreeBuildOrders;
#endif

#if RSTL_CORRUPTION

class CAnimation;
class CPrimitive;
#endif
class IMetaAnim;

class CAnimationDatabase {
public:
#if RSTL_CORRUPTION
  explicit CAnimationDatabase(const rstl::vector< CAnimation >& animations);
#else
  RSTL_TRILOGY_VIRTUAL ~CAnimationDatabase() {}
#endif

#if RSTL_CORRUPTION
  rstl::rc_ptr< IMetaAnim > GetMetaAnim(uint idx) const;
  rstl::string GetMetaAnimName(uint idx) const;
  void GetUniquePrimitivesFromMetaAnim(rstl::set< CPrimitive >& primsOut,
                                       const rstl::string& name) const;
#else
  virtual const rstl::rc_ptr< IMetaAnim >& GetMetaAnim(uint idx) const = 0;
  virtual uint GetNumMetaAnims() const = 0;
  virtual rstl::string GetMetaAnimName(uint idx) const = 0;
  virtual void GetAllUniquePrimitives(rstl::vector< CPrimitive >& primsOut) const = 0;
  virtual void GetUniquePrimitivesFromMetaAnim(rstl::set< CPrimitive >& primsOut,
                                               const rstl::string& name) const = 0;

  explicit CAnimationDatabase(const rstl::optional_object< TToken< CAnimationDatabase > >& token =
                                  rstl::optional_object_null())
  : x4_(token) {}
#endif

private:
#if RSTL_CORRUPTION
  rstl::vector< rstl::rc_ptr< IMetaAnim > > mMetaAnims;
#else
  rstl::optional_object< TToken< CAnimationDatabase > > x4_;
#endif
};
#if RSTL_CORRUPTION
CHECK_SIZEOF(CAnimationDatabase, 0x10)
#else
DECLARE_FULL_SIZE_FOR(CAnimationDatabase, 0x10)
CHECK_SIZEOF(CAnimationDatabase, CAnimationDatabase_FULL_SIZE)
#endif

#endif // _CANIMATIONDATABASE
