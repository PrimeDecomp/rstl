#ifndef _CGENDESCRIPTION
#define _CGENDESCRIPTION
#include "rstl/RstlVersions.h"
#include "types.h"

#include "Kyoto/CToken.hpp"
#include "Kyoto/Particles/CSpawnSystemKeyframeData.hpp"

#include "Kyoto/Particles/IElement.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/single_ptr.hpp"

class CElectricDescription;
class CModel;
#if !RSTL_PRIME1
class CSpawnSystemKeyframeData;
#endif

class CSwooshDescription;

class CGenDescription {
public:
#if !RSTL_PRIME1
  typedef rstl::optional_object< TCachedToken< CModel > > TParticleModel;
  typedef rstl::optional_object< TCachedToken< CGenDescription > > TChildGeneratorDesc;
  typedef rstl::optional_object< TCachedToken< CSwooshDescription > > TSwooshGeneratorDesc;
  typedef rstl::optional_object< CToken > TElectricGeneratorDesc;
#endif

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
  bool mAAPH : 1;
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
#if !RSTL_PRIME1
  bool mINDM : 1;
#endif

  bool mOPTS : 1;
#if !RSTL_PRIME1
  bool mRDOP : 1;
  bool mVMPC : 1;
#endif

  CIntElement* mMBSP;
  CRealElement* mSIZE;
  CRealElement* mROTA;
  CUVElement* mTEXR;
  CUVElement* mTIND;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  rstl::optional_object< TCachedToken< CModel > > mPMDL;
#else
  CIntElement* mXTAD;
  CRealElement* mFXBR;
  CVectorElement* mFXBO;
  TParticleModel mPMDL;
#endif

  CVectorElement* mPMOP;
  CVectorElement* mPMRT;
  CVectorElement* mPMSC;
  CColorElement* mPMCL;
#if !RSTL_PRIME1
  CVectorElement* mPMOV;
#endif

  CModVectorElement* mVEL1;
  CModVectorElement* mVEL2;
  CModVectorElement* mVEL3;
  CModVectorElement* mVEL4;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  rstl::optional_object< TCachedToken< CGenDescription > > mICTS;
#else
  TChildGeneratorDesc mICTS;
#endif

  CIntElement* mNCSY;
  CIntElement* mCSSD;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  rstl::optional_object< TCachedToken< CGenDescription > > mIDTS;
#else
  TChildGeneratorDesc mIDTS;
#endif

  CIntElement* mNDSY;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  rstl::optional_object< TCachedToken< CGenDescription > > mIITS;
#else
  TChildGeneratorDesc mIITS;
#endif

  CIntElement* mPISY;
  CIntElement* mSISY;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  rstl::single_ptr< CSpawnSystemKeyframeData > mKSSM;
  rstl::optional_object< TCachedToken< CSwooshDescription > > mSSWH;
#else
  CSpawnSystemKeyframeData* mKSSM;
  TSwooshGeneratorDesc mSSWH;
#endif

  CIntElement* mSSSD;
  CVectorElement* mSSPO;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  rstl::optional_object< CToken > mSELC;
#else
  TElectricGeneratorDesc mSELC;
#endif

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
#if !RSTL_PRIME1
  CRealElement* mADV9;
  CVectorElement* mVAV1;
  CVectorElement* mVAV2;
  CVectorElement* mVAV3;
  int mDFLG;
#endif

};
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CHECK_SIZEOF(CGenDescription, 0x12c)
#else
CHECK_SIZEOF(CGenDescription, 0x150)
#endif

#endif // _CGENDESCRIPTION
