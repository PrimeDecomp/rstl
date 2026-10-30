#ifndef _CFACTORYMGR
#define _CFACTORYMGR
#include "rstl/RstlVersions.h"

#include "types.h"

#include "Kyoto/CFactoryFnReturn.hpp"
#include "Kyoto/IObjectStore.hpp"
#include "Kyoto/TToken.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

#include "rstl/map.hpp"

class CInputStream;
class CVParamTransfer;
class SObjectTag;

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
typedef const CFactoryFnReturn (*FFactoryFunc)(const SObjectTag&, CInputStream&,
                                               const CVParamTransfer&);
typedef const CFactoryFnReturn (*FMemFactoryFunc)(const SObjectTag&, const rstl::auto_ptr< uchar >&,
                                                  int, const CVParamTransfer&);
#else
typedef CFactoryFnReturn (*FFactoryFunc)(const SObjectTag& tag, CInputStream& in,
                                         const CVParamTransfer& xfer);
typedef CFactoryFnReturn (*FMemFactoryFunc)(const SObjectTag& tag,
                                            const rstl::auto_ptr< uchar >& buffer, int size,
                                            const CVParamTransfer& xfer);
#endif

class CFactoryMgr {
public:
  CFactoryMgr();
  ~CFactoryMgr();

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  void AddFactory(uint type, FFactoryFunc factory);
  void AddFactory(uint type, FMemFactoryFunc factory);
#else
  void AddFactory(FourCC type, FFactoryFunc factory);
  void AddFactory(FourCC type, FMemFactoryFunc factory);
#endif

  bool CanMakeMemory(const SObjectTag& tag) const;
  rstl::auto_ptr< IObj > MakeObject(const SObjectTag& tag, CInputStream& in,
                                    const CVParamTransfer& params);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  rstl::auto_ptr< IObj > MakeObjectFromMemory(const SObjectTag& tag, void* buffer, int size,
#else
  rstl::auto_ptr< IObj > MakeObjectFromMemory(const SObjectTag& tag,
                                              const rstl::auto_ptr< uchar >& buffer, int size,
#endif

                                              bool compressed, const CVParamTransfer& params);

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static uint FourCCToTypeIdx(uint fcc);
  static uint TypeIdxToFourCC(uint idx);
#else
  static uint FourCCToTypeIdx(uint fourCC);
  static uint TypeIdxToFourCC(uint typeIdx);
#endif

private:
  rstl::map< int, FFactoryFunc > mFactories;
  rstl::map< int, FMemFactoryFunc > mMemFactories;
};
CHECK_SIZEOF(CFactoryMgr, 0x28)

#if !RSTL_PRIME1
CFactoryFnReturn FStringTableFactory(const SObjectTag& tag, CInputStream& in,
                                     const CVParamTransfer& xfer);
#endif

#if !RSTL_PRIME1
CFactoryFnReturn FDependencyGroupFactory(const SObjectTag& tag, CInputStream& in,
                                         const CVParamTransfer& xfer);
#endif

#endif // _CFACTORYMGR
