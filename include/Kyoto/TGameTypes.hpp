#ifndef _TGAMETYPES
#define _TGAMETYPES

#include "rstl/static_assert.hpp"
#include "Kyoto/TAreaId.hpp"

#include "rstl/construct.hpp"
#include "rstl/pair.hpp"
#include "rstl/types.h"

class CInputStream;
class COutputStream;

struct TEditorId;
struct TUniqueId;

struct TEditorId {
  uint value;

  TEditorId(uint value) : value(value) {}
  TEditorId(CInputStream& in);
  uint Value() const { return value & 0x3FFFFFF; }
  uint Id() const { return value & 0xffff; }
  int AreaNum() const { return (value >> 16) & 0x3ff; }
  int LayerNum() const { return (value >> 26) & 0x3f; }
  void SetAreaNum(int areaNum) { value = (value & ~0x3FF0000) | ((areaNum << 16) & 0x3FF0000); }

  void PutTo(COutputStream&) const;

  bool operator==(const TEditorId& other) const { return Value() == other.Value(); }
  bool operator!=(const TEditorId& other) const { return Value() != other.Value(); }
  bool operator<(const TEditorId& other) const { return Value() < other.Value(); }
};
CHECK_SIZEOF(TEditorId, 0x4)

struct TUniqueId {
#if RSTL_KYOTO_UINT_UNIQUE_ID
  uint value;

  explicit TUniqueId(uint packed) : value(packed) {}
  // TODO: investigate if the 2 argument constructor can be used in Corruption

  int Value() const { return value & 0xFFFF; }
#else
  ushort value;

  TUniqueId() {}
  explicit TUniqueId(ushort packed) : value(packed) {}
  TUniqueId(const ushort version, const ushort id)
  : value(id | (version << 10)) {}

  ushort Value() const { return value & 0x3FF; }
  ushort Version() const { return (value >> 10) & 0x3F; }
#endif

  bool operator==(const TUniqueId& other) const { return value == other.value; }
  bool operator!=(const TUniqueId& other) const { return value != other.value; }
  bool operator<(const TUniqueId& other) const { return value < other.value; }
};
CHECK_SIZEOF(TUniqueId, RSTL_KYOTO_UINT_UNIQUE_ID ? 0x4 : 0x2)

struct TLayerId {
  explicit TLayerId(int value) : mValue(value) {}
  int Value() const { return mValue; }

private:
  int mValue;
};
CHECK_SIZEOF(TLayerId, 0x4)

typedef ushort TSfxId;
const TSfxId InvalidSfxId = 0xFFFFu;

#if RSTL_CORRUPTION

// Every TU that includes this header has these in consecutive order. Fun!
static const TEditorId kInvalidEditorId(-1);
static const TUniqueId kInvalidUniqueId(-1);
static const TAreaId kInvalidAreaId(-1);
static const TAreaId kAreaId0(0);  // Guessed name
static const TAreaId kAreaId1(1);  // Guessed name
static const TAreaId kAreaId2(2);  // Guessed name
static const TEditorId kUnkId(-1); // Unreferenced in every G2MEAB unit seen so far

#else

extern const TAreaId kInvalidAreaId;
extern const TEditorId kInvalidEditorId;
extern const TEditorId kUnkId;
extern const TUniqueId kInvalidUniqueId;
// Guessed names
extern const uint kInvalidPlayerIndex;
extern const uint kFirstPlayerIndex; // Unreferenced in Echoes' retail DOL.

#endif

#if !RSTL_PRIME1

namespace rstl {

RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(TUniqueId)
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(TEditorId)
RSTL_DECLARE_OPTIONAL_VALUE_CONSTRUCTION(TUniqueId)
RSTL_DECLARE_VECTOR_CONSTRUCTION(TUniqueId)
RSTL_DECLARE_VECTOR_CONSTRUCTION(TEditorId)
RSTL_DECLARE_VECTOR_CONSTRUCTION(pair< uint, TEditorId >)
RSTL_DECLARE_VECTOR_RESERVE_CONSTRUCTION(pair< uint, TEditorId >)

inline void construct(void* dest, const TEditorId& src) {
  construction_policy< TEditorId, true >::construct(dest, src);
}

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

#endif // !RSTL_PRIME1

#endif // _TGAMETYPES
