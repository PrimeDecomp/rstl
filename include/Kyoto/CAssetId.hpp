#ifndef _CASSETID
#define _CASSETID

#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"
#include "rstl/types.h"

#include "rstl/construct.hpp"
#include "rstl/string.hpp"

class CInputStream;
class COutputStream;

class CAssetId {
public:
  CAssetId() {}
  explicit CAssetId(unsigned long long id) : mId(id) {}
  explicit CAssetId(CInputStream& in);
  explicit CAssetId(const char* str);

  rstl::string ToHexString() const; // Guessed name
  void PutTo(COutputStream& out) const;
  unsigned long long Value() const { return mId; } // Guessed name

  bool operator==(CAssetId other) const { return mId == other.mId; }
  bool operator!=(CAssetId other) const { return mId != other.mId; }
  bool operator<(CAssetId other) const { return mId < other.mId; }
  bool operator>(CAssetId other) const { return mId > other.mId; }

private:
  static long long ParseDecimalString(const char* str); // Guessed name

  unsigned long long mId;
};
CHECK_SIZEOF(CAssetId, 0x8)

namespace rstl {
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(CAssetId)
} // namespace rstl

extern const CAssetId kInvalidAssetId; // Guessed name

#endif // _CASSETID
