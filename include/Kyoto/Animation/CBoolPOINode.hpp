#ifndef _CBOOLPOINODE
#define _CBOOLPOINODE
#include "rstl/RstlVersions.h"

#include "Kyoto/Animation/CPOINode.hpp"

class CBoolPOINode : public CPOINode {
public:
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

  CBoolPOINode(CInputStream& in);
  static CBoolPOINode CopyNodeMinusStartTime(const CBoolPOINode& node,
                                             const CCharAnimTime& startTime);
// TODO: verify that this Prime/Echoes difference is real
  RSTL_PRIME1_CONST bool GetValue() const { return mVal; }

private:
  bool mVal;
};

#endif // _CBOOLPOINODE
