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
  CSegId(uint id) : mId(id) {}
  CSegId(const char& id) : mId(id) {}
#if RSTL_PRIME1
  CSegId(CInputStream& in) : mId(in.Get< uint >()) {}
#else
  CSegId(CInputStream& in);
#endif

  const uchar val() const { return mId; }

  static CSegId Null() {
#if RSTL_PRIME1
    return CSegId(0);
#else
    return CSegId(99);
#endif
  }
  static CSegId Invalid() {
#if RSTL_PRIME1
    return CSegId(-1);
#else
    return CSegId(255);
#endif
  }
#if RSTL_PRIME1
  static CSegId Root() { return CSegId(3); }
#endif
  static CSegId Character() {
#if RSTL_PRIME1
    return CSegId(2);
#else
    return CSegId(97);
#endif
  }

#if !RSTL_PRIME1 && RSTL_VERSION >= RSTL_R32J
  bool operator==(CSegId other) const { return mId == other.mId; }
#else
  bool operator==(const CSegId& other) const { return mId == other.mId; }
#endif
  bool operator!=(const CSegId& other) const { return mId != other.mId; }

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
