#ifndef _CPRIMITIVE
#define _CPRIMITIVE
#include "rstl/RstlVersions.h"

#include "rstl/static_assert.hpp"
#include "Kyoto/SObjectTag.hpp"
#if !RSTL_CORRUPTION
#include "rstl/RstlVersions.h"
#endif

#include "rstl/StringExtras.hpp"
#include "rstl/string.hpp"

class CInputStream;
class COutputStream;

class CPrimitive {
public:
  CPrimitive(CInputStream& in);
  void PutTo(COutputStream& out) const;
#if RSTL_CORRUPTION
  const CAssetId& GetResourceId() const { return mAnimId; }
  int GetAnimDatabaseIndex() const { return mAnimIdx; }
#else
  CAssetId GetResourceId() const { return mAnimId; }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  uint GetAnimDatabaseIndex() const { return mAnimIdx; }
#else
  int GetAnimDatabaseIndex() const { return mAnimIdx; }
#endif
#endif
  const rstl::string& GetName() const { return mAnimName; }
  int Compare(const CPrimitive& other) const {
    return CStringExtras::CompareCaseInsensitive(mAnimName, other.mAnimName);
  }

private:
  CAssetId mAnimId;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  uint mAnimIdx;
#else
  int mAnimIdx;
#endif
  rstl::string mAnimName;
};

#if RSTL_CORRUPTION
CHECK_SIZEOF(CPrimitive, 0x20)
#else
CHECK_SIZEOF(CPrimitive, 0x18)
#endif

inline bool operator<(const CPrimitive& a, const CPrimitive& b) { return a.Compare(b) < 0; }

#endif // _CPRIMITIVE
