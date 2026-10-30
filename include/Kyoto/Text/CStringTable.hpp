#ifndef _CSTRINGTABLE
#define _CSTRINGTABLE
#include "rstl/RstlVersions.h"
#if RSTL_PRIME1
#include "stdint.h"
#endif

#include "types.h"

#if !RSTL_PRIME1
#include "Kyoto/SObjectTag.hpp"
#endif

#include "rstl/auto_ptr.hpp"
#include "rstl/single_ptr.hpp"
#if RSTL_PRIME1
#if TARGET_LITTLE_ENDIAN || WCHAR_MAX > 0xffff
#include "rstl/vector.hpp"
#endif
#endif

#if RSTL_PRIME1
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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_GM8P_00
  ~CStringTable();
#endif
#else
  ~CStringTable();
#endif

  void Load(CInputStream& in);

  static void SetLanguage(int language);
#if RSTL_PRIME1
  void Reload(CAssetId id, CResFactory& factory);
  void TryFinishReload();
  bool IsReloading() const;
#endif

  const wchar_t* GetString(int idx) const;
#if !RSTL_PRIME1
  const wchar_t* GetStringByName(const char* name) const;
  int GetStringIndexByName(const char* name) const;
#endif

  int GetStringCount() const { return mStringCount; }

private:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  int mStringCount;
  rstl::single_ptr< uchar > mData;
#if RSTL_VERSION >= RSTL_GM8P_00
  rstl::single_ptr< SReloadData > mReloadData;
#endif
#if TARGET_LITTLE_ENDIAN || WCHAR_MAX > 0xffff
  rstl::vector< rstl::vector< wchar_t > > mNativeStrings;
#endif
#else
  // Echoes STRG files can name strings; names are sorted for binary search.
  struct SStringName {
    const char* mName;
    int mIndex;
#endif

};

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_GM8P_00
CHECK_SIZEOF(CStringTable, 0xc)
#else
CHECK_SIZEOF(CStringTable, 0x8)
#endif
#else
  int mStringCount;
  int mNameCount;
  rstl::single_ptr< uchar > mData;
  SStringName* mNames;
  const wchar_t** mStrings;
  rstl::single_ptr< SReloadData > mReloadData;
};
CHECK_SIZEOF(CStringTable, 0x18)
#endif

NESTED_CHECK_SIZEOF(CStringTable, SReloadData, 0x10)

extern CStringTable* gpStringTable;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
const CFactoryFnReturn FStringTableFactory(const SObjectTag& tag, CInputStream& in,
                                     const CVParamTransfer& xfer);
#else
extern const FourCC skLanguages[7]; // Shared resource language codes.
#endif

#endif // _CSTRINGTABLE
