#ifndef _CSTRINGTABLE
#define _CSTRINGTABLE

#include "Kyoto/ByteOrder.hpp"
#include "Kyoto/CFactoryFnReturn.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "rstl/RstlVersions.h"
#include "rstl/auto_ptr.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/static_assert.hpp"
#include "rstl/types.h"
#include "rstl/vector.hpp"

#if TARGET_LITTLE_ENDIAN || WCHAR_MAX > 0xffff
#define STRINGTABLE_NONMATCHING 1
#else
#define STRINGTABLE_NONMATCHING 0
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
#if RSTL_CSTRINGTABLE_RELOAD_SUPPORT
  ~CStringTable();
#endif

  void Load(CInputStream& in);

  static void SetLanguage(int language);
  void Reload(CAssetId id, CResFactory& factory);
  void TryFinishReload();
  bool IsReloading() const;

  const wchar_t* GetString(int idx) const;
  const wchar_t* GetStringByName(const char* name) const;
  int GetStringIndexByName(const char* name) const;

  int GetStringCount() const { return mStringCount; }

private:
  void LoadStringsNative(CInputStream& in, uint size, uint version);

  struct SStringName {
    const char* mName;
    int mIndex;
  };

  int mStringCount;
#if RSTL_CSTRINGTABLE_NAME_TABLES
  int mNameCount;
#endif
  rstl::single_ptr< uchar > mData;
#if RSTL_CSTRINGTABLE_NAME_TABLES
  SStringName* mNames;
  const wchar_t** mStrings;
#endif
#if RSTL_CSTRINGTABLE_RELOAD_SUPPORT
  mutable rstl::single_ptr< SReloadData > mReloadData;
#endif
#if STRINGTABLE_NONMATCHING
  rstl::vector< SStringName > mNativeNames;
  rstl::vector< rstl::vector< wchar_t > > mNativeStrings;
#endif
};

CHECK_SIZEOF(CStringTable, RSTL_PRIME1 ? (RSTL_CSTRINGTABLE_RELOAD_SUPPORT ? 0xc : 0x8) : 0x18)
NESTED_CHECK_SIZEOF(CStringTable, SReloadData, 0x10)

extern CStringTable* gpStringTable;

#if RSTL_KYOTO_BUFFER_FACTORIES
CFactoryFnReturn FStringTableFactory(const SObjectTag& tag, const rstl::auto_ptr< uchar >& buffer,
                                     int size, const CVParamTransfer& xfer);
#else
RSTL_PRIME1_CONST CFactoryFnReturn FStringTableFactory(const SObjectTag& tag, CInputStream& in,
                                                       const CVParamTransfer& xfer);
#endif
extern const FourCC skLanguages[7]; // Shared resource language codes.

#endif // _CSTRINGTABLE
