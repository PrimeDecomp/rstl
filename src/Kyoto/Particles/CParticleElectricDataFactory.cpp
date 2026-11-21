#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/RstlVersions.h"
#endif
#include "Kyoto/Particles/CParticleElectricDataFactory.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/Streams/CMemoryInStream.hpp"
#endif

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/CVParamTransfer.hpp"

#include "Kyoto/Particles/CElectricDescription.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/Particles/CIntElement.hpp"
#endif
#include "Kyoto/Particles/CParticleDataFactory.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

#if !RSTL_CORRUPTION
RSTL_PRIME1_CONST CFactoryFnReturn
FParticleElectricDataFactory(const SObjectTag& tag, CInputStream& in,
                             const CVParamTransfer& transfer) {
#endif
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_CORRUPTION
#if RSTL_PRIME1
  rstl::rc_ptr< IVParamObj > obj = transfer.mObj;
#else
  rstl::rc_ptr< IVParamObj > obj = transfer.GetObj();
#endif
#endif

#if RSTL_CORRUPTION
CFactoryFnReturn FParticleElectricDataFactory(const SObjectTag& tag,
                                              const rstl::auto_ptr< uchar >& buffer, int size,
                                              const CVParamTransfer& transfer) {
  CMemoryInStream in(buffer.get(), size);
  rstl::rc_ptr< IVParamObj > obj = transfer.GetObj();
#endif
  CSimplePool* pool = static_cast< TObjOwnerParam< CSimplePool* >* >(obj.GetPtr())->GetData();
  CElectricDescription* desc = CParticleElectricDataFactory::GetGeneratorDesc(in, pool);
  return desc;
}

CElectricDescription* CParticleElectricDataFactory::GetGeneratorDesc(CInputStream& in,
                                                                     CSimplePool* pool) {
#if RSTL_CORRUPTION
  rstl::vector< uint > resources;
#else
  rstl::vector< CAssetId > resources;
#endif
  return CreateGeneratorDescription(in, pool);
}

CElectricDescription* CParticleElectricDataFactory::CreateGeneratorDescription(CInputStream& in,
                                                                               CSimplePool* pool) {
  const FourCC classId = CParticleDataFactory::GetClassID(in);
  if (classId != 'ELSM') {
    return nullptr;
  }
#if RSTL_CORRUPTION
  CElectricDescription* desc = rs_new_line(90) CElectricDescription();
#else
  CElectricDescription* desc = rs_new CElectricDescription();
#endif
  CreateELSM(desc, in, pool);
  LoadELSMTokens(desc);
  return desc;
}

void CParticleElectricDataFactory::LoadELSMTokens(CElectricDescription* desc) {
  if (desc->mSSWH) {
    desc->mSSWH->ForceCache();
  }
  if (desc->mGPSM) {
    desc->mGPSM->ForceCache();
  }
  if (desc->mEPSM) {
    desc->mEPSM->ForceCache();
  }
}

bool CParticleElectricDataFactory::CreateELSM(CElectricDescription* desc, CInputStream& in,
                                              CSimplePool* pool) {
  bool done = false;
  CRandom16 random(99);
  while (!done) {
    CGlobalRandom globalRandom(random);
    const FourCC classId = CParticleDataFactory::GetClassID(in);
    switch (classId) {
    case 'LIFE':
      desc->mLIFE = CParticleDataFactory::GetIntElement(in);
      break;
    case 'SLIF':
      desc->mSLIF = CParticleDataFactory::GetIntElement(in);
      break;
    case 'GRAT':
      desc->mGRAT = CParticleDataFactory::GetRealElement(in);
      break;
    case 'SCNT':
      desc->mSCNT = CParticleDataFactory::GetIntElement(in);
      break;
    case 'SSEG':
      desc->mSSEG = CParticleDataFactory::GetIntElement(in);
      break;
    case 'COLR':
      desc->mCOLR = CParticleDataFactory::GetColorElement(in);
      break;
    case 'IEMT':
      desc->mIEMT = CParticleDataFactory::GetEmitterElement(in);
      break;
    case 'FEMT':
      desc->mFEMT = CParticleDataFactory::GetEmitterElement(in);
      break;
    case 'AMPL':
      desc->mAMPL = CParticleDataFactory::GetRealElement(in);
      break;
    case 'AMPD':
      desc->mAMPD = CParticleDataFactory::GetRealElement(in);
      break;
    case 'SSWH': {
      const FourCC childId = CParticleDataFactory::GetClassID(in);
      if (childId != 'NONE') {
#if RSTL_CORRUPTION
        const CAssetId id(in);
        if (id != kInvalidAssetId) {
#else
        const CAssetId id = in.ReadLong();

        if (id != 0) {
#endif
          desc->mSSWH = TCachedToken< CSwooshDescription >(pool->GetObj(SObjectTag('SWHC', id)));
        }
      }
      break;
    }
    case 'GPSM': {
      rstl::vector< CAssetId > resources;
      rstl::optional_object< TToken< CGenDescription > > child =
          CParticleDataFactory::GetChildGeneratorDesc(in, pool, resources);
      if (child) {
        desc->mGPSM = TCachedToken< CGenDescription >(*child);
      }
      break;
    }
    case 'EPSM': {
      rstl::vector< CAssetId > resources;
      rstl::optional_object< TToken< CGenDescription > > child =
          CParticleDataFactory::GetChildGeneratorDesc(in, pool, resources);
      if (child) {
        desc->mEPSM = TCachedToken< CGenDescription >(*child);
      }
      break;
    }
    case 'LWD1':
      desc->mLWD1 = CParticleDataFactory::GetRealElement(in);
      break;
    case 'LWD2':
      desc->mLWD2 = CParticleDataFactory::GetRealElement(in);
      break;
    case 'LWD3':
      desc->mLWD3 = CParticleDataFactory::GetRealElement(in);
      break;
    case 'LCL1':
      desc->mLCL1 = CParticleDataFactory::GetColorElement(in);
      break;
    case 'LCL2':
      desc->mLCL2 = CParticleDataFactory::GetColorElement(in);
      break;
    case 'LCL3':
      desc->mLCL3 = CParticleDataFactory::GetColorElement(in);
      break;
    case 'ZERY':
      desc->mZERY = CParticleDataFactory::GetBool(in);
      break;
#if RSTL_CORRUPTION
    case 'TEXR':
      desc->mTEXR = CParticleDataFactory::GetTextureElement(in, pool);
      break;
    case 'BLDM':
      desc->mBLDM = CParticleDataFactory::GetIntElement(in);
#else
#if !RSTL_PRIME1
    case 'TEXR':
      desc->mTEXR = CParticleDataFactory::GetTextureElement(in, pool);
      break;
#endif

    case '_END':
      done = true;
#endif
      break;
#if RSTL_CORRUPTION
    case 'DFLG':
      desc->mDFLG = CParticleDataFactory::GetBitflag(in);
      break;
    case '_END':
      done = true;
      break;
#else
#if !RSTL_PRIME1
    case 'DFLG':
      desc->mDFLG = CParticleDataFactory::GetBitflag(in);
      break;
#endif
#endif
    default:
      return false;
    }
  }
#if RSTL_CORRUPTION
  if (desc->mBLDM == nullptr) {
    desc->mBLDM = rs_new_line(284) CIEConstant(0);
  }
#endif
  return true;
}
