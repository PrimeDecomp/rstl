#ifndef _CPARTICLEELECTRICDATAFACTORY
#define _CPARTICLEELECTRICDATAFACTORY
#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/RstlVersions.h"
#endif

#if !RSTL_CORRUPTION
#include "Kyoto/CFactoryFnReturn.hpp"
#endif
#include "Kyoto/CFactoryMgr.hpp"

#if !RSTL_CORRUPTION
class CInputStream;
#endif

class CElectricDescription;
#if RSTL_CORRUPTION
class CInputStream;
#endif
class CSimplePool;

class CParticleElectricDataFactory {
public:
  static CElectricDescription* GetGeneratorDesc(CInputStream& in, CSimplePool* pool);

private:
  static CElectricDescription* CreateGeneratorDescription(CInputStream& in, CSimplePool* pool);
  static void LoadELSMTokens(CElectricDescription* desc);
  static bool CreateELSM(CElectricDescription* desc, CInputStream& in, CSimplePool* pool);
};

#if RSTL_CORRUPTION
CFactoryFnReturn FParticleElectricDataFactory(const SObjectTag& tag,
                                              const rstl::auto_ptr< uchar >& buffer, int size,
#else
RSTL_PRIME1_CONST CFactoryFnReturn FParticleElectricDataFactory(const SObjectTag& tag, CInputStream& in,

#endif
                                              const CVParamTransfer& transfer);

#endif // _CPARTICLEELECTRICDATAFACTORY
