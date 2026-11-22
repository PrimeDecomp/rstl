#ifndef _CASSETIDLIST
#define _CASSETIDLIST

#include "rstl/static_assert.hpp"
#include "rstl/types.h"

#include "Kyoto/CAssetId.hpp"

#include "rstl/set.hpp"
#include "rstl/vector.hpp"

class CInputStream;

// Guessed name. A flat list of asset ids, built either from a stream or from a sorted set.
class CAssetIdList {
public:
  explicit CAssetIdList(CInputStream& in);
  explicit CAssetIdList(const rstl::set< CAssetId >& ids);

  const rstl::vector< CAssetId >& GetAssetIds() const;

private:
  rstl::vector< CAssetId > mAssetIds;
};
CHECK_SIZEOF(CAssetIdList, 0x10)

#endif // _CASSETIDLIST
