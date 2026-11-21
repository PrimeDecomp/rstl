#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/RstlVersions.h"
#include "rstl/macros.hpp"
#endif
#include "Kyoto/Particles/CParticleDataFactory.hpp"

#if !RSTL_CORRUPTION
#include "Kyoto/CFactoryFnReturn.hpp"
#endif
#include "Kyoto/CFactoryMgr.hpp"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/CVParamTransfer.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Particles/CColorElement.hpp"
#include "Kyoto/Particles/CElementAllocationChunk.hpp"

#include "Kyoto/Particles/CEmitterElement.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "Kyoto/Particles/CIntElement.hpp"
#include "Kyoto/Particles/CModVectorElement.hpp"
#include "Kyoto/Particles/CParticleGen.hpp"
#include "Kyoto/Particles/CRealElement.hpp"
#include "Kyoto/Particles/CSpawnSystemKeyframeData.hpp"
#include "Kyoto/Particles/CSwooshDescription.hpp"
#include "Kyoto/Particles/CUVElement.hpp"
#include "Kyoto/Particles/CVectorElement.hpp"
#include "Kyoto/Particles/IElement.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/Streams/CMemoryInStream.hpp"
#endif
#include "Kyoto/TToken.hpp"
#include "dolphin/types.h"
#include "rstl/algorithm.hpp"
#include "rstl/list.hpp"
#include "rstl/optional_object.hpp"
#define SBIG(v) v

CTexture* CreateTexture(int value);

rstl::list< CElementAllocationChunk > sElementAllocationChunks;
CElementAllocationChunk* IElement::CElementAllocator::sCurrentChunk = nullptr;
CElementAllocationChunk* IElement::CElementAllocator::sFreeChunk = nullptr;

void* IElement::CElementAllocator::Alloc(size_t size, const char*, const char*) {
  if (sCurrentChunk == nullptr || !sCurrentChunk->CanAllocate(size)) {
    sElementAllocationChunks.push_back(CElementAllocationChunk());
    sCurrentChunk = &sElementAllocationChunks.back();
  }

  return sCurrentChunk->Allocate(size);
}

