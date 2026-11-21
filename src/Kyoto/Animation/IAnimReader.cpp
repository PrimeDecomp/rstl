#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/RstlVersions.h"
#endif
#include "Kyoto/Animation/IAnimReader.hpp"
#if RSTL_CORRUPTION

#include "Kyoto/Alloc/Assert.hpp"


#else
#include "Kyoto/Animation/CParticlePOINode.hpp"
#include "Kyoto/Animation/CSoundPOINode.hpp"
#endif

IAnimReader::~IAnimReader() {}

rstl::optional_object< rstl::ownership_transfer< IAnimReader > > IAnimReader::VSimplified() {
  return rstl::optional_object_null();
}

SAdvancementResults IAnimReader::VGetAdvancementResults(const CCharAnimTime& time,
                                                        const CCharAnimTime&) const {
  return SAdvancementResults::RemainderOnly(time);
}
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_CORRUPTION
#if RSTL_PRIME1
;
#endif

#endif
uint IAnimReader::GetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, uint capacity,
#if RSTL_CORRUPTION
                                 uint iterator, const CSegIdMaskMap& mask) const {
  return time.GreaterThanZero() ? VGetBoolPOIList(time, listOut, capacity, iterator, mask) : 0;
#else
                                 uint iterator, int additive) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (time.GreaterThanZero()) {
    return VGetBoolPOIList(time, listOut, capacity, iterator, additive);
  }
  return 0;
#else
  return time.GreaterThanZero() ? VGetBoolPOIList(time, listOut, capacity, iterator, additive) : 0;
#endif
}

uint IAnimReader::GetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, uint capacity,
                                  uint iterator, int additive) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (time.GreaterThanZero()) {
    return VGetInt32POIList(time, listOut, capacity, iterator, additive);
  }
  return 0;
#else
  return time.GreaterThanZero() ? VGetInt32POIList(time, listOut, capacity, iterator, additive) : 0;
#endif
#endif
}

uint IAnimReader::GetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut,
#if RSTL_CORRUPTION
                                     uint capacity, uint iterator, const CSegIdMaskMap& mask) const {
  return time.GreaterThanZero() ? VGetParticlePOIList(time, listOut, capacity, iterator, mask)
                                : 0;
#else
                                     uint capacity, uint iterator, int additive) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (time.GreaterThanZero()) {
    return VGetParticlePOIList(time, listOut, capacity, iterator, additive);
  }
  return 0;
#else
  return time.GreaterThanZero() ? VGetParticlePOIList(time, listOut, capacity, iterator, additive)
                                : 0;
#endif
#endif
}

uint IAnimReader::GetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, uint capacity,
#if RSTL_CORRUPTION
                                  uint iterator, const CSegIdMaskMap& mask) const {
  return time.GreaterThanZero() ? VGetSoundPOIList(time, listOut, capacity, iterator, mask) : 0;
  }

uint IAnimReader::GetAssetSoundPOIList(const CCharAnimTime& time, CAssetSoundPOINode* listOut,
                                       uint capacity, uint iterator, const CSegIdMaskMap& mask) const {
  return time.GreaterThanZero() ? VGetAssetSoundPOIList(time, listOut, capacity, iterator, mask)
                                : 0;
}

uint IAnimReader::GetAssetPOIList(const CCharAnimTime& time, CAssetPOINode* listOut, uint capacity,
                                  uint iterator, const CSegIdMaskMap& mask) const {
  return time.GreaterThanZero() ? VGetAssetPOIList(time, listOut, capacity, iterator, mask) : 0;
#else
                                  uint iterator, int additive) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (time.GreaterThanZero()) {
    return VGetSoundPOIList(time, listOut, capacity, iterator, additive);
  }
  return 0;
#else
  return time.GreaterThanZero() ? VGetSoundPOIList(time, listOut, capacity, iterator, additive) : 0;
#endif
#endif
}

bool IAnimReader::IsCAnimTreeNode() const { return false; }

#if RSTL_CORRUPTION
void IAnimReader::VGetJointData_Linear(const CCharLayoutInfo&, CJointData_LinearStorage&,
                                       const CCharAnimTime&) const {
  RS_VERIFY_THROW(172, false, false, "Function not yet impletemented.");
}

void IAnimReader::VGetJointData_Linear(const CCharLayoutInfo&, CJointData_LinearStorage&) const {
  RS_VERIFY_THROW(181, false, false, "Function not yet impletemented.");
}

const CVisibilityAnimationDataReader* IAnimReader::GetVisibilityDataAtCurrentTime() const {
  return VGetVisibilityDataAtCurrentTime();
}

const CVisibilityAnimationDataReader*
IAnimReader::GetVisibilityDataAtTime(const CCharAnimTime& time) const {
  return VGetVisibilityDataAtTime(time);
}
#else
#if !RSTL_PRIME1
void IAnimReader::VGetJointData_Linear(const CCharLayoutInfo&, CJointData_LinearStorage&,
                                       const CCharAnimTime&) const {}

void IAnimReader::VGetJointData_Linear(const CCharLayoutInfo&, CJointData_LinearStorage&) const {}
#endif
#endif
