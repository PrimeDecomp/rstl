#include "Kyoto/Animation/CAnimPOIDatabase.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

#include "rstl/algorithm.hpp"
#include "rstl/functional.hpp"



CAnimPOIDatabase::CAnimPOIDatabase(CInputStream& in) : mVersion(in.Get< uchar >()), mEntries(in) {}

const CAnimPOIData* CAnimPOIDatabase::FindPOIDataByAssetId(const CAssetId& assetId) const {
  const CAssetId id = assetId;
  rstl::vector< TEntry >::const_iterator it = rstl::binary_find(
      mEntries.begin(), mEntries.end(), id,
      rstl::pair_sorter_finder< TEntry, rstl::less< CAssetId > >(rstl::less< CAssetId >()));
  if (it != mEntries.end()) {
    return &it->second;
  }
  return nullptr;
}

CAnimPOIDatabase::~CAnimPOIDatabase() {}
