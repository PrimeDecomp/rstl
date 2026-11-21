#include "Kyoto/CAssetIdList.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

CAssetIdList::CAssetIdList(const rstl::set< CAssetId >& ids) : mAssetIds(ids.begin(), ids.end()) {}

CAssetIdList::CAssetIdList(CInputStream& in) : mAssetIds(in) {}

const rstl::vector< CAssetId >& CAssetIdList::GetAssetIds() const { return mAssetIds; }

