#ifndef _CBOOLPOINODE
#define _CBOOLPOINODE
#include "rstl/RstlVersions.h"

#include "Kyoto/Animation/CPOINode.hpp"

class CBoolPOINode : public CPOINode {
public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CBoolPOINode(const rstl::string name = "", const EPOIType type = kPT_EmptyBool,
               const CCharAnimTime& time = CCharAnimTime(), const int index = -1,
               const bool unique = false, const float weight = 1.f, const int charIdx = -1,
               const int flags = 0, const bool value = false)
  : CPOINode(name, type, time, index, unique, weight, charIdx, flags), mVal(value) {}
#else
  CBoolPOINode(uint nameHash = -1, EPOIType type = kPT_EmptyBool,
               const CCharAnimTime& time = CCharAnimTime(), int index = -1, bool unique = false,
               float weight = 1.f, int charIdx = -1, int flags = 0, bool value = false)
  : CPOINode(nameHash, type, time, index, unique, weight, charIdx, flags), mVal(value) {}
#endif

  CBoolPOINode(CInputStream& in);
  static CBoolPOINode CopyNodeMinusStartTime(const CBoolPOINode& node,
                                             const CCharAnimTime& startTime);
// TODO: verify that this Prime/Echoes difference is real
  RSTL_PRIME1_CONST bool GetValue() const { return mVal; }

private:
  bool mVal;
};

#endif // _CBOOLPOINODE
