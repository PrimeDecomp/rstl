#ifndef _CMATERIALFILTER
#define _CMATERIALFILTER

#include "rstl/RstlVersions.h"
#include "rstl/static_assert.hpp"
#include "rstl/types.h"

#include "Collision/CMaterialList.hpp"

class CMaterialFilter {
public:
  static const CMaterialFilter skPassEverything;

  enum EFilterType {
    kFT_Always,
    kFT_Include,
    kFT_Exclude,
    kFT_IncludeExclude,
#if !RSTL_PRIME1
    kFT_Never, // Guessed name.
#endif
  };

  CMaterialFilter() : mInclude(0x00000000FFFFFFFF), mExclude(0), mType(kFT_Always) {}
  CMaterialFilter(const CMaterialList& include, const CMaterialList& exclude, EFilterType type)
  : mInclude(include), mExclude(exclude), mType(type) {}

  const CMaterialList& GetIncludeList() const { return mInclude; }
  CMaterialList& IncludeList() { return mInclude; }
  const CMaterialList& GetExcludeList() const { return mExclude; }
  CMaterialList& ExcludeList() { return mExclude; }
  EFilterType GetType() const { return mType; }
  static CMaterialFilter MakeInclude(const CMaterialList& include) {
    return CMaterialFilter(include, CMaterialList(), kFT_Include);
  }
  static CMaterialFilter MakeExclude(const CMaterialList& exclude) {
    return CMaterialFilter(CMaterialList(0x00000000FFFFFFFF), exclude, kFT_Exclude);
  }
  static CMaterialFilter MakeIncludeExclude(const CMaterialList& include,
                                            const CMaterialList& exclude) {
    return CMaterialFilter(include, exclude, kFT_IncludeExclude);
  }

  static const CMaterialFilter& GetPassEverything() { return skPassEverything; }

  bool Passes(const CMaterialList& other) const;
#if !RSTL_PRIME1
  // Guessed name; simplify the filter for geometry carrying these fixed materials.
  CMaterialFilter WithImplicitMaterials(const CMaterialList& materials) const;
#endif

private:
  CMaterialList mInclude;
  CMaterialList mExclude;
  EFilterType mType;
};
CHECK_SIZEOF(CMaterialFilter, 0x18)

#endif // _CMATERIALFILTER
