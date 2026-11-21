#include "Kyoto/Particles/CUserEvaluatorDataFactory.hpp"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/CVParamTransfer.hpp"
#include "Kyoto/Particles/CParticleDataFactory.hpp"
#include "Kyoto/Particles/CUserEvaluatorDescription.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"



CFactoryFnReturn FUserEvaluatorDataFactory(const SObjectTag& tag,
                                           const rstl::auto_ptr< uchar >& buffer, int size,
                                           const CVParamTransfer& transfer) {
  CMemoryInStream in(buffer.get(), size);
  rstl::rc_ptr< IVParamObj > obj = transfer.GetObj();
  CSimplePool* pool = static_cast< TObjOwnerParam< CSimplePool* >* >(obj.GetPtr())->GetData();
  CUserEvaluatorDescription* desc = CUserEvaluatorDataFactory::GetDescription(in, pool);
  return desc;
}

CUserEvaluatorDescription* CUserEvaluatorDataFactory::GetDescription(CInputStream& in,
                                                                     CSimplePool* pool) {
  rstl::vector< uint > resources;
  return CreateDescription(in, pool);
}

CUserEvaluatorDescription* CUserEvaluatorDataFactory::CreateDescription(CInputStream& in,
                                                                        CSimplePool* pool) {
  const FourCC classId = CParticleDataFactory::GetClassID(in);
  if (classId != 'USER') {
    return nullptr;
  }
  CUserEvaluatorDescription* desc = rs_new_line(92) CUserEvaluatorDescription();
  ParseProperties(desc, in, pool);
  return desc;
}

bool CUserEvaluatorDataFactory::ParseProperties(CUserEvaluatorDescription* desc, CInputStream& in,
                                                CSimplePool* pool) {
  bool done = false;
  CRandom16 random(99);
  while (!done) {
    CGlobalRandom globalRandom(random);
    const FourCC classId = CParticleDataFactory::GetClassID(in);
    switch (classId) {
    case 'RPM0':
      desc->mRealElements[0] = CParticleDataFactory::GetRealElement(in);
      break;
    case 'RPM1':
      desc->mRealElements[1] = CParticleDataFactory::GetRealElement(in);
      break;
    case 'RPM2':
      desc->mRealElements[2] = CParticleDataFactory::GetRealElement(in);
      break;
    case 'RPM3':
      desc->mRealElements[3] = CParticleDataFactory::GetRealElement(in);
      break;
    case 'RPM4':
      desc->mRealElements[4] = CParticleDataFactory::GetRealElement(in);
      break;
    case 'RPM5':
      desc->mRealElements[5] = CParticleDataFactory::GetRealElement(in);
      break;
    case 'RPM6':
      desc->mRealElements[6] = CParticleDataFactory::GetRealElement(in);
      break;
    case 'RPM7':
      desc->mRealElements[7] = CParticleDataFactory::GetRealElement(in);
      break;
    case 'RPM8':
      desc->mRealElements[8] = CParticleDataFactory::GetRealElement(in);
      break;
    case 'RPM9':
      desc->mRealElements[9] = CParticleDataFactory::GetRealElement(in);
      break;
    case 'IPM0':
      desc->mIntElements[0] = CParticleDataFactory::GetIntElement(in);
      break;
    case 'IPM1':
      desc->mIntElements[1] = CParticleDataFactory::GetIntElement(in);
      break;
    case 'IPM2':
      desc->mIntElements[2] = CParticleDataFactory::GetIntElement(in);
      break;
    case 'IPM3':
      desc->mIntElements[3] = CParticleDataFactory::GetIntElement(in);
      break;
    case 'IPM4':
      desc->mIntElements[4] = CParticleDataFactory::GetIntElement(in);
      break;
    case 'CLR0':
      desc->mColorElements[0] = CParticleDataFactory::GetColorElement(in);
      break;
    case 'CLR1':
      desc->mColorElements[1] = CParticleDataFactory::GetColorElement(in);
      break;
    case 'VEC0':
      desc->mVectorElements[0] = CParticleDataFactory::GetVectorElement(in);
      break;
    case 'VEC1':
      desc->mVectorElements[1] = CParticleDataFactory::GetVectorElement(in);
      break;
    case 'VEC2':
      desc->mVectorElements[2] = CParticleDataFactory::GetVectorElement(in);
      break;
    case 'EMT0':
      desc->mEmitterElements[0] = CParticleDataFactory::GetEmitterElement(in);
      break;
    case 'EMT1':
      desc->mEmitterElements[1] = CParticleDataFactory::GetEmitterElement(in);
      break;
    case 'MVC0':
      desc->mModVectorElements[0] = CParticleDataFactory::GetModVectorElement(in);
      break;
    case 'MVC1':
      desc->mModVectorElements[1] = CParticleDataFactory::GetModVectorElement(in);
      break;
    case 'TEX0':
      desc->mTextureElements[0] = CParticleDataFactory::GetTextureElement(in, pool);
      break;
    case 'TEX1':
      desc->mTextureElements[1] = CParticleDataFactory::GetTextureElement(in, pool);
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
