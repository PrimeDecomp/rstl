#include "rstl/RstlVersions.h"
#include "Kyoto/Particles/CParticleSortedSystemDataFactory.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/Streams/CMemoryInStream.hpp"
#endif

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/CVParamTransfer.hpp"
#include "Kyoto/Particles/CParticleDataFactory.hpp"
#include "Kyoto/Particles/CSortedParticleSystemDescription.hpp"
#include "Kyoto/Particles/CSpawnSystemKeyframeData.hpp"

#if RSTL_CORRUPTION


CFactoryFnReturn FSortedParticleSystemDataFactory(const SObjectTag& tag,
                                                  const rstl::auto_ptr< uchar >& buffer, int size,
#else
CFactoryFnReturn FSortedParticleSystemDataFactory(const SObjectTag& tag, CInputStream& in,
#endif
                                                  const CVParamTransfer& transfer) {
#if RSTL_CORRUPTION
  CMemoryInStream in(buffer.get(), size);
#endif
  rstl::rc_ptr< IVParamObj > obj = transfer.GetObj();
  CSimplePool* pool = static_cast< TObjOwnerParam< CSimplePool* >* >(obj.GetPtr())->GetData();
  CSortedParticleSystemDescription* desc =
      CParticleSortedSystemDataFactory::GetGeneratorDesc(in, pool);
  return desc;
}

#if !RSTL_CORRUPTION
// Guessed; stands in for a function stripped from the link.
static void StrippedDestroy(TObjOwnerDerivedFromIObj< CSortedParticleSystemDescription >* obj) {
  delete obj;
}

#endif
CSortedParticleSystemDescription*
CParticleSortedSystemDataFactory::GetGeneratorDesc(CInputStream& in, CSimplePool* pool) {
#if RSTL_CORRUPTION
  rstl::vector< uint > resources;
#else
  rstl::vector< CAssetId > resources;
#endif
  return CreateGeneratorDescription(in, pool);
}

CSortedParticleSystemDescription*
CParticleSortedSystemDataFactory::CreateGeneratorDescription(CInputStream& in, CSimplePool* pool) {
  const FourCC classId = CParticleDataFactory::GetClassID(in);
  if (classId != 'SRSM') {
    return nullptr;
  }
#if RSTL_CORRUPTION
  CSortedParticleSystemDescription* desc = rs_new_line(82) CSortedParticleSystemDescription();
#else
  CSortedParticleSystemDescription* desc = rs_new CSortedParticleSystemDescription();
#endif
  CreateSRSM(desc, in, pool);
  return desc;
}

bool CParticleSortedSystemDataFactory::CreateSRSM(CSortedParticleSystemDescription* desc,
                                                  CInputStream& in, CSimplePool* pool) {
  bool done = false;
  CRandom16 random(99);
  while (!done) {
    CGlobalRandom globalRandom(random);
    const FourCC classId = CParticleDataFactory::GetClassID(in);
    switch (classId) {
    case 'SPWN': {
      const FourCC childId = CParticleDataFactory::GetClassID(in);
      if (childId == 'CNST') {
#if RSTL_CORRUPTION
        desc->mSPWN = rs_new_line(107) CSpawnSystemKeyframeData(in);
#else
        desc->mSPWN = rs_new CSpawnSystemKeyframeData(in);
#endif
        desc->mSPWN->LoadAllSpawnedSystemTokens(pool);
      }
      break;
    }
    case '_END':
      done = true;
      break;
    default:
      return false;
    }
  }
  return true;
}
