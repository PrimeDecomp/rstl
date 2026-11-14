#ifndef _CANIMATIONDATABASE
#define _CANIMATIONDATABASE

#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"

#include "Kyoto/TToken.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/set.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

class CPrimitive;
class CAnimation;
class CAnimTreeNode;
class CAnimSysContext;
class CMetaAnimTreeBuildOrders;

class IMetaAnim;

class CAnimationDatabase {
public:
  RSTL_TRILOGY_VIRTUAL ~CAnimationDatabase() {}

  virtual const rstl::rc_ptr< IMetaAnim >& GetMetaAnim(uint idx) const = 0;
  virtual uint GetNumMetaAnims() const = 0;
  virtual rstl::string GetMetaAnimName(uint idx) const = 0;
  virtual void GetAllUniquePrimitives(rstl::vector< CPrimitive >& primsOut) const = 0;
  virtual void GetUniquePrimitivesFromMetaAnim(rstl::set< CPrimitive >& primsOut,
                                               const rstl::string& name) const = 0;

  explicit CAnimationDatabase(const rstl::optional_object< TToken< CAnimationDatabase > >& token =
                                  rstl::optional_object_null())
  : x4_(token) {}

private:
  rstl::optional_object< TToken< CAnimationDatabase > > x4_;
};
DECLARE_FULL_SIZE_FOR(CAnimationDatabase, 0x10)
CHECK_SIZEOF(CAnimationDatabase, CAnimationDatabase_FULL_SIZE)

#endif // _CANIMATIONDATABASE
