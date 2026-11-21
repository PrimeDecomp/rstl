#ifndef _CGENDESCRIPTION
#define _CGENDESCRIPTION
#include "rstl/RstlVersions.h"
#include "rstl/static_assert.hpp"
#if !RSTL_CORRUPTION
#include "rstl/RstlVersions.h"
#endif
#include "rstl/types.h"

#include "Kyoto/CToken.hpp"
#if !RSTL_CORRUPTION
#include "Kyoto/Particles/CSpawnSystemKeyframeData.hpp"

#endif
#include "Kyoto/Particles/IElement.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/optional_object.hpp"
#if !RSTL_CORRUPTION
#include "rstl/single_ptr.hpp"
#endif

class CElectricDescription;
class CModel;
class CSpawnSystemKeyframeData;

class CSwooshDescription;

class CGenDescription {
public:
  typedef rstl::optional_object< TCachedToken< CModel > > TParticleModel;
  typedef rstl::optional_object< TCachedToken< CGenDescription > > TChildGeneratorDesc;
  typedef rstl::optional_object< TCachedToken< CSwooshDescription > > TSwooshGeneratorDesc;
  typedef rstl::optional_object< CToken > TElectricGeneratorDesc;

  CGenDescription();
  ~CGenDescription();

  CIntElement* mPSLT;
  CIntElement* mPSWT;
  CRealElement* mPSTS;
  CVectorElement* mPOFS;
  CIntElement* mSEED;
  CRealElement* mLENG;
  CRealElement* mWIDT;
  CIntElement* mMAXP;
  CRealElement* mGRTE;
  CColorElement* mCOLR;
  CIntElement* mLTME;
  CEmitterElement* mEMTR;
  bool mLINE : 1;
  bool mFXLL : 1;
#if !RSTL_CORRUPTION
  bool mAAPH : 1;
#endif
  bool mZBUF : 1;
  bool mSORT : 1;
  bool mLIT_ : 1;
  bool mORNT : 1;
  bool mRSOP : 1;
  bool mMBLR : 1;
  bool mPMAB : 1;
  bool mPMUS : 1;
  bool mPMOO : 1;
  bool mVMD1 : 1;
  bool mVMD2 : 1;
  bool mVMD3 : 1;
  bool mVMD4 : 1;
  bool mCIND : 1;
#if RSTL_CORRUPTION
  bool mINDM : 1;
#else
#if !RSTL_PRIME1
  bool mINDM : 1;
#endif

#endif
  bool mOPTS : 1;
#if RSTL_CORRUPTION
  bool mRDOP : 1;
  bool mVMPC : 1;
#else
#if RSTL_PARTICLE_RDOP
  bool mRDOP : 1;
#endif
#if !RSTL_PRIME1
  bool mVMPC : 1;
#endif

#endif
  CIntElement* mMBSP;
  CRealElement* mSIZE;
#if RSTL_CORRUPTION
  CRealElement* mISVF;
  CRealElement* mSVEO;
#endif
  CRealElement* mROTA;
  CUVElement* mTEXR;
  CUVElement* mTIND;
#if RSTL_CORRUPTION
  CIntElement* mPBDM;
  CIntElement* mXTAD;
  CRealElement* mFXBR;
  CVectorElement* mFXBO;
#else
#if !RSTL_PRIME1
  CIntElement* mXTAD;
#endif
#if RSTL_PARTICLE_BOUNDS_MODES
  CRealElement* mFXBR;
  CVectorElement* mFXBO;
#endif
#endif
  TParticleModel mPMDL;

  CVectorElement* mPMOP;
  CVectorElement* mPMRT;
  CVectorElement* mPMSC;
  CColorElement* mPMCL;
#if RSTL_CORRUPTION
  CVectorElement* mPMOV;
#else
#if !RSTL_PRIME1
  CVectorElement* mPMOV;
#endif

#endif
  CModVectorElement* mVEL1;
  CModVectorElement* mVEL2;
  CModVectorElement* mVEL3;
  CModVectorElement* mVEL4;
  TChildGeneratorDesc mICTS;

  CIntElement* mNCSY;
  CIntElement* mCSSD;
  TChildGeneratorDesc mIDTS;

  CIntElement* mNDSY;
  TChildGeneratorDesc mIITS;

  CIntElement* mPISY;
  CIntElement* mSISY;
#if RSTL_CORRUPTION
  CSpawnSystemKeyframeData* mKSSM;
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  rstl::single_ptr< CSpawnSystemKeyframeData > mKSSM;
#else
  CSpawnSystemKeyframeData* mKSSM;
#endif
#endif
  TSwooshGeneratorDesc mSSWH;

  CIntElement* mSSSD;
  CVectorElement* mSSPO;
  TElectricGeneratorDesc mSELC;

  CIntElement* mSESD;
  CVectorElement* mSEPO;
  CIntElement* mLTYP;
  CColorElement* mLCLR;
  CRealElement* mLINT;
  CVectorElement* mLOFF;
  CVectorElement* mLDIR;
  CIntElement* mLFOT;
  CRealElement* mLFOR;
  CRealElement* mLSLA;
  CRealElement* mADV1;
  CRealElement* mADV2;
  CRealElement* mADV3;
  CRealElement* mADV4;
  CRealElement* mADV5;
  CRealElement* mADV6;
  CRealElement* mADV7;
  CRealElement* mADV8;
#if RSTL_CORRUPTION
  CRealElement* mADV9;
  CVectorElement* mVAV1;
  CVectorElement* mVAV2;
  CVectorElement* mVAV3;
  int mDFLG;
#else
#if !RSTL_PRIME1
  CRealElement* mADV9;
  CVectorElement* mVAV1;
  CVectorElement* mVAV2;
  CVectorElement* mVAV3;
  int mDFLG;
#endif

#endif
};
#if RSTL_CORRUPTION
CHECK_SIZEOF(CGenDescription, 0x15C)
#else
CHECK_SIZEOF(CGenDescription, RSTL_PRIME1 ? 0x12c : RSTL_PARTICLE_BOUNDS_MODES ? 0x150 : 0x148)
#endif

#endif // _CGENDESCRIPTION
