#ifndef _TAREAID
#define _TAREAID

#include "rstl/construct.hpp"

struct TAreaId {
  int value;

  TAreaId() : value(-1) {}
  TAreaId(int value) : value(value) {}
  int Value() const { return value; }

  bool operator==(const TAreaId& other) const { return value == other.value; }
  bool operator!=(const TAreaId& other) const { return value != other.value; }
};
CHECK_SIZEOF(TAreaId, 0x4)

namespace rstl {
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(TAreaId)
} // namespace rstl

#endif // _TAREAID
