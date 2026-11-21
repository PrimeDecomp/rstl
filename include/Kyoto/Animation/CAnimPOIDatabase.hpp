#ifndef _CANIMPOIDATABASE
#define _CANIMPOIDATABASE

#include "rstl/static_assert.hpp"
#include "rstl/types.h"

#include "Kyoto/Animation/CAnimPOIData.hpp"
#include "Kyoto/CAssetId.hpp"

#include "rstl/pair.hpp"
#include "rstl/vector.hpp"



class CInputStream;

// Guessed name. Lets the animation system look up the points of interest of an animation asset.
class IAnimPOIDatabase {
public:
  virtual ~IAnimPOIDatabase() {}
  virtual const CAnimPOIData* FindPOIDataByAssetId(const CAssetId& id) const = 0;
};

class CAnimPOIDatabase : public IAnimPOIDatabase {
public:
  typedef rstl::pair< CAssetId, CAnimPOIData > TEntry;

  explicit CAnimPOIDatabase(CInputStream& in);
  ~CAnimPOIDatabase() override;

  const CAnimPOIData* FindPOIDataByAssetId(const CAssetId& id) const override;

private:
  uchar mVersion;                  // Guessed name.
  rstl::vector< TEntry > mEntries; // Sorted by asset id.
};
CHECK_SIZEOF(CAnimPOIDatabase, 0x18)


#endif // _CANIMPOIDATABASE
