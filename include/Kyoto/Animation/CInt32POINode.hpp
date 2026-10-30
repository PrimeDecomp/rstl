#ifndef _CINT32POINODE
#define _CINT32POINODE
#include "rstl/RstlVersions.h"

#include "Kyoto/Animation/CPOINode.hpp"
#if !RSTL_PRIME1
#include "rstl/string.hpp"
#endif

class CInt32POINode : public CPOINode {
public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CInt32POINode(const rstl::string name = rstl::string_l(""), const EPOIType type = kPT_EmptyInt32,
                const CCharAnimTime& time = CCharAnimTime(), const int index = -1,
                const bool unique = false, const float weight = 1.f, const int charIdx = -1,
                const int flags = 0, const int value = 0,
#else
  CInt32POINode(uint nameHash = -1, EPOIType type = kPT_EmptyInt32,
                const CCharAnimTime& time = CCharAnimTime(), int index = -1, bool unique = false,
                float weight = 1.f, int charIdx = -1, int flags = 0, int value = 0,
#endif

                const rstl::string& locatorName = rstl::string_l("root"))
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  : CPOINode(name, type, time, index, unique, weight, charIdx, flags)
#else
  : CPOINode(nameHash, type, time, index, unique, weight, charIdx, flags)
#endif

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
