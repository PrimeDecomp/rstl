#ifndef _CPASANIMINFO
#define _CPASANIMINFO

#include "Kyoto/Animation/CPASAnimParm.hpp"

#include "rstl/RstlVersions.h"
#include "rstl/vector.hpp"
#include "rstl/reserved_vector.hpp"

class CPASAnimInfo {
public:
  CPASAnimInfo(int id);
  CPASAnimInfo(int id, const rstl::reserved_vector< CPASAnimParm::UParmValue, 8 >& parms);

  CPASAnimInfo(const CPASAnimInfo& other) : mId(other.GetAnimId()), mParms(other.mParms) {}

  int GetAnimId() const { return mId; }
  CPASAnimParm GetAnimParmData(uint idx, CPASAnimParm::EParmType type) const;
  const CPASAnimParm::UParmValue& GetAnimParmValue(uint idx) const;

  bool operator<(const CPASAnimInfo& rhs) const { return mId < rhs.mId; }

private:
  int mId;
  rstl::reserved_vector< CPASAnimParm::UParmValue, 8 > mParms;
};
CHECK_SIZEOF(CPASAnimInfo, 0x28)

#if RSTL_KYOTO_CONSTRUCTION_POLICIES
namespace rstl {
RSTL_DECLARE_TRIVIALLY_DESTRUCTIBLE(CPASAnimInfo)
RSTL_DECLARE_BITWISE_CONSTRUCTION(CPASAnimInfo)

template <> struct vector_copy_construction_policy< CPASAnimInfo > {
  enum { value = true };
  static void copy_n(CPASAnimInfo* src, int n, CPASAnimInfo* dest) {
    CPASAnimInfo* cur = dest;
    for (int remaining = n; remaining != 0; --remaining, ++src, ++cur) {
      bitwise_copy< sizeof(CPASAnimInfo) / sizeof(double) >(cur, src);
    }
  }
};
} // namespace rstl
#endif

#endif // _CPASANIMINFO
