#ifndef _CBOOLPOINODE
#define _CBOOLPOINODE
#include "rstl/RstlVersions.h"
#include "rstl/RstlVersions.h"

#if RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#endif
#include "Kyoto/Animation/CPOINode.hpp"

class CBoolPOINode : public CPOINode {
public:
#if RSTL_CORRUPTION
  CBoolPOINode(uint nameHash = -1, EPOIType type = kPT_EmptyBool,
               const CCharAnimTime& time = CCharAnimTime(), int index = -1, bool unique = false,
               float weight = 1.f, int charIdx = -1, int flags = 0,
               int unk2C = CPOINode::skDefaultUnk2C, const CSegId& segId = CSegId(),
               bool value = false)
  : CPOINode(nameHash, type, time, index, unique, weight, charIdx, flags, unk2C, segId)
  , mVal(value) {}
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CBoolPOINode(const rstl::string name = ""
#else
  CBoolPOINode(uint name = -1
#endif
               , RSTL_PRIME1_CONST EPOIType type = kPT_EmptyBool,
               const CCharAnimTime& time = CCharAnimTime(), RSTL_PRIME1_CONST int index = -1,
               RSTL_PRIME1_CONST bool unique = false, RSTL_PRIME1_CONST float weight = 1.f,
               RSTL_PRIME1_CONST int charIdx = -1, RSTL_PRIME1_CONST int flags = 0,
               RSTL_PRIME1_CONST bool value = false)
  : CPOINode(name, type, time, index, unique, weight, charIdx, flags), mVal(value) {}
#endif

  CBoolPOINode(CInputStream& in);
  static CBoolPOINode CopyNodeMinusStartTime(const CBoolPOINode& node,
                                             const CCharAnimTime& startTime);
#if RSTL_CORRUPTION
  bool GetValue() const { return mVal; }
#else
  RSTL_PRIME1_CONST bool GetValue() const { return mVal; }
#endif

private:
  bool mVal;
};
#if RSTL_CORRUPTION
CHECK_SIZEOF(CBoolPOINode, 0x38)
#endif

#endif // _CBOOLPOINODE
