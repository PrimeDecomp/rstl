#ifndef _CSTRINGTABLE
#define _CSTRINGTABLE
#include "rstl/RstlVersions.h"

#include "rstl/static_assert.hpp"
#if !RSTL_CORRUPTION
#include "rstl/RstlVersions.h"
#include "Kyoto/ByteOrder.hpp"
#if RSTL_PRIME1
#include "stdint.h"
#endif

#endif
#include "rstl/types.h"

#include "Kyoto/SObjectTag.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/single_ptr.hpp"
#if !RSTL_CORRUPTION
#include "rstl/vector.hpp"
#endif

#if !RSTL_CORRUPTION
#include "Kyoto/CFactoryFnReturn.hpp"
#endif

class CInputStream;

class CResFactory;
class CDvdRequest;
class CStringTable {
public:
  struct SReloadData {
    uint mSize;
    rstl::single_ptr< CDvdRequest > mRequest;
    rstl::auto_ptr< uchar > mBuffer;

    SReloadData(CAssetId id, CResFactory& factory);
    ~SReloadData();
  };

  CStringTable(CInputStream& in);
#if RSTL_CORRUPTION
  ~CStringTable();
#else
#if RSTL_VERSION >= RSTL_GM8P_00
  ~CStringTable();
#endif
#endif

  void Load(CInputStream& in);
#if RSTL_CORRUPTION
  void Reload(CAssetId id, CResFactory& factory); // Guessed name
#endif

  static void SetLanguage(int language);
#if !RSTL_CORRUPTION
  void Reload(CAssetId id, CResFactory& factory);
  void TryFinishReload();
  bool IsReloading() const;
#endif

  const wchar_t* GetString(int idx) const;
#if RSTL_CORRUPTION
  const wchar_t* GetString(const char* name) const;
  int GetStringIndex(const char* name) const;
#else
  const wchar_t* GetStringByName(const char* name) const;
  int GetStringIndexByName(const char* name) const;

#endif
  int GetStringCount() const { return mStringCount; }

private:
#if RSTL_CORRUPTION
  struct SStringName {
    const char* mName;
    int mIndex;
  };

#else
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
  // Echoes STRG files can name strings; names are sorted for binary search.
  struct SStringName {
    const char* mName;
    int mIndex;
  };

#endif
#endif
  int mStringCount;
#if RSTL_CORRUPTION
  int mNameCount;
#else
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
  int mNameCount;
#endif
#endif
  rstl::single_ptr< uchar > mData;
#if RSTL_CORRUPTION
  SStringName* mNames;
  const wchar_t** mStrings;
  rstl::single_ptr< SReloadData > mReloadData;
#else
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
  SStringName* mNames;
  const wchar_t** mStrings;
#endif
#if RSTL_VERSION >= RSTL_GM8P_00
  rstl::single_ptr< SReloadData > mReloadData;
#endif
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if TARGET_LITTLE_ENDIAN || WCHAR_MAX > 0xffff
  rstl::vector< rstl::vector< wchar_t > > mNativeStrings;
#endif
#endif
#endif
};
#if RSTL_CORRUPTION
CHECK_SIZEOF(CStringTable, 0x18)
#else

CHECK_SIZEOF(CStringTable, RSTL_PRIME1 ? (RSTL_VERSION >= RSTL_GM8P_00 ? 0xc : 0x8) : 0x18)

#endif
NESTED_CHECK_SIZEOF(CStringTable, SReloadData, 0x10)

extern CStringTable* gpStringTable;
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_CORRUPTION
#if RSTL_PRIME1
const CFactoryFnReturn FStringTableFactory(const SObjectTag& tag, CInputStream& in,
                                           const CVParamTransfer& xfer);
#else
extern const FourCC skLanguages[7]; // Shared resource language codes.
#endif
#endif

#endif // _CSTRINGTABLE
