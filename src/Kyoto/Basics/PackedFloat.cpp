#include "Kyoto/Basics/PackedFloat.hpp"

#include "types.h"

extern "C" float PackedFloat_LoadFloat32(const void* data) {
  union {
    uint bits;
    float value;
  } packed;
  packed.bits = *static_cast< const uint* >(data);
  return packed.value;
}
