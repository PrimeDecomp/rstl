#ifndef _CPRIMITIVE
#define _CPRIMITIVE

#include "Kyoto/SObjectTag.hpp"
#include "rstl/RstlVersions.h"

#include "rstl/StringExtras.hpp"
#include "rstl/string.hpp"

class CInputStream;
class COutputStream;

class CPrimitive {
public:
  CPrimitive(CInputStream& in);
  void PutTo(COutputStream& out) const;
  CAssetId GetResourceId() const { return mAnimId; }
#if RSTL_PRIME1
  uint GetAnimDatabaseIndex() const { return mAnimIdx; }
#else
  int GetAnimDatabaseIndex() const { return mAnimIdx; }
#endif
  const rstl::string& GetName() const { return mAnimName; }
  int Compare(const CPrimitive& other) const {
    return CStringExtras::CompareCaseInsensitive(mAnimName, other.mAnimName);
  }

private:
  CAssetId mAnimId;
#if RSTL_PRIME1
  uint mAnimIdx;
#else
  int mAnimIdx;
#endif
  rstl::string mAnimName;
};

CHECK_SIZEOF(CPrimitive, 0x18)

inline bool operator<(const CPrimitive& a, const CPrimitive& b) { return a.Compare(b) < 0; }

#endif // _CPRIMITIVE
