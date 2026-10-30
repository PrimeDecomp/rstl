#ifndef _CSEGID
#define _CSEGID
#include "rstl/RstlVersions.h"

#include "types.h"

#include "Kyoto/Streams/CInputStream.hpp"

class CSegId {
public:
#if !RSTL_PRIME1
  CSegId() : mId(-1) {}
#endif

  CSegId(int id) : mId(id) {}
#if !RSTL_PRIME1
  CSegId(uint id) : mId(id) {}
#endif

  CSegId(const char& id) : mId(id) {}
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CSegId(uint id) : mId(id) {}
  CSegId(CInputStream& in) : mId(in.Get< uint >()) {}
#else
  CSegId(CInputStream& in);

  static CSegId Null() { return CSegId(99); }

  static CSegId Character() { return CSegId(97); }

  static CSegId Invalid() { return CSegId(255); }

#if RSTL_VERSION >= RSTL_R32J
  bool operator==(CSegId other) const { return mId == other.mId; }
#else
  bool operator==(const CSegId& other) const { return mId == other.mId; }
#endif

  bool operator!=(const CSegId& other) const { return mId != other.mId; }
#endif

  const uchar val() const { return mId; }
#if RSTL_PRIME1
  static CSegId Null() { return CSegId(0); }
  static CSegId Invalid() { return CSegId(-1); }
  static CSegId Root() { return CSegId(3); }
  static CSegId Character() { return CSegId(2); }

  bool operator==(const CSegId& rhs) const { return mId == rhs.mId; }
  bool operator!=(const CSegId& rhs) const { return mId != rhs.mId; }
#endif

private:
  uchar mId;
};
#if !RSTL_PRIME1
CHECK_SIZEOF(CSegId, 0x1)

namespace rstl {
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(CSegId)
} // namespace rstl
namespace rstl {
RSTL_DECLARE_DIRECT_CONSTRUCTION(CSegId)
} // namespace rstl
#endif

#endif // _CSEGID
