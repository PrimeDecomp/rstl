#ifndef _CPARTICLEELECTRICDATAFACTORY
#define _CPARTICLEELECTRICDATAFACTORY
#include "rstl/RstlVersions.h"

#include "Kyoto/CFactoryFnReturn.hpp"
#include "Kyoto/CFactoryMgr.hpp"

class CInputStream;

class CElectricDescription;
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
