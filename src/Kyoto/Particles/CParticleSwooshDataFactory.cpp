#include "rstl/RstlVersions.h"
#include "Kyoto/Particles/CParticleSwooshDataFactory.hpp"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/CVParamTransfer.hpp"

#include "Kyoto/Particles/CParticleDataFactory.hpp"
#include "Kyoto/Particles/CSwooshDescription.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

// TODO: verify that this Prime/Echoes difference is real
RSTL_PRIME1_CONST CFactoryFnReturn FParticleSwooshDataFactory(const SObjectTag& tag, CInputStream& in,

                                            const CVParamTransfer& transfer) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  rstl::rc_ptr< IVParamObj > obj = transfer.mObj;
#else
  rstl::rc_ptr< IVParamObj > obj = transfer.GetObj();
#endif

  CSimplePool* pool = static_cast< TObjOwnerParam< CSimplePool* >* >(obj.GetPtr())->GetData();
  CSwooshDescription* desc = CParticleSwooshDataFactory::GetGeneratorDesc(in, pool);
  return desc;
}

// Force function ordering
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
static void hack(TObjOwnerDerivedFromIObj< CSwooshDescription >* obj) {
  obj->~TObjOwnerDerivedFromIObj< CSwooshDescription >();
}
#else
static void hack() { TObjOwnerDerivedFromIObj< CSwooshDescription >::GetNewDerivedObject(nullptr); }
#endif

CSwooshDescription* CParticleSwooshDataFactory::GetGeneratorDesc(CInputStream& in,
                                                                 CSimplePool* pool) {
  rstl::vector< CAssetId > ids;
  return CreateGeneratorDescription(in, pool);
}

