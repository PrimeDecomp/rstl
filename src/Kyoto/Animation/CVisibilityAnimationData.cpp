#include "Kyoto/Animation/CVisibilityAnimationData.hpp"

#include "Kyoto/Alloc/Assert.hpp"
#include "Kyoto/Basics/PackedFloat.hpp"

float CVisibilityAnimationDataReader::GetTime() const { return PackedFloat_LoadFloat32(this); }

const uint* CVisibilityAnimationDataReader::GetBitFlagsData() const {
  return reinterpret_cast< const uint* >(reinterpret_cast< const uchar* >(this) +
                                         sizeof(x0_) + sizeof(mVisiblityBitFlagsArraySize));
}

uint CVisibilityAnimationDataReader::GetVisibilityBitFlags(uchar arrayIndex) const {
  RS_VERIFY_THROW_DIRECT(
      76, arrayIndex < mVisiblityBitFlagsArraySize, false,
      "CVisibilityAnimationDataReader::GetVisibilityBitFlags -- Invalid array index!");
  return GetBitFlagsData()[arrayIndex];
}

