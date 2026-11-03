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

typedef RSTL_PRIME1_CONST CFactoryFnReturn (*FFactoryFunc)(const SObjectTag& tag, CInputStream& in,
                                                           const CVParamTransfer& xfer);
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

  void AddFactory(FourCC type, FFactoryFunc factory);
  void AddFactory(FourCC type, FMemFactoryFunc factory);

  bool CanMakeMemory(const SObjectTag& tag) const;
  rstl::auto_ptr< IObj > MakeObject(const SObjectTag& tag, CInputStream& in,
                                    const CVParamTransfer& params);
  rstl::auto_ptr< IObj > MakeObjectFromMemory(const SObjectTag& tag,
                                              MemoryBufferPtr buffer,
                                              int size, bool compressed,
                                              const CVParamTransfer& params);

  static uint FourCCToTypeIdx(uint fcc);
  static uint TypeIdxToFourCC(uint idx);

private:
  rstl::map< int, FFactoryFunc > mFactories;
  rstl::map< int, FMemFactoryFunc > mMemFactories;
};
CHECK_SIZEOF(CFactoryMgr, 0x28)

RSTL_PRIME1_CONST CFactoryFnReturn FStringTableFactory(const SObjectTag& tag, CInputStream& in,
                                                       const CVParamTransfer& xfer);

RSTL_PRIME1_CONST CFactoryFnReturn FDependencyGroupFactory(const SObjectTag& tag, CInputStream& in,
                                                           const CVParamTransfer& xfer);

#endif // _CFACTORYMGR