CSwooshDescription* CParticleSwooshDataFactory::CreateGeneratorDescription(CInputStream& in,
                                                                           CSimplePool* pool) {
  const FourCC classId = CParticleDataFactory::GetClassID(in);
  if (classId != 'SWSH') {
    return nullptr;
  }
  CSwooshDescription* desc = rs_new CSwooshDescription();

  CreateWPSM(desc, in, pool);

  return desc;
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
// This is actually what it was named, must be a copy+paste job
bool CParticleSwooshDataFactory::CreateWPSM(CSwooshDescription* swoosh, CInputStream& in,
#else
bool CParticleSwooshDataFactory::CreateWPSM(CSwooshDescription* desc, CInputStream& in,
#endif

                                            CSimplePool* pool) {
  bool done = false;
  CRandom16 random(99);
  CGlobalRandom globalRandom(random);

  while (!done) {
    const FourCC classId = CParticleDataFactory::GetClassID(in);
    switch (classId) {
    case 'PSLT':
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      swoosh->mPSLT = CParticleDataFactory::GetIntElement(in);
#else
      desc->mPSLT = CParticleDataFactory::GetIntElement(in);
#endif

      break;
    case 'TIME':
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      swoosh->mTIME = CParticleDataFactory::GetRealElement(in);
#else
      desc->mTIME = CParticleDataFactory::GetRealElement(in);
#endif

      break;
    case 'LRAD':
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      swoosh->mLRAD = CParticleDataFactory::GetRealElement(in);
#else
      desc->mLRAD = CParticleDataFactory::GetRealElement(in);
#endif

      break;
    case 'RRAD':
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      swoosh->mRRAD = CParticleDataFactory::GetRealElement(in);
#else
      desc->mRRAD = CParticleDataFactory::GetRealElement(in);
#endif

      break;
    case 'LLRD':
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      swoosh->mLLRD = CParticleDataFactory::GetBool(in);
#else
      desc->mLLRD = CParticleDataFactory::GetBool(in);
#endif

      break;
    case 'LENG':
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      swoosh->mLENG = CParticleDataFactory::GetIntElement(in);
#else
      desc->mLENG = CParticleDataFactory::GetIntElement(in);
#endif

      break;
    case 'COLR':
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      swoosh->mCOLR = CParticleDataFactory::GetColorElement(in);
#else
      desc->mCOLR = CParticleDataFactory::GetColorElement(in);
#endif

      break;
    case 'SIDE':
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      swoosh->mSIDE = CParticleDataFactory::GetIntElement(in);
#else
      desc->mSIDE = CParticleDataFactory::GetIntElement(in);
#endif

      break;
    case 'CROS':
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      swoosh->mCROS = CParticleDataFactory::GetBool(in);
#else
      desc->mCROS = CParticleDataFactory::GetBool(in);
#endif

      break;
    case 'SROT':
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      swoosh->mSROT = CParticleDataFactory::GetBool(in);
#else
      desc->mSROT = CParticleDataFactory::GetBool(in);
#endif

      break;
    case 'IROT':
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      swoosh->mIROT = CParticleDataFactory::GetRealElement(in);
#else
      desc->mIROT = CParticleDataFactory::GetRealElement(in);
#endif

      break;
    case 'ROTM':
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      swoosh->mROTM = CParticleDataFactory::GetRealElement(in);
#else
      desc->mROTM = CParticleDataFactory::GetRealElement(in);
#endif

      break;
    case 'POFS':
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      swoosh->mPOFS = CParticleDataFactory::GetVectorElement(in);
#else
      desc->mPOFS = CParticleDataFactory::GetVectorElement(in);
#endif

      break;
    case 'IVEL':
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      swoosh->mIVEL = CParticleDataFactory::GetVectorElement(in);
#else
      desc->mIVEL = CParticleDataFactory::GetVectorElement(in);
#endif

      break;
    case 'NPOS':
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      swoosh->mNPOS = CParticleDataFactory::GetVectorElement(in);
#else
      desc->mNPOS = CParticleDataFactory::GetVectorElement(in);
#endif

      break;
    case 'VELM':
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      swoosh->mVELM = CParticleDataFactory::GetModVectorElement(in);
#else
      desc->mVELM = CParticleDataFactory::GetModVectorElement(in);
#endif

      break;
    case 'VLS1':
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      swoosh->mVLS1 = CParticleDataFactory::GetBool(in);
#else
      desc->mVLS1 = CParticleDataFactory::GetBool(in);
#endif

      break;
    case 'VLM2':
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      swoosh->mVLM2 = CParticleDataFactory::GetModVectorElement(in);
#else
      desc->mVLM2 = CParticleDataFactory::GetModVectorElement(in);
#endif

      break;
    case 'VLS2':
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      swoosh->mVLS2 = CParticleDataFactory::GetBool(in);
#else
      desc->mVLS2 = CParticleDataFactory::GetBool(in);
#endif

      break;
    case 'SPLN':
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      swoosh->mSPLN = CParticleDataFactory::GetIntElement(in);
#else
      desc->mSPLN = CParticleDataFactory::GetIntElement(in);
#endif

      break;
    case 'WIRE':
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      swoosh->mWIRE = CParticleDataFactory::GetBool(in);
#else
      desc->mWIRE = CParticleDataFactory::GetBool(in);
#endif

      break;
    case 'TEXR':
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      swoosh->mTEXR = CParticleDataFactory::GetTextureElement(in, pool);
#else
      desc->mTEXR = CParticleDataFactory::GetTextureElement(in, pool);
#endif

      break;
    case 'TSPN':
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      swoosh->mTSPN = CParticleDataFactory::GetIntElement(in);
#else
      desc->mTSPN = CParticleDataFactory::GetIntElement(in);
#endif

      break;
    case 'AALP':
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      swoosh->mAALP = CParticleDataFactory::GetBool(in);
#else
      desc->mAALP = CParticleDataFactory::GetBool(in);
#endif

      break;
    case 'ZBUF':
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      swoosh->mZBUF = CParticleDataFactory::GetBool(in);
#else
      desc->mZBUF = CParticleDataFactory::GetBool(in);
#endif

      break;
    case 'ORNT':
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      swoosh->mORNT = CParticleDataFactory::GetBool(in);
#else
      desc->mORNT = CParticleDataFactory::GetBool(in);
#endif

      break;
    case 'TEXW':
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      swoosh->mTEXW = CParticleDataFactory::GetBool(in);
#else
      desc->mTEXW = CParticleDataFactory::GetBool(in);
      break;
    case 'CLTX':
      desc->mCLTX = CParticleDataFactory::GetBool(in);
#endif

      break;
    case 'CRND':
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      swoosh->mCRND = CParticleDataFactory::GetBool(in);
#else
      desc->mCRND = CParticleDataFactory::GetBool(in);
      break;
    case 'DFLG':
      desc->mDFLG = CParticleDataFactory::GetBitflag(in);
#endif

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

