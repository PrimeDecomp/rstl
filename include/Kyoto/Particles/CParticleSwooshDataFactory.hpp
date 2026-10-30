#ifndef _CPARTICLESWOOSHDATAFACTORY
#define _CPARTICLESWOOSHDATAFACTORY
#include "rstl/RstlVersions.h"
#include "Kyoto/CFactoryFnReturn.hpp"
#include "Kyoto/Particles/CSwooshDescription.hpp"
#include "Kyoto/CFactoryMgr.hpp"

class CInputStream;

class CSimplePool;
#if !RSTL_PRIME1
class CSwooshDescription;
#endif

class CParticleSwooshDataFactory {
public:
  static CSwooshDescription* GetGeneratorDesc(CInputStream& in, CSimplePool* pool);
#if !RSTL_PRIME1
private:
#endif

  static CSwooshDescription* CreateGeneratorDescription(CInputStream& in, CSimplePool* pool);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static bool CreateWPSM(CSwooshDescription* swoosh, CInputStream& in, CSimplePool* pool);
#else
  static bool CreateWPSM(CSwooshDescription* desc, CInputStream& in, CSimplePool* pool);
#endif

};

// TODO: verify that this Prime/Echoes difference is real
RSTL_PRIME1_CONST CFactoryFnReturn FParticleSwooshDataFactory(const SObjectTag& tag, CInputStream& in,

                                            const CVParamTransfer& transfer);

#endif
