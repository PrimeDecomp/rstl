#ifndef _CFACTORYMGR
#define _CFACTORYMGR
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"

#include "rstl/types.h"

#include "Kyoto/CFactoryFnReturn.hpp"
#include "Kyoto/IObjectStore.hpp"
#include "Kyoto/TToken.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

#include "rstl/map.hpp"

class CInputStream;
class CVParamTransfer;
class SObjectTag;

#if !RSTL_KYOTO_BUFFER_FACTORIES
typedef RSTL_PRIME1_CONST CFactoryFnReturn (*FFactoryFunc)(const SObjectTag& tag, CInputStream& in,
                                                           const CVParamTransfer& xfer);
#endif
typedef RSTL_PRIME1_CONST CFactoryFnReturn (*FMemFactoryFunc)(
    const SObjectTag& tag, const rstl::auto_ptr< uchar >& buffer, int size,
    const CVParamTransfer& xfer);



class CFactoryMgr {
public:
  #if RSTL_PRIME1
  typedef void* MemoryBufferPtr;
  #else
  typedef const rstl::auto_ptr< uchar >& MemoryBufferPtr;
  #endif

  CFactoryMgr();
  ~CFactoryMgr();

#if !RSTL_KYOTO_BUFFER_FACTORIES
  void AddFactory(FourCC type, FFactoryFunc factory);
#endif
  void AddFactory(FourCC type, FMemFactoryFunc factory);

#if !RSTL_KYOTO_BUFFER_FACTORIES
  bool CanMakeMemory(const SObjectTag& tag) const;
  rstl::auto_ptr< IObj > MakeObject(const SObjectTag& tag, CInputStream& in,
                                    const CVParamTransfer& params);
#endif
  rstl::auto_ptr< IObj > MakeObjectFromMemory(const SObjectTag& tag,
                                              MemoryBufferPtr buffer,
                                              int size, bool compressed,
                                              const CVParamTransfer& params);

  static uint FourCCToTypeIdx(uint fcc);
  static uint TypeIdxToFourCC(uint idx);

private:
#if !RSTL_KYOTO_BUFFER_FACTORIES
  rstl::map< int, FFactoryFunc > mFactories;
#endif
  rstl::map< int, FMemFactoryFunc > mMemFactories;
};
CHECK_SIZEOF(CFactoryMgr, RSTL_KYOTO_BUFFER_FACTORIES ? 0x14 : 0x28)

#if RSTL_KYOTO_BUFFER_FACTORIES
CFactoryFnReturn FStringTableFactory(const SObjectTag& tag, const rstl::auto_ptr< uchar >& buffer,
                                     int size, const CVParamTransfer& xfer);

CFactoryFnReturn FSaveWorldFactory(const SObjectTag& tag, const rstl::auto_ptr< uchar >& buffer,
                                   int size, const CVParamTransfer& xfer);

CFactoryFnReturn FDependencyGroupFactory(const SObjectTag& tag,
                                         const rstl::auto_ptr< uchar >& buffer, int size,
                                         const CVParamTransfer& xfer);
#else
RSTL_PRIME1_CONST CFactoryFnReturn FStringTableFactory(const SObjectTag& tag, CInputStream& in,
                                                       const CVParamTransfer& xfer);

RSTL_PRIME1_CONST CFactoryFnReturn FDependencyGroupFactory(const SObjectTag& tag, CInputStream& in,
                                                           const CVParamTransfer& xfer);
#endif

#endif // _CFACTORYMGR
