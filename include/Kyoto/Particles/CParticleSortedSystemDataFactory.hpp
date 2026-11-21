#ifndef _CPARTICLESORTEDSYSTEMDATAFACTORY
#define _CPARTICLESORTEDSYSTEMDATAFACTORY
#include "rstl/RstlVersions.h"

#include "Kyoto/CFactoryMgr.hpp"
#include "rstl/types.h"

#if !RSTL_CORRUPTION
class CInputStream;
#endif

class CSortedParticleSystemDescription;
#if RSTL_CORRUPTION
class CInputStream;
#endif
class CSimplePool;
class CVParamTransfer;

// Guessed name; class/method spellings follow sibling factories, with no original export known.
class CParticleSortedSystemDataFactory {
public:
  static CSortedParticleSystemDescription* GetGeneratorDesc(CInputStream& in, CSimplePool* pool);

private:
  static CSortedParticleSystemDescription* CreateGeneratorDescription(CInputStream& in,
                                                                      CSimplePool* pool);
  static bool CreateSRSM(CSortedParticleSystemDescription* desc, CInputStream& in,
                         CSimplePool* pool);
};

// Guessed free-factory name, supported by the SRSC registration and parsed descriptor type.
#if RSTL_CORRUPTION
CFactoryFnReturn FSortedParticleSystemDataFactory(const SObjectTag& tag,
                                                  const rstl::auto_ptr< uchar >& buffer, int size,
#else
CFactoryFnReturn FSortedParticleSystemDataFactory(const SObjectTag& tag, CInputStream& in,
#endif
                                                  const CVParamTransfer& transfer);

#endif // _CPARTICLESORTEDSYSTEMDATAFACTORY