void IElement::CElementAllocator::Free(void* ptr, size_t) {
  if (ptr == nullptr) {
    return;
  }

  if (sFreeChunk == nullptr || !sFreeChunk->Contains(ptr)) {
    sFreeChunk = nullptr;
#if RSTL_CORRUPTION
    for (rstl::list< CElementAllocationChunk >::iterator it = sElementAllocationChunks.begin();
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    for (AUTO(it, sElementAllocationChunks.begin());
#else
    for (rstl::list< CElementAllocationChunk >::iterator it = sElementAllocationChunks.begin();
#endif
#endif
         it != sElementAllocationChunks.end(); ++it) {
      if (it->Contains(ptr)) {
        sFreeChunk = &*it;
        break;
      }
    }
  }

  sFreeChunk->Free(ptr);
  if (sFreeChunk->GetAllocationCount() == 0) {
#if RSTL_CORRUPTION
    for (rstl::list< CElementAllocationChunk >::iterator it = sElementAllocationChunks.begin();
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    for (AUTO(it, sElementAllocationChunks.begin());
#else
    for (rstl::list< CElementAllocationChunk >::iterator it = sElementAllocationChunks.begin();
#endif
#endif
         it != sElementAllocationChunks.end(); ++it) {
      if (&*it == sFreeChunk) {
        sElementAllocationChunks.erase(it);
        if (sCurrentChunk == sFreeChunk) {
          sCurrentChunk = nullptr;
        }
        break;
      }
    }
    sFreeChunk = nullptr;
  }
}

#if RSTL_CORRUPTION
CFactoryFnReturn FParticleFactory(const SObjectTag& tag, const rstl::auto_ptr< uchar >& buffer,
                                  int size, const CVParamTransfer& xfer) {
  CMemoryInStream in(buffer.get(), size);
  rstl::rc_ptr< IVParamObj > obj = xfer.GetObj();
#else
RSTL_PRIME1_CONST CFactoryFnReturn FParticleFactory(const SObjectTag& tag, CInputStream& in,
                                                    const CVParamTransfer& xfer) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  rstl::rc_ptr< IVParamObj > obj = xfer.mObj;
#else
  rstl::rc_ptr< IVParamObj > obj = xfer.GetObj();
#endif

#endif
  CSimplePool* pool = static_cast< TObjOwnerParam< CSimplePool* >* >(obj.GetPtr())->GetData();
#if RSTL_CORRUPTION
  CGenDescription* desc = CParticleDataFactory::GetGeneratorDesc(in, pool, tag.id);
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CGenDescription* desc = CParticleDataFactory::GetGeneratorDesc(in, pool, tag.GetId());
#else
  CGenDescription* desc = CParticleDataFactory::GetGeneratorDesc(in, pool, tag.id);
#endif

#endif
  return desc;
}

CGenDescription* CParticleDataFactory::GetGeneratorDesc(CInputStream& in, CSimplePool* pool,
#if RSTL_CORRUPTION
                                                        CAssetId id) {
  rstl::vector< CAssetId > assets;
#else
                                                        uint id) {
  rstl::vector< uint > assets;
#endif
  assets.reserve(8);
  return CParticleDataFactory::CreateGeneratorDescription(in, assets, id, pool);
}

CGenDescription* CParticleDataFactory::CreateGeneratorDescription(CInputStream& in,
#if RSTL_CORRUPTION
                                                                  rstl::vector< CAssetId >& assets,
                                                                  CAssetId id, CSimplePool* pool) {
#else
                                                                  rstl::vector< uint >& assets,
                                                                  const uint id,
                                                                  CSimplePool* pool) {
#endif
  if (rstl::count(assets.begin(), assets.end(), id) != 0) {
    return nullptr;
  }
#if RSTL_CORRUPTION
  assets.push_back_unsafe(id);
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  assets.push_back(id);
#else
  assets.push_back_unsafe(id);
#endif

#endif
  FourCC clsId = GetClassID(in);
  if (clsId != SBIG('GPSM')) {
    return nullptr;
  }
  CGenDescription* desc = rs_new_line(178) CGenDescription;
  CreateGPSM(desc, in, assets, pool);
  LoadGPSMTokens(desc);
  return desc;
}

void CParticleDataFactory::LoadGPSMTokens(CGenDescription* desc) {
  if (desc->mPMDL) {
    desc->mPMDL->ForceCache();
  }
  if (desc->mICTS) {
    desc->mICTS->ForceCache();
  }
  if (desc->mIDTS) {
    desc->mIDTS->ForceCache();
  }
  if (desc->mIITS) {
    desc->mIITS->ForceCache();
  }
  if (desc->mSSWH) {
    desc->mSSWH->ForceCache();
  }
}

bool CParticleDataFactory::CreateGPSM(CGenDescription* desc, CInputStream& in,
                                      rstl::vector< CAssetId >& resources, CSimplePool* pool) {
  bool done = false;
  CRandom16 random(99);
  CGlobalRandom context(random);
#if RSTL_CORRUPTION
  rstl::optional_object< bool > additiveAlpha;
#endif
  while (!done) {
    FourCC clsId = GetClassID(in);
    switch (clsId) {
#if RSTL_CORRUPTION
    case SBIG('PSIV'): {
      CVectorElement* element = GetVectorElement(in);
      if (element != nullptr) {
        gpfnWarningPrintf(
            "BUG THIS!:PSIV - Particle system initial velocity has been depreciated %s.\n",
            resources.back().ToHexString().c_str());
        rs_debugger_printf(
            "BUG THIS!:PSIV - Particle system initial velocity has been depreciated %s.\n",
            resources.back().ToHexString().c_str());
      }
      delete element;
#else
    case SBIG('PSIV'):
      delete GetVectorElement(in);
#endif
      break;
#if RSTL_CORRUPTION
    }
    case SBIG('PSVM'): {
      CModVectorElement* element = GetModVectorElement(in);
      if (element != nullptr) {
        gpfnWarningPrintf(
            "BUG THIS!:PSVM - Particle system velocity modifier has been depreciated %s.\n",
            resources.back().ToHexString().c_str());
        rs_debugger_printf(
            "BUG THIS!:PSVM - Particle system velocity modifier has been depreciated %s.\n",
            resources.back().ToHexString().c_str());
      }
      delete element;
#else
    case SBIG('PSVM'):
      delete GetModVectorElement(in);
#endif
      break;
#if RSTL_CORRUPTION
    }
    case SBIG('PSOV'): {
      CVectorElement* element = GetVectorElement(in);
      if (element != nullptr) {
        gpfnWarningPrintf(
            "BUG THIS!:PSOV - Particle system orientation velocity has been depreciated %s.\n",
            resources.back().ToHexString().c_str());
        rs_debugger_printf(
            "BUG THIS!:PSOV - Particle system orientation velocity has been depreciated %s.\n",
            resources.back().ToHexString().c_str());
      }
      delete element;
#else
    case SBIG('PSOV'):
      delete GetVectorElement(in);
#endif
      break;
#if RSTL_CORRUPTION
    }
#endif
    case SBIG('PSTS'):
      desc->mPSTS = GetRealElement(in);
      break;
    case SBIG('PSLT'):
      desc->mPSLT = GetIntElement(in);
      break;
    case SBIG('PSWT'):
      desc->mPSWT = GetIntElement(in);
      break;
    case SBIG('LIT_'):
      desc->mLIT_ = GetBool(in);
      break;
    case SBIG('ORNT'):
      desc->mORNT = GetBool(in);
      break;
    case SBIG('RSOP'):
      desc->mRSOP = GetBool(in);
      break;
    case SBIG('AAPH'):
#if RSTL_CORRUPTION
      gpfnWarningPrintf(
          "BUG THIS!:AAPH - Particle system additive alpha has been depreciated %s.\n",
          resources.back().ToHexString().c_str());
      rs_debugger_printf(
          "BUG THIS!:AAPH - Particle system additive alpha has been depreciated %s.\n",
          resources.back().ToHexString().c_str());
      additiveAlpha = GetBool(in);
#else
      desc->mAAPH = GetBool(in);
#endif
      break;
#if RSTL_CORRUPTION
    case SBIG('PBDM'):
      desc->mPBDM = GetIntElement(in);
      break;
    case SBIG('XTAD'):
      desc->mXTAD = GetIntElement(in);
      break;
#else
#if !RSTL_PRIME1
    case SBIG('XTAD'):
      desc->mXTAD = GetIntElement(in);
      break;
#endif
#endif
    case SBIG('ZBUF'):
      desc->mZBUF = GetBool(in);
      break;
    case SBIG('SORT'):
      desc->mSORT = GetBool(in);
      break;
    case SBIG('MBLR'):
      desc->mMBLR = GetBool(in);
      break;
    case SBIG('MBSP'):
      desc->mMBSP = GetIntElement(in);
      break;
    case SBIG('MAXP'):
      desc->mMAXP = GetIntElement(in);
      break;
    case SBIG('GRTE'):
      desc->mGRTE = GetRealElement(in);
      break;
#if RSTL_CORRUPTION
    case SBIG('ILOC'): {
      CVectorElement* element = GetVectorElement(in);
      if (element != nullptr) {
        gpfnWarningPrintf("BUG THIS!:ILOC - Particle initial location has been depreciated %s.\n",
                          resources.back().ToHexString().c_str());
        rs_debugger_printf("BUG THIS!:ILOC - Particle initial location has been depreciated %s.\n",
                           resources.back().ToHexString().c_str());
      }
      delete element;
#else
    case SBIG('ILOC'):
      delete GetVectorElement(in);
#endif
      break;
#if RSTL_CORRUPTION
    }
    case SBIG('IVEC'): {
      CVectorElement* element = GetVectorElement(in);
      if (element != nullptr) {
        gpfnWarningPrintf("BUG THIS!:IVEC - Particle initial velocity has been depreciated %s.\n",
                          resources.back().ToHexString().c_str());
        rs_debugger_printf("BUG THIS!:IVEC - Particle initial velocity has been depreciated %s.\n",
                           resources.back().ToHexString().c_str());
      }
      delete element;
#else
    case SBIG('IVEC'):
      delete GetVectorElement(in);
#endif
      break;
#if RSTL_CORRUPTION
    }
#endif
    case SBIG('EMTR'):
      desc->mEMTR = GetEmitterElement(in);
      break;
    case SBIG('SIZE'):
      desc->mSIZE = GetRealElement(in);
      break;
    case SBIG('COLR'):
      desc->mCOLR = GetColorElement(in);
      break;
    case SBIG('POFS'):
      desc->mPOFS = GetVectorElement(in);
      break;
    case SBIG('VMD1'):
      desc->mVMD1 = GetBool(in);
      break;
    case SBIG('VMD2'):
      desc->mVMD2 = GetBool(in);
      break;
    case SBIG('VMD3'):
      desc->mVMD3 = GetBool(in);
      break;
    case SBIG('VMD4'):
      desc->mVMD4 = GetBool(in);
      break;
    case SBIG('VEL1'):
      desc->mVEL1 = GetModVectorElement(in);
      break;
    case SBIG('VEL2'):
      desc->mVEL2 = GetModVectorElement(in);
      break;
    case SBIG('VEL3'):
      desc->mVEL3 = GetModVectorElement(in);
      break;
    case SBIG('VEL4'):
      desc->mVEL4 = GetModVectorElement(in);
      break;
    case SBIG('LTME'):
      desc->mLTME = GetIntElement(in);
      break;
    case SBIG('ROTA'):
      desc->mROTA = GetRealElement(in);
      break;
    case SBIG('LENG'):
      desc->mLENG = GetRealElement(in);
      break;
    case SBIG('WIDT'):
      desc->mWIDT = GetRealElement(in);
      break;
    case SBIG('TEXR'):
      desc->mTEXR = GetTextureElement(in, pool);
      break;
    case SBIG('TIND'):
      desc->mTIND = GetTextureElement(in, pool);
      break;
    case SBIG('CIND'):
      desc->mCIND = GetBool(in);
      break;
#if RSTL_CORRUPTION
    case SBIG('INDM'):
      desc->mINDM = GetBool(in);
      break;
#else
#if !RSTL_PRIME1
    case SBIG('INDM'):
      desc->mINDM = GetBool(in);
      break;
#endif

#endif
    case SBIG('PMDL'): {
      rstl::optional_object< TToken< CModel > > model(GetModel(in, pool));
      if (model) {
        desc->mPMDL = TCachedToken< CModel >(*model);
      } else {
#if RSTL_CORRUPTION
        desc->mPMDL = CGenDescription::TParticleModel();
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
        desc->mPMDL = rstl::optional_object_null();
#else
        desc->mPMDL = CGenDescription::TParticleModel();
#endif

#endif
      }
    } break;
    case SBIG('PMOP'):
      desc->mPMOP = GetVectorElement(in);
      break;
    case SBIG('PMRT'):
      desc->mPMRT = GetVectorElement(in);
      break;
    case SBIG('PMSC'):
      desc->mPMSC = GetVectorElement(in);
      break;
    case SBIG('PMCL'):
      desc->mPMCL = GetColorElement(in);
      break;
    case SBIG('PMAB'):
      desc->mPMAB = GetBool(in);
      break;
    case SBIG('PMUS'):
      desc->mPMUS = GetBool(in);
      break;
    case SBIG('PMOO'):
      desc->mPMOO = GetBool(in);
      break;
#if RSTL_CORRUPTION
    case SBIG('VMPC'):
      desc->mVMPC = GetBool(in);
      break;
    case SBIG('PMOV'):
      desc->mPMOV = GetVectorElement(in);
      break;
#else
#if !RSTL_PRIME1
    case SBIG('VMPC'):
      desc->mVMPC = GetBool(in);
      break;
    case SBIG('PMOV'):
      desc->mPMOV = GetVectorElement(in);
      break;
#endif

#endif
    case SBIG('SEED'):
      desc->mSEED = GetIntElement(in);
      break;
    case SBIG('ICTS'): {
      rstl::optional_object< TToken< CGenDescription > > child(
          GetChildGeneratorDesc(in, pool, resources));
      if (child) {
        desc->mICTS = TCachedToken< CGenDescription >(*child);
      } else {
#if RSTL_CORRUPTION
        desc->mICTS = CGenDescription::TChildGeneratorDesc();
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
        desc->mICTS = rstl::optional_object_null();
#else
        desc->mICTS = CGenDescription::TChildGeneratorDesc();
#endif

#endif
      }
      break;
    }
    case SBIG('NCSY'):
      desc->mNCSY = GetIntElement(in);
      break;
    case SBIG('CSSD'):
      desc->mCSSD = GetIntElement(in);
      break;
    case SBIG('IDTS'): {
      rstl::optional_object< TToken< CGenDescription > > child(
          GetChildGeneratorDesc(in, pool, resources));
      if (child) {
        desc->mIDTS = TCachedToken< CGenDescription >(*child);
      } else {
#if RSTL_CORRUPTION
        desc->mIDTS = CGenDescription::TChildGeneratorDesc();
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
        desc->mIDTS = rstl::optional_object_null();
#else
        desc->mIDTS = CGenDescription::TChildGeneratorDesc();
#endif

#endif
      }
      break;
    }
    case SBIG('NDSY'):
      desc->mNDSY = GetIntElement(in);
      break;
    case SBIG('IITS'): {
      rstl::optional_object< TToken< CGenDescription > > child(
          GetChildGeneratorDesc(in, pool, resources));
      if (child) {
        desc->mIITS = TCachedToken< CGenDescription >(*child);
      } else {
#if RSTL_CORRUPTION
        desc->mIITS = CGenDescription::TChildGeneratorDesc();
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
        desc->mIITS = rstl::optional_object_null();
#else
        desc->mIITS = CGenDescription::TChildGeneratorDesc();
#endif

#endif
      }
      break;
    }
    case SBIG('PISY'):
      desc->mPISY = GetIntElement(in);
      break;
    case SBIG('SISY'):
      desc->mSISY = GetIntElement(in);
      break;
    case SBIG('SSWH'): {
      rstl::optional_object< TToken< CSwooshDescription > > child(GetSwooshGeneratorDesc(in, pool));
      if (child) {
        desc->mSSWH = TCachedToken< CSwooshDescription >(*child);
      } else {
#if RSTL_CORRUPTION
        desc->mSSWH = CGenDescription::TSwooshGeneratorDesc();
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
        desc->mSSWH = rstl::optional_object_null();
#else
        desc->mSSWH = CGenDescription::TSwooshGeneratorDesc();
#endif

#endif
      }
      break;
    }
    case SBIG('SSSD'):
      desc->mSSSD = GetIntElement(in);
      break;
    case SBIG('SSPO'):
      desc->mSSPO = GetVectorElement(in);
      break;
    case SBIG('SELC'): {
      rstl::optional_object< TToken< CElectricDescription > > electric =
          GetElectricGeneratorDesc(in, pool);
      if (electric) {
        desc->mSELC = *electric;
      } else {
#if RSTL_CORRUPTION
        desc->mSELC = CGenDescription::TElectricGeneratorDesc();
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
        desc->mSELC = rstl::optional_object_null();
#else
        desc->mSELC = CGenDescription::TElectricGeneratorDesc();
#endif

#endif
      }
      break;
    }
    case SBIG('SESD'):
      desc->mSESD = GetIntElement(in);
      break;
    case SBIG('SEPO'):
      desc->mSEPO = GetVectorElement(in);
      break;
    case SBIG('KSSM'):
      if (GetClassID(in) == SBIG('CNST')) {
        desc->mKSSM = rs_new_line(692) CSpawnSystemKeyframeData(in);
        desc->mKSSM->LoadAllSpawnedSystemTokens(pool);
      }
      break;
    case SBIG('LINE'):
      desc->mLINE = GetBool(in);
      break;
    case SBIG('FXLL'):
      desc->mFXLL = GetBool(in);
      break;
    case SBIG('LTYP'):
      desc->mLTYP = GetIntElement(in);
      break;
    case SBIG('LCLR'):
      desc->mLCLR = GetColorElement(in);
      break;
    case SBIG('LINT'):
      desc->mLINT = GetRealElement(in);
      break;
    case SBIG('LOFF'):
      desc->mLOFF = GetVectorElement(in);
      break;
    case SBIG('LDIR'):
      desc->mLDIR = GetVectorElement(in);
      break;
    case SBIG('LFOT'):
      desc->mLFOT = GetIntElement(in);
      break;
    case SBIG('LFOR'):
      desc->mLFOR = GetRealElement(in);
      break;
    case SBIG('LSLA'):
      desc->mLSLA = GetRealElement(in);
      break;
    case SBIG('OPTS'):
      desc->mOPTS = GetBool(in);
      break;
    case SBIG('ADV1'):
      desc->mADV1 = GetRealElement(in);
      break;
    case SBIG('ADV2'):
      desc->mADV2 = GetRealElement(in);
      break;
    case SBIG('ADV3'):
      desc->mADV3 = GetRealElement(in);
      break;
    case SBIG('ADV4'):
      desc->mADV4 = GetRealElement(in);
      break;
    case SBIG('ADV5'):
      desc->mADV5 = GetRealElement(in);
      break;
    case SBIG('ADV6'):
      desc->mADV6 = GetRealElement(in);
      break;
    case SBIG('ADV7'):
      desc->mADV7 = GetRealElement(in);
      break;
    case SBIG('ADV8'):
      desc->mADV8 = GetRealElement(in);
      break;
#if RSTL_CORRUPTION
    case SBIG('ADV9'):
      desc->mADV9 = GetRealElement(in);
      break;
    case SBIG('VAV1'):
      desc->mVAV1 = GetVectorElement(in);
      break;
    case SBIG('VAV2'):
      desc->mVAV2 = GetVectorElement(in);
      break;
    case SBIG('VAV3'):
      desc->mVAV3 = GetVectorElement(in);
      break;
    case SBIG('DFLG'):
      desc->mDFLG = GetBitflag(in);
      break;
    case SBIG('FXBR'):
      desc->mFXBR = GetRealElement(in);
      break;
    case SBIG('FXBO'):
      desc->mFXBO = GetVectorElement(in);
      break;
    case SBIG('RDOP'):
      desc->mRDOP = GetBool(in);
      break;
    case SBIG('ISVF'):
      desc->mISVF = GetRealElement(in);
      break;
    case SBIG('SVEO'):
      desc->mSVEO = GetRealElement(in);
      break;
#else
#if !RSTL_PRIME1
    case SBIG('ADV9'):
      desc->mADV9 = GetRealElement(in);
      break;
    case SBIG('VAV1'):
      desc->mVAV1 = GetVectorElement(in);
      break;
    case SBIG('VAV2'):
      desc->mVAV2 = GetVectorElement(in);
      break;
    case SBIG('VAV3'):
      desc->mVAV3 = GetVectorElement(in);
      break;
    case SBIG('DFLG'):
      desc->mDFLG = GetBitflag(in);
      break;
#if RSTL_PARTICLE_BOUNDS_MODES
    case SBIG('FXBR'):
      desc->mFXBR = GetRealElement(in);
      break;
    case SBIG('FXBO'):
      desc->mFXBO = GetVectorElement(in);
      break;
#endif
#if RSTL_PARTICLE_RDOP
    case SBIG('RDOP'):
      desc->mRDOP = GetBool(in);
      break;
#endif
#endif

#endif
    case SBIG('_END'):
      done = true;
      break;
    default:
      return false;
    }
  }

#if RSTL_CORRUPTION
  if (desc->mPBDM == nullptr) {
    // Older data expresses the blend mode as the additive alpha flag.
    const int additive = additiveAlpha.valid() ? (*additiveAlpha != 0) : 0;
    desc->mPBDM = rs_new_line(893) CIEConstant(additive);
  }

#endif
  return true;
}

#if RSTL_CORRUPTION
FourCC CParticleDataFactory::GetClassID(CInputStream& in) { return in.ReadInt32(); }
#else
FourCC CParticleDataFactory::GetClassID(CInputStream& in) {
  return in.ReadLong();
}
#endif

bool CParticleDataFactory::GetBool(CInputStream& in) {
  GetClassID(in);
  return in.ReadBool();
}

#if RSTL_CORRUPTION
int CParticleDataFactory::GetBitflag(CInputStream& in) {
  GetClassID(in);
  FourCC type = GetClassID(in);
  uint count = in.ReadInt32();
  switch (type) {
  case SBIG('BITF'):
    return in.ReadInt32();
  default:
    for (uint i = 0; i < count; ++i) {
      in.ReadUint8();
    }
    return 0;
  }
}
#else
#if !RSTL_PRIME1
int CParticleDataFactory::GetBitflag(CInputStream& in) {
  GetClassID(in);
  FourCC type = GetClassID(in);
  uint count = in.ReadInt32();
  switch (type) {
  case SBIG('BITF'):
    return in.ReadInt32();
  default:
    for (uint i = 0; i < count; ++i) {
      in.ReadUint8();
    }
    return 0;
  }
}
#endif
#endif

int CParticleDataFactory::GetInt(CInputStream& in) { return in.ReadInt32(); }

float CParticleDataFactory::GetReal(CInputStream& in) { return in.ReadFloat(); }

CIntElement* CParticleDataFactory::GetIntElement(CInputStream& in) {
  FourCC clsId = GetClassID(in);
  switch (clsId) {
  case SBIG('CNST'): {
    return rs_new_line(991) CIEConstant(GetInt(in));
  }
  case SBIG('KEYE'):
  case SBIG('KEYP'): {
    return rs_new_line(996) CIEKeyframeEmitter(in);
  }
#if RSTL_CORRUPTION
  case SBIG('KEYF'): {
    return rs_new_line(1000) CIEKeyframeInput(in);
  }
#else
#if !RSTL_PRIME1
  case SBIG('KEYF'): {
    return rs_new CIEKeyframeInput(in);
  }
#endif
#endif
  case SBIG('TSCL'): {
    return rs_new_line(1004) CIETimescale(GetRealElement(in));
  }
  case SBIG('DETH'): {
    CIntElement* a = GetIntElement(in);
    CIntElement* b = GetIntElement(in);
    return rs_new_line(1012) CIEDeath(a, b);
  }
  case SBIG('CHAN'): {
    CIntElement* a = GetIntElement(in);
    CIntElement* b = GetIntElement(in);
    CIntElement* c = GetIntElement(in);
    return rs_new_line(1022) CIETimeChain(a, b, c);
  }
  case SBIG('ADD_'): {
    CIntElement* a = GetIntElement(in);
    CIntElement* b = GetIntElement(in);
    return rs_new_line(1031) CIEAdd(a, b);
  }
  case SBIG('MULT'): {
    CIntElement* a = GetIntElement(in);
    CIntElement* b = GetIntElement(in);
    return rs_new_line(1040) CIEMultiply(a, b);
  }
#if RSTL_CORRUPTION
  case SBIG('DIVD'): {
    CIntElement* a = GetIntElement(in);
    CIntElement* b = GetIntElement(in);
    return rs_new_line(1049) CIEDivide(a, b);
  }
#else
#if !RSTL_PRIME1
  case SBIG('DIVD'): {
    CIntElement* a = GetIntElement(in);
    CIntElement* b = GetIntElement(in);
    return rs_new CIEDivide(a, b);
  }
#endif
#endif
  case SBIG('MODU'): {
    CIntElement* a = GetIntElement(in);
    CIntElement* b = GetIntElement(in);
    return rs_new_line(1058) CIEModulo(a, b);
  }
  case SBIG('RAND'): {
    CIntElement* a = GetIntElement(in);
    CIntElement* b = GetIntElement(in);
    return rs_new_line(1067) CIERandom(a, b);
  }
  case SBIG('IMPL'): {
    return rs_new_line(1073) CIEImpulse(GetIntElement(in));
  }
  case SBIG('ILPT'): {
    return rs_new_line(1079) CIELifetimePercent(GetIntElement(in));
  }
  case SBIG('SPAH'): {
    CIntElement* a = GetIntElement(in);
    CIntElement* b = GetIntElement(in);
    CIntElement* c = GetIntElement(in);
    return rs_new_line(1089) CIESampleAndHold(c, a, b);
  }
  case SBIG('IRND'): {
    CIntElement* a = GetIntElement(in);
    CIntElement* b = GetIntElement(in);
    return rs_new_line(1098) CIEInitialRandom(a, b);
  }
  case SBIG('CLMP'): {
    CIntElement* a = GetIntElement(in);
    CIntElement* b = GetIntElement(in);
    CIntElement* c = GetIntElement(in);
    return rs_new_line(1108) CIEClamp(a, b, c);
  }
  case SBIG('PULS'): {
    CIntElement* a = GetIntElement(in);
    CIntElement* b = GetIntElement(in);
    CIntElement* c = GetIntElement(in);
    CIntElement* d = GetIntElement(in);
    return rs_new_line(1119) CIEPulse(a, b, c, d);
  }
  case SBIG('NONE'): {
    return nullptr;
  }
  case SBIG('RTOI'): {
    CRealElement* a = GetRealElement(in);
    CRealElement* b = GetRealElement(in);
    return rs_new_line(1132) CIERealToInt(a, b);
  }
  case SBIG('SUB_'): {
    CIntElement* a = GetIntElement(in);
    CIntElement* b = GetIntElement(in);
    return rs_new_line(1141) CIESubtract(a, b);
  }
  case SBIG('GTCP'): {
    return rs_new_line(1147) CIEGetCumulativeParticleCount();
  }
  case SBIG('GAPC'): {
    return rs_new_line(1153) CIEGetActiveParticleCount();
  }
  case SBIG('GEMT'): {
    return rs_new_line(1159) CIEGetEmitterTime();
  }
#if RSTL_CORRUPTION
  case SBIG('ISWT'): {
    CIntElement* a = GetIntElement(in);
    CIntElement* b = GetIntElement(in);
    return rs_new_line(1167) CIEInitialSwitch(a, b);
  }
  case SBIG('KPIN'): {
    CIntElement* a = GetIntElement(in);
    return rs_new_line(1174) CIEKeepInitial(a);
  }
  case SBIG('PCRT'): {
    return rs_new_line(1180) CIEParticleCreationTime();
  }
  case SBIG('PDET'): {
    return rs_new_line(1186) CIEParticleCreationTime();
  }
#else
#if !RSTL_PRIME1
  case SBIG('ISWT'): {
    CIntElement* a = GetIntElement(in);
    CIntElement* b = GetIntElement(in);
    return rs_new CIEInitialSwitch(a, b);
  }
  case SBIG('KPIN'): {
    CIntElement* a = GetIntElement(in);
    return rs_new CIEKeepInitial(a);
  }
  case SBIG('PCRT'): {
    return rs_new CIEParticleCreationTime();
  }
  case SBIG('PDET'): {
    return rs_new CIEParticleCreationTime();
  }
#endif

#endif
  }
  return nullptr;
}

CRealElement* CParticleDataFactory::GetRealElement(CInputStream& in) {
  FourCC clsId = GetClassID(in);
  switch (clsId) {
  case SBIG('CNST'): {
    return rs_new_line(1208) CREConstant(GetReal(in));
  }
  case SBIG('NONE'): {
    return nullptr;
  }
  case SBIG('KEYE'):
  case SBIG('KEYP'): {
    return rs_new_line(1217) CREKeyframeEmitter(in);
  }
#if RSTL_CORRUPTION
  case SBIG('KEYF'): {
    return rs_new_line(1222) CREKeyframeInput(in);
  }
#else
#if !RSTL_PRIME1
  case SBIG('KEYF'): {
    return rs_new CREKeyframeInput(in);
  }
#endif
#endif
  case SBIG('SCAL'): {
    return rs_new_line(1227) CRETimeScale(GetRealElement(in));
  }
  case SBIG('SINE'): {
    CRealElement* a = GetRealElement(in);
    CRealElement* b = GetRealElement(in);
    CRealElement* c = GetRealElement(in);
    return rs_new_line(1236) CRESineWave(c, a, b);
  }
  case SBIG('ADD_'): {
    CRealElement* a = GetRealElement(in);
    CRealElement* b = GetRealElement(in);
    return rs_new_line(1245) CREAdd(a, b);
  }
  case SBIG('MULT'): {
    CRealElement* a = GetRealElement(in);
    CRealElement* b = GetRealElement(in);
    return rs_new_line(1254) CREMultiply(a, b);
  }
  case SBIG('DOTP'): {
    CVectorElement* a = GetVectorElement(in);
    CVectorElement* b = GetVectorElement(in);
    return rs_new_line(1263) CREDotProduct(a, b);
  }
  case SBIG('RAND'): {
    CRealElement* min = GetRealElement(in);
    CRealElement* max = GetRealElement(in);
    return rs_new_line(1272) CRERandom(min, max);
  }
  case SBIG('IRND'): {
    CRealElement* min = GetRealElement(in);
    CRealElement* max = GetRealElement(in);
    return rs_new_line(1281) CREInitialRandom(min, max);
  }
  case SBIG('CHAN'): {
    CRealElement* a = GetRealElement(in);
    CRealElement* b = GetRealElement(in);
    CIntElement* c = GetIntElement(in);
    return rs_new_line(1291) CRETimeChain(a, b, c);
  }
  case SBIG('CLMP'): {
    CRealElement* a = GetRealElement(in);
    CRealElement* b = GetRealElement(in);
    CRealElement* c = GetRealElement(in);
    return rs_new_line(1301) CREClamp(a, b, c);
  }
  case SBIG('PULS'): {
    CIntElement* a = GetIntElement(in);
    CIntElement* b = GetIntElement(in);
    CRealElement* c = GetRealElement(in);
    CRealElement* d = GetRealElement(in);
    return rs_new_line(1312) CREPulse(a, b, c, d);
  }
  case SBIG('RLPT'): {
    CRealElement* a = GetRealElement(in);
    return rs_new_line(1320) CRELifetimePercent(a);
  }
  case SBIG('LFTW'): {
    CRealElement* a = GetRealElement(in);
    CRealElement* b = GetRealElement(in);
    return rs_new_line(1329) CRELifetimeTween(a, b);
  }
  case SBIG('PRLW'): {
    return rs_new_line(1335) CREParticleRotationOrLineWidth();
  }
  case SBIG('PSLL'): {
    return rs_new_line(1341) CREParticleSizeOrLineLength();
  }
  case SBIG('PAP1'): {
    return rs_new_line(1347) CREParticleAccessParameter1();
  }
  case SBIG('PAP2'): {
    return rs_new_line(1353) CREParticleAccessParameter2();
  }
  case SBIG('PAP3'): {
    return rs_new_line(1359) CREParticleAccessParameter3();
  }
  case SBIG('PAP4'): {
    return rs_new_line(1365) CREParticleAccessParameter4();
  }
  case SBIG('PAP5'): {
    return rs_new_line(1371) CREParticleAccessParameter5();
  }
  case SBIG('PAP6'): {
    return rs_new_line(1377) CREParticleAccessParameter6();
  }
  case SBIG('PAP7'): {
    return rs_new_line(1383) CREParticleAccessParameter7();
  }
  case SBIG('PAP8'): {
    return rs_new_line(1389) CREParticleAccessParameter8();
  }
#if RSTL_CORRUPTION
  case SBIG('PAP9'): {
    return rs_new_line(1395) CREParticleAccessParameter9();
  }
#else
#if !RSTL_PRIME1
  case SBIG('PAP9'): {
    return rs_new CREParticleAccessParameter9();
  }
#endif

#endif
  case SBIG('VXTR'): {
    CVectorElement* a = GetVectorElement(in);
    return rs_new_line(1402) CREVectorXToReal(a);
  }
  case SBIG('VYTR'): {
    CVectorElement* a = GetVectorElement(in);
    return rs_new_line(1409) CREVectorYToReal(a);
  }
  case SBIG('VZTR'): {
    CVectorElement* a = GetVectorElement(in);
    return rs_new_line(1416) CREVectorZToReal(a);
  }
  case SBIG('VMAG'): {
    CVectorElement* a = GetVectorElement(in);
    return rs_new_line(1423) CREVectorMagnitude(a);
  }
  case SBIG('ISWT'): {
    CRealElement* a = GetRealElement(in);
    CRealElement* b = GetRealElement(in);
    return rs_new_line(1431) CREInitialSwitch(a, b);
  }
  case SBIG('CLTN'): {
    CRealElement* a = GetRealElement(in);
    CRealElement* b = GetRealElement(in);
    CRealElement* c = GetRealElement(in);
    CRealElement* d = GetRealElement(in);
    return rs_new_line(1441) CRECompareLessThan(a, b, c, d);
  }
  case SBIG('CEQL'): {
    CRealElement* a = GetRealElement(in);
    CRealElement* b = GetRealElement(in);
    CRealElement* c = GetRealElement(in);
    CRealElement* d = GetRealElement(in);
    return rs_new_line(1451) CRECompareEqual(a, b, c, d);
  }
  case SBIG('CRNG'): {
    CRealElement* a = GetRealElement(in);
    CRealElement* b = GetRealElement(in);
    CRealElement* c = GetRealElement(in);
    CRealElement* d = GetRealElement(in);
    CRealElement* e = GetRealElement(in);
    return rs_new_line(1462) CREConstantRange(a, b, c, d, e);
  }
  case SBIG('CEXT'): {
    CIntElement* a = GetIntElement(in);
    return rs_new_line(1469) CREExternalVar(a);
  }
  case SBIG('ITRL'): {
    CIntElement* a = GetIntElement(in);
    CRealElement* b = GetRealElement(in);
    return rs_new_line(1478) CREIntTimesReal(a, b);
  }
  case SBIG('SUB_'): {
    CRealElement* a = GetRealElement(in);
    CRealElement* b = GetRealElement(in);
    return rs_new_line(1486) CRESubtract(a, b);
  }
  case SBIG('GTCR'): {
    CColorElement* alpha = GetColorElement(in);
    return rs_new_line(1493) CREGetComponentRed(alpha);
  }
  case SBIG('GTCG'): {
    CColorElement* alpha = GetColorElement(in);
    return rs_new_line(1500) CREGetComponentGreen(alpha);
  }
  case SBIG('GTCB'): {
    CColorElement* alpha = GetColorElement(in);
    return rs_new_line(1507) CREGetComponentBlue(alpha);
  }
  case SBIG('GTCA'): {
    CColorElement* alpha = GetColorElement(in);
    return rs_new_line(1514) CREGetComponentAlpha(alpha);
  }
#if RSTL_CORRUPTION
  case SBIG('GTCP'): {
    return rs_new_line(1520) CREGetCumulativeParticleCount();
  }
  case SBIG('KPIN'): {
    CRealElement* a = GetRealElement(in);
    return rs_new_line(1527) CREKeepInitial(a);
  }
  case SBIG('OCSP'): {
    CIntElement* a = GetIntElement(in);
    return rs_new_line(1534) CREOscillatingSweep(a);
  }
  case SBIG('TOCS'): {
    const bool a = GetBool(in);
    CIntElement* b = GetIntElement(in);
    CIntElement* c = GetIntElement(in);
    CIntElement* d = GetIntElement(in);
    return rs_new_line(1544) CRETimeOscillatingSweep(a, b, c, d);
  }
  case SBIG('PRN1'): {
    CRealElement* a = GetRealElement(in);
    return rs_new_line(1551) CREPerlinNoise1d(a);
  }
  case SBIG('PRN2'): {
    CRealElement* a = GetRealElement(in);
    CRealElement* b = GetRealElement(in);
    return rs_new_line(1559) CREPerlinNoise2d(a, b);
  }
  case SBIG('PRN3'): {
    CVectorElement* a = GetVectorElement(in);
    return rs_new_line(1566) CREPerlinNoise3d(a);
  }
  case SBIG('PRN4'): {
    CVectorElement* a = GetVectorElement(in);
    CRealElement* b = GetRealElement(in);
    return rs_new_line(1574) CREPerlinNoise4d(a, b);
  }
  case SBIG('PNO1'): {
    CRealElement* a = GetRealElement(in);
    CRealElement* b = GetRealElement(in);
    CRealElement* c = GetRealElement(in);
    CIntElement* d = GetIntElement(in);
    return rs_new_line(1584) CREPerlinNoiseOctave1d(a, b, c, d);
  }
  case SBIG('PNO2'): {
    CRealElement* a = GetRealElement(in);
    CRealElement* b = GetRealElement(in);
    CRealElement* c = GetRealElement(in);
    CRealElement* d = GetRealElement(in);
    CIntElement* e = GetIntElement(in);
    return rs_new_line(1595) CREPerlinNoiseOctave2d(a, b, c, d, e);
  }
  case SBIG('PNO3'): {
    CVectorElement* a = GetVectorElement(in);
    CRealElement* b = GetRealElement(in);
    CRealElement* c = GetRealElement(in);
    CIntElement* d = GetIntElement(in);
    return rs_new_line(1605) CREPerlinNoiseOctave3d(a, b, c, d);
  }
  case SBIG('PNO4'): {
    CVectorElement* a = GetVectorElement(in);
    CRealElement* b = GetRealElement(in);
    CRealElement* c = GetRealElement(in);
    CRealElement* d = GetRealElement(in);
    CIntElement* e = GetIntElement(in);
    return rs_new_line(1616) CREPerlinNoiseOctave4d(a, b, c, d, e);
  }
#else
#if !RSTL_PRIME1
  case SBIG('GTCP'): {
    return rs_new CREGetCumulativeParticleCount();
  }
  case SBIG('KPIN'): {
    CRealElement* a = GetRealElement(in);
    return rs_new CREKeepInitial(a);
  }
  case SBIG('OCSP'): {
    CIntElement* a = GetIntElement(in);
    return rs_new CREOscillatingSweep(a);
  }
  case SBIG('TOCS'): {
    bool a = GetBool(in);
    CIntElement* b = GetIntElement(in);
    CIntElement* c = GetIntElement(in);
    CIntElement* d = GetIntElement(in);
    return rs_new CRETimeOscillatingSweep(a, b, c, d);
  }
  case SBIG('PRN1'): {
    CRealElement* a = GetRealElement(in);
    return rs_new CREPerlinNoise1d(a);
  }
  case SBIG('PRN2'): {
    CRealElement* a = GetRealElement(in);
    CRealElement* b = GetRealElement(in);
    return rs_new CREPerlinNoise2d(a, b);
  }
  case SBIG('PRN3'): {
    CVectorElement* a = GetVectorElement(in);
    return rs_new CREPerlinNoise3d(a);
  }
  case SBIG('PRN4'): {
    CVectorElement* a = GetVectorElement(in);
    CRealElement* b = GetRealElement(in);
    return rs_new CREPerlinNoise4d(a, b);
  }
  case SBIG('PNO1'): {
    CRealElement* a = GetRealElement(in);
    CRealElement* b = GetRealElement(in);
    CRealElement* c = GetRealElement(in);
    CIntElement* d = GetIntElement(in);
    return rs_new CREPerlinNoiseOctave1d(a, b, c, d);
  }
  case SBIG('PNO2'): {
    CRealElement* a = GetRealElement(in);
    CRealElement* b = GetRealElement(in);
    CRealElement* c = GetRealElement(in);
    CRealElement* d = GetRealElement(in);
    CIntElement* e = GetIntElement(in);
    return rs_new CREPerlinNoiseOctave2d(a, b, c, d, e);
  }
  case SBIG('PNO3'): {
    CVectorElement* a = GetVectorElement(in);
    CRealElement* b = GetRealElement(in);
    CRealElement* c = GetRealElement(in);
    CIntElement* d = GetIntElement(in);
    return rs_new CREPerlinNoiseOctave3d(a, b, c, d);
  }
  case SBIG('PNO4'): {
    CVectorElement* a = GetVectorElement(in);
    CRealElement* b = GetRealElement(in);
    CRealElement* c = GetRealElement(in);
    CRealElement* d = GetRealElement(in);
    CIntElement* e = GetIntElement(in);
    return rs_new CREPerlinNoiseOctave4d(a, b, c, d, e);
  }
#endif

#endif
  }
  return nullptr;
}

CVectorElement* CParticleDataFactory::GetVectorElement(CInputStream& in) {
  CVectorElement* ret;
  FourCC clsId = GetClassID(in);
  switch (clsId) {
  case SBIG('NONE'):
    ret = nullptr;
    break;
  case SBIG('CNST'): {
    CElementAllocationChunk* allocationContext = IElement::CElementAllocator::GetCurrentChunk();
    uint initialSize = IElement::CElementAllocator::GetCurrentAllocatedSize();
    CRealElement* x = GetRealElement(in);
    CRealElement* y = GetRealElement(in);
    CRealElement* z = GetRealElement(in);
    if (x && y && z) {
      if (x->IsConstant() && y->IsConstant() && z->IsConstant()) {
        float xf, yf, zf;
        x->GetValue(0, xf);
        y->GetValue(0, yf);
        z->GetValue(0, zf);

        delete x;
        delete y;
        delete z;

        if (allocationContext != nullptr &&
            allocationContext == IElement::CElementAllocator::GetCurrentChunk()) {
          allocationContext->Rewind(allocationContext->GetAllocatedSize() - initialSize);
        }
        ret = rs_new_line(1670) CVEFastConstant(xf, yf, zf);
        break;
      }
    }
    ret = rs_new_line(1674) CVEConstant(x, y, z);
    break;
  }
  case SBIG('KEYE'):
  case SBIG('KEYP'):
    ret = rs_new_line(1681) CVEKeyframeEmitter(in);
    break;
#if RSTL_CORRUPTION
  case SBIG('KEYF'): {
    ret = rs_new_line(1685) CVEKeyframeInput(in);
    break;
  }
#else
#if !RSTL_PRIME1
  case SBIG('KEYF'): {
    ret = rs_new CVEKeyframeInput(in);
    break;
  }
#endif

#endif
  case SBIG('ANGC'): {
    CRealElement* angleXBias = GetRealElement(in);
    CRealElement* angleYBias = GetRealElement(in);
    CRealElement* angleXRange = GetRealElement(in);
    CRealElement* angleYRange = GetRealElement(in);
    CRealElement* magnitude = GetRealElement(in);
    ret =
        rs_new_line(1696) CVEAngleCone(angleXBias, angleYBias, angleXRange, angleYRange, magnitude);
    break;
  }
  case SBIG('CONE'): {
    CVectorElement* direction = GetVectorElement(in);
    CRealElement* baseRadius = GetRealElement(in);
    ret = rs_new_line(1705) CVECone(direction, baseRadius);
    break;
  }
  case SBIG('CIRC'): {
    CVectorElement* circleOffset = GetVectorElement(in);
    CVectorElement* circleNormal = GetVectorElement(in);
    CRealElement* angleConstant = GetRealElement(in);
    CRealElement* angleLinear = GetRealElement(in);
    CRealElement* radius = GetRealElement(in);
    ret =
        rs_new_line(1717) CVECircle(circleOffset, circleNormal, angleConstant, angleLinear, radius);
    break;
  }
#if RSTL_CORRUPTION
  case SBIG('RNDV'): {
    CRealElement* a = GetRealElement(in);
    ret = rs_new_line(1725) CVERandomVector(a);
    break;
  }
#else
#if !RSTL_PRIME1
  case SBIG('RNDV'): {
    CRealElement* a = GetRealElement(in);
    ret = rs_new CVERandomVector(a);
    break;
  }
#endif
#endif
  case SBIG('CCLU'): {
    CVectorElement* circleOffset = GetVectorElement(in);
    CVectorElement* circleNormal = GetVectorElement(in);
    CIntElement* cycleFrames = GetIntElement(in);
    CRealElement* randomFactor = GetRealElement(in);

    ret = rs_new_line(1736) CVECircleCluster(circleOffset, circleNormal, cycleFrames, randomFactor);
    break;
  }
  case SBIG('ADD_'): {
    CVectorElement* a = GetVectorElement(in);
    CVectorElement* b = GetVectorElement(in);
    ret = rs_new_line(1745) CVEAdd(a, b);
    break;
  }
  case SBIG('MULT'): {
    CVectorElement* a = GetVectorElement(in);
    CVectorElement* b = GetVectorElement(in);
    ret = rs_new_line(1754) CVEMultiply(a, b);
    break;
  }
  case SBIG('CHAN'): {
    CVectorElement* a = GetVectorElement(in);
    CVectorElement* b = GetVectorElement(in);
    CIntElement* switchFrame = GetIntElement(in);

    ret = rs_new_line(1764) CVETimeChain(a, b, switchFrame);
    break;
  }
  case SBIG('PULS'): {
    CIntElement* durationA = GetIntElement(in);
    CIntElement* durationB = GetIntElement(in);
    CVectorElement* a = GetVectorElement(in);
    CVectorElement* b = GetVectorElement(in);
    ret = rs_new_line(1775) CVEPulse(durationA, durationB, a, b);
    break;
  }
  case SBIG('RTOV'): {
    CRealElement* value = GetRealElement(in);
    ret = rs_new_line(1782) CVERealToVector(value);
    break;
  }
  case SBIG('PLOC'): {
    ret = rs_new_line(1788) CVEParticleLocation();
    break;
  }
  case SBIG('PLCO'): {
    ret = rs_new_line(1794) CVEParticlePreviousLocation();
    break;
  }
  case SBIG('PVEL'): {
    ret = rs_new_line(1800) CVEParticleVelocity();
    break;
  }
  case SBIG('PSOF'): {
    ret = rs_new_line(1806) CVEParticleSystemOrientationFront();
    break;
  }
  case SBIG('PSOU'): {
    ret = rs_new_line(1812) CVEParticleSystemOrientationUp();
    break;
  }
  case SBIG('PSOR'): {
    ret = rs_new_line(1818) CVEParticleSystemOrientationRight();
    break;
  }
  case SBIG('PSTR'): {
    ret = rs_new_line(1824) CVEParticleSystemTranslation();
    break;
  }
  case SBIG('SUB_'): {
    CVectorElement* a = GetVectorElement(in);
    CVectorElement* b = GetVectorElement(in);
    ret = rs_new_line(1833) CVESubtract(a, b);
    break;
  }
  case SBIG('CTVC'): {
    CColorElement* value = GetColorElement(in);
    ret = rs_new_line(1841) CVEColorToVector(value);
    break;
  }
#if RSTL_CORRUPTION
  case SBIG('PNCV'): {
    ret = rs_new_line(1847) CVENormalizedCompensatedVelocity();
    break;
  }
  case SBIG('NORM'): {
    CVectorElement* a = GetVectorElement(in);
    ret = rs_new_line(1854) CVENormalize(a);
    break;
  }
  case SBIG('PAP1'): {
    ret = rs_new_line(1860) CVEParticleAccessParameter1();
    break;
  }
  case SBIG('PAP2'): {
    ret = rs_new_line(1866) CVEParticleAccessParameter2();
    break;
  }
  case SBIG('PAP3'): {
    ret = rs_new_line(1872) CVEParticleAccessParameter3();
    break;
  }
  case SBIG('ISWT'): {
    CVectorElement* a = GetVectorElement(in);
    CVectorElement* b = GetVectorElement(in);
    ret = rs_new_line(1880) CVEInitialSwitch(a, b);
    break;
  }
  case SBIG('KPIN'): {
    CVectorElement* a = GetVectorElement(in);
    ret = rs_new_line(1887) CVEKeepInitial(a);
    break;
  }
  case SBIG('PIVL'): {
    ret = rs_new_line(1893) CVEParticleInitialVelocity();
    break;
  }
  case SBIG('PINV'): {
    ret = rs_new_line(1899) CVEParticleInitialNormalizedVelocity();
    break;
  }
  case SBIG('PITR'): {
    ret = rs_new_line(1905) CVEParticleInitialTranslation();
    break;
  }
  case SBIG('PEVL'): {
    ret = rs_new_line(1911) CVEParticleEndVelocity();
    break;
  }
  case SBIG('PENV'): {
    ret = rs_new_line(1917) CVEParticleEndNormalizedVelocity();
    break;
  }
  case SBIG('PETR'): {
    ret = rs_new_line(1923) CVEParticleEndTranslation();
    break;
  }
#else
#if !RSTL_PRIME1
  case SBIG('PNCV'): {
    ret = rs_new CVENormalizedCompensatedVelocity();
    break;
  }
  case SBIG('NORM'): {
    CVectorElement* a = GetVectorElement(in);
    ret = rs_new CVENormalize(a);
    break;
  }
  case SBIG('PAP1'): {
    ret = rs_new CVEParticleAccessParameter1();
    break;
  }
  case SBIG('PAP2'): {
    ret = rs_new CVEParticleAccessParameter2();
    break;
  }
  case SBIG('PAP3'): {
    ret = rs_new CVEParticleAccessParameter3();
    break;
  }
  case SBIG('ISWT'): {
    CVectorElement* a = GetVectorElement(in);
    CVectorElement* b = GetVectorElement(in);
    ret = rs_new CVEInitialSwitch(a, b);
    break;
  }
  case SBIG('KPIN'): {
    CVectorElement* a = GetVectorElement(in);
    ret = rs_new CVEKeepInitial(a);
    break;
  }
  case SBIG('PIVL'): {
    ret = rs_new CVEParticleInitialVelocity();
    break;
  }
  case SBIG('PINV'): {
    ret = rs_new CVEParticleInitialNormalizedVelocity();
    break;
  }
  case SBIG('PITR'): {
    ret = rs_new CVEParticleInitialTranslation();
    break;
  }
  case SBIG('PEVL'): {
    ret = rs_new CVEParticleEndVelocity();
    break;
  }
  case SBIG('PENV'): {
    ret = rs_new CVEParticleEndNormalizedVelocity();
    break;
  }
  case SBIG('PETR'): {
    ret = rs_new CVEParticleEndTranslation();
    break;
  }
#endif

#endif
  default:
    ret = nullptr;
    break;
  }
  return ret;
}

CEmitterElement* CParticleDataFactory::GetEmitterElement(CInputStream& in) {
  CEmitterElement* ret;
  FourCC clsId = GetClassID(in);
  switch (clsId) {
  case SBIG('NONE'):
    ret = nullptr;
    break;
  case SBIG('SETR'): {
    FourCC prop = GetClassID(in);
    CVectorElement* pos = nullptr;
    CVectorElement* vel = nullptr;
    bool valid = false;
    if (prop == SBIG('ILOC')) {
      pos = GetVectorElement(in);
      prop = GetClassID(in);
      if (prop == SBIG('IVEC')) {
        vel = GetVectorElement(in);
        valid = true;
      }
    }
    ret = valid ? rs_new_line(1966) CEESimpleEmitter(pos, vel) : nullptr;
    break;
  }
  case SBIG('SEMR'): {
    CVectorElement* pos = GetVectorElement(in);
    CVectorElement* vel = GetVectorElement(in);
    ret = rs_new_line(1975) CEESimpleEmitter(pos, vel);
    break;
  }
  case SBIG('SPHE'): {
    CVectorElement* origin = GetVectorElement(in);
    CRealElement* radius = GetRealElement(in);
    CRealElement* velocity = GetRealElement(in);
    ret = rs_new_line(1985) CVESphere(origin, radius, velocity);
    break;
  }
#if RSTL_CORRUPTION
  case SBIG('ELPS'): {
    CVectorElement* a = GetVectorElement(in);
    CVectorElement* b = GetVectorElement(in);
    CVectorElement* c = GetVectorElement(in);
    CRealElement* d = GetRealElement(in);
    bool e = GetBool(in);
    ret = rs_new_line(1997) CVEEllipsoid(a, b, c, d, e);
    break;
  }
#else
#if !RSTL_PRIME1
  case SBIG('ELPS'): {
    CVectorElement* a = GetVectorElement(in);
    CVectorElement* b = GetVectorElement(in);
    CVectorElement* c = GetVectorElement(in);
    CRealElement* d = GetRealElement(in);
    bool e = GetBool(in);
    ret = rs_new CVEEllipsoid(a, b, c, d, e);
    break;
  }
#endif

#endif
  case SBIG('ASPH'): {
    CVectorElement* origin = GetVectorElement(in);
    CRealElement* angleXBias = GetRealElement(in);
    CRealElement* angleYBias = GetRealElement(in);
    CRealElement* angleXRange = GetRealElement(in);
    CRealElement* angleYRange = GetRealElement(in);
    CRealElement* radius = GetRealElement(in);
    CRealElement* velocity = GetRealElement(in);
    ret = rs_new_line(2011)
        CVEAngleSphere(origin, radius, velocity, angleXBias, angleYBias, angleXRange, angleYRange);
    break;
  }
#if RSTL_CORRUPTION
  case SBIG('PLNE'): {
    CVectorElement* a = GetVectorElement(in);
    CVectorElement* b = GetVectorElement(in);
    CVectorElement* c = GetVectorElement(in);
    CRealElement* d = GetRealElement(in);
    CRealElement* e = GetRealElement(in);
    CRealElement* f = GetRealElement(in);
    ret = rs_new_line(2024) CEEPlaneEmitter(a, b, c, d, e, f);
    break;
  }
#else
#if !RSTL_PRIME1
  case SBIG('PLNE'): {
    CVectorElement* a = GetVectorElement(in);
    CVectorElement* b = GetVectorElement(in);
    CVectorElement* c = GetVectorElement(in);
    CRealElement* d = GetRealElement(in);
    CRealElement* e = GetRealElement(in);
    CRealElement* f = GetRealElement(in);
    ret = rs_new CEEPlaneEmitter(a, b, c, d, e, f);
    break;
  }
#endif
#endif
  default:
    ret = nullptr;
    break;
  }
  return ret;
}

CModVectorElement* CParticleDataFactory::GetModVectorElement(CInputStream& in) {
  CModVectorElement* ret;
  FourCC clsId = GetClassID(in);
  switch (clsId) {
  case SBIG('NONE'): {
    ret = nullptr;
    break;
  }
  case SBIG('CNST'): {
    CRealElement* a = GetRealElement(in);
    CRealElement* b = GetRealElement(in);
    CRealElement* c = GetRealElement(in);
    if (a && b && c && a->IsConstant() && b->IsConstant() && c->IsConstant()) {
      float af, bf, cf;
#if NONMATCHING
      a->GetValue(0, af);
      b->GetValue(0, bf);
      c->GetValue(0, cf);
#else
      // BUG: Fetching from the same element for each
      a->GetValue(0, af);
      a->GetValue(0, bf);
      a->GetValue(0, cf);
#endif
      ret = rs_new_line(2064) CMVEFastConstant(af, bf, cf);
      delete a;
      delete b;
      delete c;
    } else {
      ret = rs_new_line(2072) CMVEConstant(a, b, c);
    }
    break;
  }
  case SBIG('GRAV'): {
    ret = rs_new_line(2078) CMVEGravity(GetVectorElement(in));
    break;
  }
  case SBIG('WIND'): {
    CVectorElement* a = GetVectorElement(in);
    CRealElement* b = GetRealElement(in);
    ret = rs_new_line(2086) CMVEWind(a, b);
    break;
  }
  case SBIG('EXPL'): {
    CRealElement* a = GetRealElement(in);
    CRealElement* b = GetRealElement(in);
    ret = rs_new_line(2095) CMVEExplode(a, b);
    break;
  }
  case SBIG('CHAN'): {
    CModVectorElement* a = GetModVectorElement(in);
    CModVectorElement* b = GetModVectorElement(in);
    CIntElement* c = GetIntElement(in);
    ret = rs_new_line(2105) CMVETimeChain(a, b, c);
    break;
  }
  case SBIG('PULS'): {
    CIntElement* a = GetIntElement(in);
    CIntElement* b = GetIntElement(in);
    CModVectorElement* c = GetModVectorElement(in);
    CModVectorElement* d = GetModVectorElement(in);
    ret = rs_new_line(2116) CMVEPulse(a, b, c, d);
    break;
  }
  case SBIG('IMPL'): {
    CVectorElement* a = GetVectorElement(in);
    CRealElement* b = GetRealElement(in);
    CRealElement* c = GetRealElement(in);
    CRealElement* d = GetRealElement(in);
    bool e = GetBool(in);
    ret = rs_new_line(2128) CMVEImplosion(a, b, c, d, e);
    break;
  }
  case SBIG('LMPL'): {
    CVectorElement* a = GetVectorElement(in);
    CRealElement* b = GetRealElement(in);
    CRealElement* c = GetRealElement(in);
    CRealElement* d = GetRealElement(in);
    bool e = GetBool(in);
    ret = rs_new_line(2140) CMVELinearImplosion(a, b, c, d, e);
    break;
  }
  case SBIG('EMPL'): {
    CVectorElement* a = GetVectorElement(in);
    CRealElement* b = GetRealElement(in);
    CRealElement* c = GetRealElement(in);
    CRealElement* d = GetRealElement(in);
    bool e = GetBool(in);
    ret = rs_new_line(2152) CMVEExponentialImplosion(a, b, c, d, e);
    break;
  }
  case SBIG('SWRL'): {
    CVectorElement* a = GetVectorElement(in);
    CVectorElement* b = GetVectorElement(in);
    CRealElement* c = GetRealElement(in);
    CRealElement* d = GetRealElement(in);
    ret = rs_new_line(2163) CMVESwirl(a, b, c, d);
    break;
  }
  case SBIG('BNCE'): {
    CVectorElement* a = GetVectorElement(in);
    CVectorElement* b = GetVectorElement(in);
    CRealElement* c = GetRealElement(in);
    CRealElement* d = GetRealElement(in);
#if RSTL_CORRUPTION
    const bool e = GetBool(in);
    ret = rs_new_line(2176) CMVEBounce(a, b, c, d, e);
#else
    bool e = GetBool(in);
    ret = rs_new CMVEBounce(a, b, c, d, e);
#endif
    break;
  }
  case SBIG('SPOS'): {
    ret = rs_new_line(2181) CMVESetPosition(GetVectorElement(in));
    break;
  }
#if RSTL_CORRUPTION
  case SBIG('SPHV'): {
    CVectorElement* a = GetVectorElement(in);
    CRealElement* b = GetRealElement(in);
    CModVectorElement* c = GetModVectorElement(in);
    ret = rs_new_line(2190) CMVESphereVolume(a, b, c);
    break;
  }
  case SBIG('BOXV'): {
    CVectorElement* a = GetVectorElement(in);
    CVectorElement* b = GetVectorElement(in);
    CModVectorElement* c = GetModVectorElement(in);
    ret = rs_new_line(2200) CMVEBoxVolume(a, b, c);
    break;
  }
  case SBIG('SWLC'): {
    CRealElement* a = GetRealElement(in);
    CRealElement* b = GetRealElement(in);
    ret = rs_new_line(2209) CMVESweptLineCollision(a, b);
    break;
  }
#else
#if !RSTL_PRIME1
  case SBIG('SPHV'): {
    CVectorElement* a = GetVectorElement(in);
    CRealElement* b = GetRealElement(in);
    CModVectorElement* c = GetModVectorElement(in);
    ret = rs_new CMVESphereVolume(a, b, c);
    break;
  }
  case SBIG('BOXV'): {
    CVectorElement* a = GetVectorElement(in);
    CVectorElement* b = GetVectorElement(in);
    CModVectorElement* c = GetModVectorElement(in);
    ret = rs_new CMVEBoxVolume(a, b, c);
    break;
  }
#endif

#endif
  default:
    return nullptr;
    break;
  }
  return ret;
}

CColorElement* CParticleDataFactory::GetColorElement(CInputStream& in) {
  CColorElement* ret;
  FourCC clsId = GetClassID(in);
  switch (clsId) {
  case SBIG('CNST'): {
    CElementAllocationChunk* allocationContext = IElement::CElementAllocator::GetCurrentChunk();
    uint initialSize = IElement::CElementAllocator::GetCurrentAllocatedSize();
    CRealElement* r = GetRealElement(in);
    CRealElement* g = GetRealElement(in);
    CRealElement* b = GetRealElement(in);
    CRealElement* a = GetRealElement(in);
    if (r && g && b && a) {
      if (r->IsConstant() && g->IsConstant() && b->IsConstant() && a->IsConstant()) {
        float rf, gf, bf, af;
        r->GetValue(0, rf);
        g->GetValue(0, gf);
        b->GetValue(0, bf);
        a->GetValue(0, af);

        delete r;
        delete g;
        delete b;
        delete a;

        if (allocationContext != nullptr &&
            allocationContext == IElement::CElementAllocator::GetCurrentChunk()) {
          allocationContext->Rewind(allocationContext->GetAllocatedSize() - initialSize);
        }
        ret = rs_new_line(2261) CCEFastConstant(rf, gf, bf, af);
        break;
      }
    }
    ret = rs_new_line(2265) CCEConstant(r, g, b, a);
    break;
  }
  case SBIG('KEYE'):
  case SBIG('KEYP'):
    ret = rs_new_line(2272) CCEKeyframeEmitter(in);
    break;
#if RSTL_CORRUPTION
  case SBIG('KEYF'): {
    ret = rs_new_line(2276) CCEKeyframeInput(in);
    break;
  }
#else
#if !RSTL_PRIME1
  case SBIG('KEYF'): {
    ret = rs_new CCEKeyframeInput(in);
    break;
  }
#endif

#endif
  case SBIG('FADE'): {
    CColorElement* a = GetColorElement(in);
    CColorElement* b = GetColorElement(in);
    CRealElement* end = GetRealElement(in);
    ret = rs_new_line(2285) CCEFade(a, b, end);
    break;
  }
  case SBIG('CFDE'): {
    CColorElement* a = GetColorElement(in);
    CColorElement* b = GetColorElement(in);
    CRealElement* start = GetRealElement(in);
    CRealElement* end = GetRealElement(in);
    ret = rs_new_line(2296) CCEFadeEnd(a, b, start, end);
    break;
  }
  case SBIG('CHAN'): {
    CColorElement* a = GetColorElement(in);
    CColorElement* b = GetColorElement(in);
    CIntElement* frame = GetIntElement(in);
    ret = rs_new_line(2306) CCETimeChain(a, b, frame);
    break;
  }
  case SBIG('PULS'): {
    CIntElement* aDuration = GetIntElement(in);
    CIntElement* bDuration = GetIntElement(in);
    CColorElement* a = GetColorElement(in);
    CColorElement* b = GetColorElement(in);
    ret = rs_new_line(2317) CCEPulse(aDuration, bDuration, a, b);
    break;
  }
  case SBIG('PCOL'):
    ret = rs_new_line(2323) CCEParticleColor();
    break;
#if RSTL_CORRUPTION
  case SBIG('MULT'): {
    CColorElement* a = GetColorElement(in);
    CColorElement* b = GetColorElement(in);
    ret = rs_new_line(2331) CCEMultiply(a, b);
    break;
  }
  case SBIG('VRTC'): {
    CVectorElement* a = GetVectorElement(in);
    CRealElement* b = GetRealElement(in);
    ret = rs_new_line(2339) CCEVectorAndRealToColor(a, b);
    break;
  }
  case SBIG('ISWT'): {
    CColorElement* a = GetColorElement(in);
    CColorElement* b = GetColorElement(in);
    ret = rs_new_line(2347) CCEInitialSwitch(a, b);
    break;
  }
  case SBIG('KPIN'): {
    CColorElement* a = GetColorElement(in);
    ret = rs_new_line(2354) CCEKeepInitial(a);
    break;
  }
  case SBIG('MDAO'): {
    CColorElement* a = GetColorElement(in);
    CRealElement* b = GetRealElement(in);
    ret = rs_new_line(2362) CCEModifyAlphaOnly(a, b);
    break;
  }
#else
#if !RSTL_PRIME1
  case SBIG('MULT'): {
    CColorElement* a = GetColorElement(in);
    CColorElement* b = GetColorElement(in);
    ret = rs_new CCEMultiply(a, b);
    break;
  }
  case SBIG('VRTC'): {
    CVectorElement* a = GetVectorElement(in);
    CRealElement* b = GetRealElement(in);
    ret = rs_new CCEVectorAndRealToColor(a, b);
    break;
  }
  case SBIG('ISWT'): {
    CColorElement* a = GetColorElement(in);
    CColorElement* b = GetColorElement(in);
    ret = rs_new CCEInitialSwitch(a, b);
    break;
  }
  case SBIG('KPIN'): {
    CColorElement* a = GetColorElement(in);
    ret = rs_new CCEKeepInitial(a);
    break;
  }
  case SBIG('MDAO'): {
    CColorElement* a = GetColorElement(in);
    CRealElement* b = GetRealElement(in);
    ret = rs_new CCEModifyAlphaOnly(a, b);
    break;
  }
#endif

#endif
  case SBIG('NONE'):
    ret = nullptr;
    break;
  default:
    ret = nullptr;
    break;
  }
  return ret;
}

CUVElement* CParticleDataFactory::GetTextureElement(CInputStream& in, CSimplePool* resPool) {
  CUVElement* ret;
  FourCC clsId = GetClassID(in);
  switch (clsId) {
  case SBIG('NONE'):
    ret = nullptr;
    break;
  case SBIG('CNST'): {
#if RSTL_CORRUPTION
    CAssetId id = kInvalidAssetId;
#else
    CAssetId id = 0;
#endif
    FourCC subId = GetClassID(in);
    if (subId != SBIG('NONE')) {
#if RSTL_CORRUPTION
      id = CAssetId(in);
#else
      id = in.ReadLong();

#endif
    }
#if RSTL_CORRUPTION
    if (id == kInvalidAssetId) {
#else
    if (id == 0) {
#endif
      TToken< CTexture > tex = CreateTexture(-1);
      ret = rs_new_line(2404) CUVEConstant(tex);
    } else {
      TToken< CTexture > tex = resPool->GetObj(SObjectTag(SBIG('TXTR'), id));
      ret = rs_new_line(2409) CUVEConstant(tex);
    }
    break;
  }
  case SBIG('ATEX'): {
#if RSTL_CORRUPTION
    CAssetId id = kInvalidAssetId;
#else
    CAssetId id = 0;
#endif
    FourCC subId = GetClassID(in);
    if (subId != SBIG('NONE')) {
#if RSTL_CORRUPTION
      id = CAssetId(in);
#else
      id = in.ReadLong();

#endif
    }
    CIntElement* tileW = GetIntElement(in);
    CIntElement* tileH = GetIntElement(in);
    CIntElement* strideW = GetIntElement(in);
    CIntElement* strideH = GetIntElement(in);
    CIntElement* cycleFrames = GetIntElement(in);
    bool loop = GetBool(in);
#if RSTL_CORRUPTION
    if (id == kInvalidAssetId) {
#else
    if (id == 0) {
#endif
      TToken< CTexture > tex = CreateTexture(-1);
      ret =
          rs_new_line(2435) CUVEAnimTexture(tex, tileW, tileH, strideW, strideH, cycleFrames, loop);
    } else {
      TToken< CTexture > tex = resPool->GetObj(SObjectTag(SBIG('TXTR'), id));
#if RSTL_CORRUPTION
      ret =
          rs_new_line(2440) CUVEAnimTexture(tex, tileW, tileH, strideW, strideH, cycleFrames, loop);
    }
    break;
  }
  case SBIG('TEXP'): {
    CAssetId id = kInvalidAssetId;
    FourCC subId = GetClassID(in);
    if (subId != SBIG('NONE')) {
      id = CAssetId(in);
    }
    CIntElement* a = GetIntElement(in);
    CIntElement* b = GetIntElement(in);
    CRealElement* c = GetRealElement(in);
    if (id == kInvalidAssetId) {
      TToken< CTexture > tex = CreateTexture(-1);
      ret = rs_new_line(2462) CUVETEXP(tex, a, b, c);
    } else {
      TToken< CTexture > tex = resPool->GetObj(SObjectTag(SBIG('TXTR'), id));
      ret = rs_new_line(2467) CUVETEXP(tex, a, b, c);
#else
      ret = rs_new CUVEAnimTexture(tex, tileW, tileH, strideW, strideH, cycleFrames, loop);
#endif
    }
    break;
  }
  default:
    return nullptr;
  }
  return ret;
}

rstl::optional_object< TToken< CGenDescription > >
CParticleDataFactory::GetChildGeneratorDesc(CInputStream& in, CSimplePool* pool,
                                            const rstl::vector< CAssetId >& resources) {
  FourCC clsId = GetClassID(in);
  CAssetId id;
  if (clsId != SBIG('NONE')) {
#if RSTL_CORRUPTION
    id = CAssetId(in);
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    id = in.Get< CAssetId >();
#else
    id = in.ReadInt32();
#endif

#endif
  } else {
#if RSTL_CORRUPTION
    return rstl::optional_object< TToken< CGenDescription > >();
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    return rstl::optional_object_null();
#else
    return rstl::optional_object< TToken< CGenDescription > >();
#endif

#endif
  }
#if RSTL_CORRUPTION
  if (id == kInvalidAssetId) {
    return rstl::optional_object< TToken< CGenDescription > >();
#else
  if (id == 0) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    return rstl::optional_object_null();
#else
    return rstl::optional_object< TToken< CGenDescription > >();
#endif

#endif
  }
  return GetChildGeneratorDesc(id, pool, resources);
}

rstl::optional_object< TToken< CGenDescription > >
CParticleDataFactory::GetChildGeneratorDesc(CAssetId id, CSimplePool* pool,
                                            const rstl::vector< CAssetId >& resources) {
  if (rstl::count(resources.begin(), resources.end(), id) == 0) {
#if RSTL_CORRUPTION
    return TToken< CGenDescription >(pool->GetObj(SObjectTag('PART', id)));
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    return TToken< CGenDescription >(pool->GetObj(SObjectTag(CParticleGen::ResType(), id)));
#else
    return TToken< CGenDescription >(pool->GetObj(SObjectTag('PART', id)));
#endif

#endif
  }
#if RSTL_CORRUPTION
  return rstl::optional_object< TToken< CGenDescription > >();
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return rstl::optional_object_null();
#else
  return rstl::optional_object< TToken< CGenDescription > >();
#endif

#endif
}

rstl::optional_object< TToken< CSwooshDescription > >
CParticleDataFactory::GetSwooshGeneratorDesc(CInputStream& in, CSimplePool* pool) {
  FourCC clsId = GetClassID(in);
  CAssetId id;
  if (clsId != SBIG('NONE')) {
#if RSTL_CORRUPTION
    id = CAssetId(in);
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    id = in.Get< CAssetId >();
#else
    id = in.ReadInt32();
#endif

#endif
  } else {
#if RSTL_CORRUPTION
    return rstl::optional_object< TToken< CSwooshDescription > >();
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    return rstl::optional_object_null();
#else
    return rstl::optional_object< TToken< CSwooshDescription > >();
#endif

#endif
  }
#if RSTL_CORRUPTION
  if (id == kInvalidAssetId) {
    return rstl::optional_object< TToken< CSwooshDescription > >();
#else
  if (id == 0) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    return rstl::optional_object_null();
#else
    return rstl::optional_object< TToken< CSwooshDescription > >();
#endif

#endif
  }
  return TToken< CSwooshDescription >(pool->GetObj(SObjectTag(SBIG('SWHC'), id)));
}

rstl::optional_object< TToken< CElectricDescription > >
CParticleDataFactory::GetElectricGeneratorDesc(CInputStream& in, CSimplePool* pool) {
  FourCC clsId = GetClassID(in);
  CAssetId id;
  if (clsId != SBIG('NONE')) {
#if RSTL_CORRUPTION
    id = CAssetId(in);
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    id = in.Get< CAssetId >();
#else
    id = in.ReadInt32();
#endif

#endif
  } else {
#if RSTL_CORRUPTION
    return rstl::optional_object< TToken< CElectricDescription > >();
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    return rstl::optional_object_null();
#else
    return rstl::optional_object< TToken< CElectricDescription > >();
#endif

#endif
  }
#if RSTL_CORRUPTION
  if (id == kInvalidAssetId) {
    return rstl::optional_object< TToken< CElectricDescription > >();
#else
  if (id == 0) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    return rstl::optional_object_null();
#else
    return rstl::optional_object< TToken< CElectricDescription > >();
#endif

#endif
  }
  return TToken< CElectricDescription >(pool->GetObj(SObjectTag(SBIG('ELSC'), id)));
}

rstl::optional_object< TToken< CModel > > CParticleDataFactory::GetModel(CInputStream& in,
                                                                         CSimplePool* pool) {
  FourCC clsId = GetClassID(in);
  CAssetId id;
  if (clsId != SBIG('NONE')) {
#if RSTL_CORRUPTION
    id = CAssetId(in);
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    id = in.Get< CAssetId >();
#else
    id = in.ReadInt32();
#endif

#endif
  } else {
#if RSTL_CORRUPTION
    return rstl::optional_object< TToken< CModel > >();
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    return rstl::optional_object_null();
#else
    return rstl::optional_object< TToken< CModel > >();
#endif

#endif
  }
#if RSTL_CORRUPTION
  if (id == kInvalidAssetId) {
    return rstl::optional_object< TToken< CModel > >();
#else
  if (id == 0) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    return rstl::optional_object_null();
#else
    return rstl::optional_object< TToken< CModel > >();
#endif

#endif
  }
  return TToken< CModel >(pool->GetObj(SObjectTag(SBIG('CMDL'), id)));
}

CTexture* CreateTexture(int value) {
  CTexture* texture = rs_new_line(2601) CTexture(kTexelFormat_RGBA8, 4, 4, 1);
  int* data = static_cast< int* >(texture->Lock());
  for (int i = 1; i <= 16; ++i) {
    data[i - 1] = value;
  }
  texture->UnLock();
  return texture;
}
