#ifndef _CANIMATIONDATABASE
#define _CANIMATIONDATABASE
#include "rstl/RstlVersions.h"

#include "Kyoto/TToken.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/set.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

class CPrimitive;
#if RSTL_PRIME1
class CAnimation;
class CAnimTreeNode;
class CAnimSysContext;
class CMetaAnimTreeBuildOrders;
#endif

class IMetaAnim;

class CAnimationDatabase {
public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_R3ME_00
  virtual
#endif
  ~CAnimationDatabase() {}
#else
  TRILOGY_VIRTUAL ~CAnimationDatabase() {}
#endif

  virtual const rstl::rc_ptr< IMetaAnim >& GetMetaAnim(uint idx) const = 0;
  virtual uint GetNumMetaAnims() const = 0;
  virtual rstl::string GetMetaAnimName(uint idx) const = 0;
  virtual void GetAllUniquePrimitives(rstl::vector< CPrimitive >& primsOut) const = 0;
  virtual void GetUniquePrimitivesFromMetaAnim(rstl::set< CPrimitive >& primsOut,
                                               const rstl::string& name) const = 0;

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  explicit CAnimationDatabase(
      const rstl::optional_object< TToken< CAnimationDatabase > >& token = rstl::optional_object_null())
#else
  explicit CAnimationDatabase(const rstl::optional_object< TToken< CAnimationDatabase > >& token =
                                  rstl::optional_object_null())
#endif

  : x4_(token) {}

private:
  rstl::optional_object< TToken< CAnimationDatabase > > x4_;
};
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CHECK_SIZEOF(CAnimationDatabase, 0x10)
#else
DECLARE_FULL_SIZE_FOR(CAnimationDatabase, 0x10)
CHECK_SIZEOF(CAnimationDatabase, CAnimationDatabase_FULL_SIZE)
#endif

#endif // _CANIMATIONDATABASE
