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

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
const CFactoryFnReturn FParticleElectricDataFactory(const SObjectTag& tag, CInputStream& in,
#else
CFactoryFnReturn FParticleElectricDataFactory(const SObjectTag& tag, CInputStream& in,
#endif

                                              const CVParamTransfer& transfer);

#endif // _CPARTICLEELECTRICDATAFACTORY
