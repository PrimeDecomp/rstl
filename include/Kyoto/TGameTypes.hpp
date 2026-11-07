#ifndef _TGAMETYPES
#define _TGAMETYPES

#include "rstl/construct.hpp"
#include "rstl/pair.hpp"
#include "types.h"

class CInputStream;
class COutputStream;

struct TAreaId;
struct TEditorId;
struct TUniqueId;

extern const TAreaId kInvalidAreaId;
extern const TEditorId kInvalidEditorId;
extern const TEditorId kUnkId;
extern const TUniqueId kInvalidUniqueId;
// Guessed names
extern const uint kInvalidPlayerIndex;
extern const uint kFirstPlayerIndex; // Unreferenced in Echoes' retail DOL.

struct TAreaId {
  int value;

  TAreaId() : value(-1) {}
  TAreaId(int value) : value(value) {}
  int Value() const { return value; }

  bool operator==(const TAreaId& other) const { return value == other.value; }
  bool operator!=(const TAreaId& other) const { return value != other.value; }
};
CHECK_SIZEOF(TAreaId, 0x4)

struct TEditorId {
  uint value;

  TEditorId(uint value) : value(value) {}
  TEditorId(CInputStream& in);
  uint Value() const { return value & 0x3FFFFFF; }
  uint Id() const { return value & 0xffff; }
  int AreaNum() const { return (value >> 16) & 0x3ff; }
  int LayerNum() const { return (value >> 26) & 0x3f; }

  void PutTo(COutputStream&) const;

  bool operator==(const TEditorId& other) const { return Value() == other.Value(); }
  bool operator!=(const TEditorId& other) const { return Value() != other.Value(); }
  bool operator<(const TEditorId& other) const { return Value() < other.Value(); }
};
CHECK_SIZEOF(TEditorId, 0x4)

struct TUniqueId {
  ushort value;

  TUniqueId() {}
  explicit TUniqueId(ushort packed) : value(packed) {}
  TUniqueId(const ushort version, const ushort id)
  : value(id | (version << 10)) {}

  ushort Value() const { return value & 0x3FF; }
  ushort Version() const { return (value >> 10) & 0x3F; }

  bool operator==(const TUniqueId& other) const { return value == other.value; }
  bool operator!=(const TUniqueId& other) const { return value != other.value; }
  bool operator<(const TUniqueId& other) const { return value < other.value; }
  operator bool() const { return *this != kInvalidUniqueId; }

private:
};
CHECK_SIZEOF(TUniqueId, 0x2)

struct TLayerId {
  explicit TLayerId(int value) : mValue(value) {}
  int Value() const { return mValue; }

private:
  int mValue;
};
CHECK_SIZEOF(TLayerId, 0x4)

typedef ushort TSfxId;
const TSfxId InvalidSfxId = 0xFFFFu;

#if RSTL_VERSION >= RSTL_G2ME

namespace rstl {
  
RSTL_DECLARE_TRIVIALLY_DESTRUCTIBLE(TUniqueId)
RSTL_DECLARE_ASSIGNMENT_CONSTRUCTION(TUniqueId)
RSTL_DECLARE_OPTIONAL_VALUE_CONSTRUCTION(TUniqueId)

RSTL_DECLARE_TRIVIALLY_DESTRUCTIBLE(TEditorId)
RSTL_DECLARE_ASSIGNMENT_CONSTRUCTION(TEditorId)

RSTL_DECLARE_VECTOR_CONSTRUCTION(TUniqueId)
RSTL_DECLARE_VECTOR_CONSTRUCTION(TEditorId)
RSTL_DECLARE_VECTOR_CONSTRUCTION(pair< uint, TEditorId >)
RSTL_DECLARE_VECTOR_RESERVE_CONSTRUCTION(pair< uint, TEditorId >)

inline void construct(void* dest, TEditorId& src) {
  construction_policy< TEditorId, true >::construct(dest, src);
}

#if RSTL_CONSTRUCT_MUTABLE_OVERLOAD
// Mutable ID ranges copy by assignment; const ranges retain placement construction.
template <>
inline void construct< TUniqueId >(void* dest, TUniqueId& src) {
  construction_policy< TUniqueId, true >::construct(dest, src);
}
#endif

#if RSTL_RESERVED_VECTOR_TRAITS && RSTL_CONSTRUCT_MUTABLE_OVERLOAD
template < typename T >
struct reserved_vector_traits;

template <>
struct reserved_vector_traits< TUniqueId > {
  typedef const TUniqueId& fill_type;
  typedef TUniqueId push_type;
  enum { trivial_destructor = false };
  static void fill(TUniqueId* dest, int count,
                   const TUniqueId& value) {
    uninitialized_fill_n(dest, count, value);
  }
};
#endif

// The native pickup, seeker and area-damage containers use the conservative
// element policy for these combinations. Other ID pairs inherit the member traits.
template <>
struct is_trivially_destructible< pair< int, TEditorId > > {
  enum { value = false };
};

template <>
struct use_assignment_for_construction< pair< int, TEditorId > > {
  enum { value = false };
};

template <>
struct is_trivially_destructible< pair< TUniqueId, int > > {
  enum { value = false };
};

template <>
struct use_assignment_for_construction< pair< TUniqueId, int > > {
  enum { value = false };
};

template <>
struct is_trivially_destructible< pair< TUniqueId, float > > {
  enum { value = false };
};

template <>
struct use_assignment_for_construction< pair< TUniqueId, float > > {
  enum { value = false };
};
} // namespace rstl

#endif // RSTL_VERSION >= RSTL_G2ME

#endif // _TGAMETYPES
