#ifndef _SOBJECTTAG
#define _SOBJECTTAG

#include "rstl/RstlVersions.h"
// The prototype declares CAssetId before the stream (include order sets template emission order).
#if RSTL_KYOTO_ASSET_ID_CLASS
#include "Corruption/Kyoto/CAssetId.hpp"
#endif
#include "Kyoto/Streams/CInputStream.hpp"
#if RSTL_KYOTO_OBJECT_TAG_TRIVIAL
#include "rstl/construct.hpp"
#endif
#include "types.h"

#if !RSTL_KYOTO_ASSET_ID_CLASS
#define kInvalidAssetId 0xFFFFFFFFu

typedef uint CAssetId;
#endif
typedef uint FourCC;

struct SObjectTag {
  FourCC type;
  CAssetId id;

  SObjectTag() {}
  SObjectTag(FourCC type, CAssetId id) : type(type), id(id) {}
#if RSTL_KYOTO_OBJECT_TAG_COPY_CTOR
  SObjectTag(const SObjectTag& other) : type(other.type), id(other.id) {}
#endif
#if RSTL_KYOTO_ASSET_ID_CLASS
  SObjectTag(CInputStream& in) : type(in.ReadInt32()), id(in) {}
#elif RSTL_STREAM_READ_INT32
  SObjectTag(CInputStream& in) : type(in.ReadInt32()), id(in.ReadInt32()) {}
#else
  SObjectTag(CInputStream& in) : type(in.Get< FourCC >()), id(in.Get< CAssetId >()) {}
#endif

#if !RSTL_KYOTO_OBJECT_TAG_TRIVIAL
  SObjectTag& operator=(const SObjectTag& other) {
    type = other.type;
    id = other.id;
    return *this;
  }
#endif

  bool operator<(const SObjectTag& other) const {
    if (type == other.type) {
      return id < other.id;
    }
    return type < other.type;
  }

#if RSTL_KYOTO_CONST_SCALAR_RETURNS
  const FourCC GetType() const { return type; }
#else
  FourCC GetType() const { return type; }
#endif
  const CAssetId& GetId() const { return id; }

  static const char* Type2Text(FourCC type);
  static FourCC Text2Type(const char* text); // Guessed name
};

#if RSTL_KYOTO_OBJECT_TAG_TRIVIAL
namespace rstl {
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(SObjectTag)
} // namespace rstl
#endif

#endif // _SOBJECTTAG
