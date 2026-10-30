#ifndef _CPARTICLESWOOSHDATAFACTORY
#define _CPARTICLESWOOSHDATAFACTORY
#include "rstl/RstlVersions.h"
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#include "Kyoto/CFactoryFnReturn.hpp"
#include "Kyoto/Particles/CSwooshDescription.hpp"
#else
#include "Kyoto/CFactoryMgr.hpp"
#endif

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
#if RSTL_PRIME1
const CFactoryFnReturn FParticleSwooshDataFactory(const SObjectTag& tag, CInputStream& in,
#else
CFactoryFnReturn FParticleSwooshDataFactory(const SObjectTag& tag, CInputStream& in,
#endif

                                            const CVParamTransfer& transfer);

#endif
