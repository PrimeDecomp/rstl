#ifndef _CPARTICLESPAWNSYSTEMDATAFACTORY
#define _CPARTICLESPAWNSYSTEMDATAFACTORY
#include "rstl/RstlVersions.h"

#if !RSTL_CORRUPTION
#include "rstl/types.h"
#endif
#include "Kyoto/CFactoryMgr.hpp"
#if RSTL_CORRUPTION
#include "rstl/types.h"
#endif

#if !RSTL_CORRUPTION
class CInputStream;
#endif

class CSpawnSystemDescription;
#if RSTL_CORRUPTION
class CInputStream;
#endif
class CSimplePool;
class CVParamTransfer;

// Reconstructed class/method names follow the sibling particle factories; no original export is known.
class CParticleSpawnSystemDataFactory {
public:
  static CSpawnSystemDescription* GetGeneratorDesc(CInputStream& in, CSimplePool* pool);

private:
  static CSpawnSystemDescription* CreateGeneratorDescription(CInputStream& in, CSimplePool* pool);
  static bool CreateSPSM(CSpawnSystemDescription* desc, CInputStream& in, CSimplePool* pool);
};

// Guessed free-factory name, supported by the SPSC registration and parsed descriptor type.
#if RSTL_CORRUPTION
CFactoryFnReturn FSpawnParticleSystemDataFactory(const SObjectTag& tag,
                                                 const rstl::auto_ptr< uchar >& buffer, int size,
#else
CFactoryFnReturn FSpawnParticleSystemDataFactory(const SObjectTag& tag, CInputStream& in,
#endif
                                                const CVParamTransfer& transfer);

#endif // _CPARTICLESPAWNSYSTEMDATAFACTORY
