#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Particles/CBurstFireDataFactory.hpp"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/CVParamTransfer.hpp"
#include "Kyoto/Particles/CBurstFireDescription.hpp"
#include "Kyoto/Particles/CParticleDataFactory.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"



CFactoryFnReturn FBurstFireDataFactory(const SObjectTag& tag, const rstl::auto_ptr< uchar >& buffer,
                                       int size, const CVParamTransfer& transfer) {
  CMemoryInStream in(buffer.get(), size);
  rstl::rc_ptr< IVParamObj > obj = transfer.GetObj();
  CSimplePool* pool = static_cast< TObjOwnerParam< CSimplePool* >* >(obj.GetPtr())->GetData();
  CBurstFireDescription* desc = CBurstFireDataFactory::GetDescription(in, pool);
  return desc;
}

CBurstFireDescription* CBurstFireDataFactory::GetDescription(CInputStream& in, CSimplePool* pool) {
  rstl::vector< uint > resources;
  return CreateDescription(in, pool);
}

CBurstFireDescription* CBurstFireDataFactory::CreateDescription(CInputStream& in,
                                                                CSimplePool* pool) {
  const FourCC classId = CParticleDataFactory::GetClassID(in);
  if (classId != 'BFRE') {
    return nullptr;
  }
  CBurstFireDescription* desc = rs_new_line(93) CBurstFireDescription();
  ParseProperties(desc, in, pool);
  return desc;
}

bool CBurstFireDataFactory::ParseProperties(CBurstFireDescription* desc, CInputStream& in,
                                            CSimplePool* pool) {
  bool done = false;
  CRandom16 random(99);
  while (!done) {
    CGlobalRandom globalRandom(random);
    const FourCC classId = CParticleDataFactory::GetClassID(in);
    switch (classId) {
    case 'OVLH':
      desc->mOVLH = CParticleDataFactory::GetRealElement(in);
      break;
    case 'OVLW':
      desc->mOVLW = CParticleDataFactory::GetRealElement(in);
      break;
    case 'SHVR':
      desc->mSHVR = CParticleDataFactory::GetRealElement(in);
      break;
    case 'BTMI':
      desc->mBTMI = CParticleDataFactory::GetIntElement(in);
      break;
    case 'B0CH':
      desc->mBurstCH[0] = CParticleDataFactory::GetRealElement(in);
      break;
    case 'B0VL':
      desc->mBurstVL[0] = CParticleDataFactory::GetRealElement(in);
      break;
    case 'B1CH':
      desc->mBurstCH[1] = CParticleDataFactory::GetRealElement(in);
      break;
    case 'B1VL':
      desc->mBurstVL[1] = CParticleDataFactory::GetRealElement(in);
      break;
    case 'B2CH':
      desc->mBurstCH[2] = CParticleDataFactory::GetRealElement(in);
      break;
    case 'B2VL':
      desc->mBurstVL[2] = CParticleDataFactory::GetRealElement(in);
      break;
    case 'B3CH':
      desc->mBurstCH[3] = CParticleDataFactory::GetRealElement(in);
      break;
    case 'B3VL':
      desc->mBurstVL[3] = CParticleDataFactory::GetRealElement(in);
      break;
    case 'B4CH':
      desc->mBurstCH[4] = CParticleDataFactory::GetRealElement(in);
      break;
    case 'B4VL':
      desc->mBurstVL[4] = CParticleDataFactory::GetRealElement(in);
      break;
    case 'B5CH':
      desc->mBurstCH[5] = CParticleDataFactory::GetRealElement(in);
      break;
    case 'B5VL':
      desc->mBurstVL[5] = CParticleDataFactory::GetRealElement(in);
      break;
    case 'B6CH':
      desc->mBurstCH[6] = CParticleDataFactory::GetRealElement(in);
      break;
    case 'B6VL':
      desc->mBurstVL[6] = CParticleDataFactory::GetRealElement(in);
      break;
    case 'B7CH':
      desc->mBurstCH[7] = CParticleDataFactory::GetRealElement(in);
      break;
    case 'B7VL':
      desc->mBurstVL[7] = CParticleDataFactory::GetRealElement(in);
      break;
    case '_END':
      done = true;
      break;
    default:
      return false;
    }
  }
  return true;
}
