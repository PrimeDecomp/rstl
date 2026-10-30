#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/IAnimReader.hpp"
#include "Kyoto/Animation/CParticlePOINode.hpp"
#include "Kyoto/Animation/CSoundPOINode.hpp"

IAnimReader::~IAnimReader() {}

rstl::optional_object< rstl::ownership_transfer< IAnimReader > > IAnimReader::VSimplified() {
  return rstl::optional_object_null();
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
SAdvancementResults IAnimReader::VGetAdvancementResults(const CCharAnimTime& aTime,
                                                        const CCharAnimTime& bTime) const {
  return SAdvancementResults::RemainderOnly(aTime);
};
#else
SAdvancementResults IAnimReader::VGetAdvancementResults(const CCharAnimTime& time,
                                                        const CCharAnimTime&) const {
  return SAdvancementResults::RemainderOnly(time);
}
#endif

uint IAnimReader::GetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, uint capacity,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                                 uint iterator, int unk) const {
  if (time.GreaterThanZero()) {
    return VGetBoolPOIList(time, listOut, capacity, iterator, unk);
  }
  return 0;
#else
                                 uint iterator, int additive) const {
  return time.GreaterThanZero() ? VGetBoolPOIList(time, listOut, capacity, iterator, additive) : 0;
#endif

}

uint IAnimReader::GetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, uint capacity,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                                  uint iterator, int unk) const {
  if (time.GreaterThanZero()) {
    return VGetInt32POIList(time, listOut, capacity, iterator, unk);
  }
  return 0;
#else
                                  uint iterator, int additive) const {
  return time.GreaterThanZero() ? VGetInt32POIList(time, listOut, capacity, iterator, additive) : 0;
#endif

}

uint IAnimReader::GetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                                     uint capacity, uint iterator, int unk) const {
  if (time.GreaterThanZero()) {
    return VGetParticlePOIList(time, listOut, capacity, iterator, unk);
  }
  return 0;
#else
                                     uint capacity, uint iterator, int additive) const {
  return time.GreaterThanZero() ? VGetParticlePOIList(time, listOut, capacity, iterator, additive)
                                : 0;
#endif

}

uint IAnimReader::GetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, uint capacity,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                                  uint iterator, int unk) const {
  if (time.GreaterThanZero()) {
    return VGetSoundPOIList(time, listOut, capacity, iterator, unk);
  }
  return 0;
#else
                                  uint iterator, int additive) const {
  return time.GreaterThanZero() ? VGetSoundPOIList(time, listOut, capacity, iterator, additive) : 0;
#endif

}

bool IAnimReader::IsCAnimTreeNode() const { return false; }

#if !RSTL_PRIME1
void IAnimReader::VGetJointData_Linear(const CCharLayoutInfo&, CJointData_LinearStorage&,
                                       const CCharAnimTime&) const {}
#endif

#if !RSTL_PRIME1
void IAnimReader::VGetJointData_Linear(const CCharLayoutInfo&, CJointData_LinearStorage&) const {}
#endif
