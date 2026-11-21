#ifndef _CINT32POINODE
#define _CINT32POINODE
#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"
#endif

#if RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#endif
#include "Kyoto/Animation/CPOINode.hpp"
#if !RSTL_CORRUPTION
#include "rstl/string.hpp"
#endif

class CInt32POINode : public CPOINode {
public:
#if RSTL_CORRUPTION
  CInt32POINode(uint nameHash = -1, EPOIType type = kPT_EmptyInt32,
                const CCharAnimTime& time = CCharAnimTime(), int index = -1, bool unique = false,
                float weight = 1.f, int charIdx = -1, int flags = 0,
                int unk2C = CPOINode::skDefaultUnk2C, const CSegId& segId = CSegId(), int value = 0)
  : CPOINode(nameHash, type, time, index, unique, weight, charIdx, flags, unk2C, segId)
  , mVal(value) {}
#else
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
#endif

  explicit CInt32POINode(CInputStream& in);

  static CInt32POINode CopyNodeMinusStartTime(const CInt32POINode& node,
                                              const CCharAnimTime& startTime);

  int GetValue() const { return mVal; }
#if !RSTL_CORRUPTION
  const rstl::string& GetLocatorName() const { return mLctrName; }
#endif

private:
  int mVal;
#if !RSTL_CORRUPTION
  rstl::string mLctrName;
#endif
};
#if RSTL_CORRUPTION
CHECK_SIZEOF(CInt32POINode, 0x38)
#else
#if !RSTL_PRIME1
CHECK_CHILD_SIZEOF(CInt32POINode, CPOINode, 0x14)
#endif
#endif

#endif // _CINT32POINODE
