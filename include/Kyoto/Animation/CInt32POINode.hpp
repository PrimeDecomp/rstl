#ifndef _CINT32POINODE
#define _CINT32POINODE
#include "rstl/RstlVersions.h"

#include "Kyoto/Animation/CPOINode.hpp"
#include "rstl/string.hpp"

class CInt32POINode : public CPOINode {
public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CInt32POINode(const rstl::string name = rstl::string_l("")
#else
  CInt32POINode(uint name = -1
#endif
                , RSTL_PRIME1_CONST EPOIType type = kPT_EmptyInt32,
                const CCharAnimTime& time = CCharAnimTime(), RSTL_PRIME1_CONST int index = -1,
                RSTL_PRIME1_CONST bool unique = false, RSTL_PRIME1_CONST float weight = 1.f,
                RSTL_PRIME1_CONST int charIdx = -1, RSTL_PRIME1_CONST int flags = 0,
                RSTL_PRIME1_CONST int value = 0,
                const rstl::string& locatorName = rstl::string_l("root"))
  : CPOINode(name, type, time, index, unique, weight, charIdx, flags)
  , mVal(value)
  , mLctrName(locatorName) {}

  explicit CInt32POINode(CInputStream& in);

  static CInt32POINode CopyNodeMinusStartTime(const CInt32POINode& node,
                                              const CCharAnimTime& startTime);

  int GetValue() const { return mVal; }
  const rstl::string& GetLocatorName() const { return mLctrName; }

private:
  int mVal;
  rstl::string mLctrName;
};
#if !RSTL_PRIME1
CHECK_CHILD_SIZEOF(CInt32POINode, CPOINode, 0x14)
#endif

#endif // _CINT32POINODE
