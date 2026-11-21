#include "Kyoto/Animation/CSegIdMaskMap.hpp"



CSegIdMaskMap::CSegIdMaskMap(const int count, int mask) : mMasks(count, mask) {}

int CSegIdMaskMap::GetMask(const CSegId& id) const {
  return id.val() < static_cast<uint>(mMasks.size()) ? mMasks[id.val()] : 0;
}

void CSegIdMaskMap::SetAll(int mask) {
  const uint count = mMasks.size();
  for (uint i = 0; i < count; ++i) {
    int& value = mMasks[i];
    value = mask;
  }
}
