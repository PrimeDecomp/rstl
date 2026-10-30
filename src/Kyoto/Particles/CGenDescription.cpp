#include "rstl/RstlVersions.h"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "Kyoto/Particles/CSpawnSystemKeyframeData.hpp"

static IElement** element_from_desc(CGenDescription* gen, int idx) {
  return reinterpret_cast< IElement** >(reinterpret_cast< uchar* >(gen) + idx);
}

static const size_t skGenOffsets[] = {
    offsetof(CGenDescription, mPSLT),   offsetof(CGenDescription, mPSWT),
    offsetof(CGenDescription, mPSTS),   offsetof(CGenDescription, mPOFS),
    offsetof(CGenDescription, mSEED),  offsetof(CGenDescription, mLENG),
    offsetof(CGenDescription, mWIDT),  offsetof(CGenDescription, mMAXP),
    offsetof(CGenDescription, mGRTE),  offsetof(CGenDescription, mCOLR),
    offsetof(CGenDescription, mLTME),  offsetof(CGenDescription, mEMTR),
    offsetof(CGenDescription, mMBSP),  offsetof(CGenDescription, mSIZE),
    offsetof(CGenDescription, mROTA),  offsetof(CGenDescription, mTEXR),
    offsetof(CGenDescription, mTIND),
#if !RSTL_PRIME1
    offsetof(CGenDescription, mXTAD),
    offsetof(CGenDescription, mFXBR),
    offsetof(CGenDescription, mFXBO),
#endif
    offsetof(CGenDescription, mPMOP),
    offsetof(CGenDescription, mPMRT),
    offsetof(CGenDescription, mPMSC),
    offsetof(CGenDescription, mPMCL),
#if !RSTL_PRIME1
    offsetof(CGenDescription, mPMOV),
#endif
    offsetof(CGenDescription, mVEL1),
    offsetof(CGenDescription, mVEL2),  offsetof(CGenDescription, mVEL3),
    offsetof(CGenDescription, mVEL4),  offsetof(CGenDescription, mNCSY),
    offsetof(CGenDescription, mCSSD),  offsetof(CGenDescription, mNDSY),
    offsetof(CGenDescription, mPISY),  offsetof(CGenDescription, mSISY),
    offsetof(CGenDescription, mSSSD),  offsetof(CGenDescription, mSSPO),
    offsetof(CGenDescription, mSESD),  offsetof(CGenDescription, mSEPO),
    offsetof(CGenDescription, mLTYP),  offsetof(CGenDescription, mLCLR),
    offsetof(CGenDescription, mLINT),  offsetof(CGenDescription, mLOFF),
    offsetof(CGenDescription, mLDIR),  offsetof(CGenDescription, mLFOT),
    offsetof(CGenDescription, mLFOR), offsetof(CGenDescription, mLSLA),
    offsetof(CGenDescription, mADV1), offsetof(CGenDescription, mADV2),
    offsetof(CGenDescription, mADV3), offsetof(CGenDescription, mADV4),
    offsetof(CGenDescription, mADV5), offsetof(CGenDescription, mADV6),
    offsetof(CGenDescription, mADV7), offsetof(CGenDescription, mADV8),
#if !RSTL_PRIME1
    offsetof(CGenDescription, mADV9),  offsetof(CGenDescription, mVAV1),
    offsetof(CGenDescription, mVAV2),  offsetof(CGenDescription, mVAV3),
#endif
};

CGenDescription::CGenDescription()
: mPSLT(nullptr)
, mPSWT(nullptr)
, mPSTS(nullptr)
, mPOFS(nullptr)
, mSEED(nullptr)
, mLENG(nullptr)
, mWIDT(nullptr)
, mMAXP(nullptr)
, mGRTE(nullptr)
, mCOLR(nullptr)
, mLTME(nullptr)
, mEMTR(nullptr)
, mLINE(false)
, mFXLL(false)
, mAAPH(false)
, mZBUF(false)
, mSORT(false)
, mLIT_(false)
, mORNT(false)
, mRSOP(false)
, mMBLR(false)
, mPMAB(false)
, mPMUS(false)
, mPMOO(true)
, mVMD1(false)
, mVMD2(false)
, mVMD3(false)
, mVMD4(false)
, mCIND(false)
#if !RSTL_PRIME1
, mINDM(false)
#endif

, mOPTS(false)
#if !RSTL_PRIME1
, mRDOP(true)
, mVMPC(false)
#endif

, mMBSP(nullptr)
, mSIZE(nullptr)
, mROTA(nullptr)
, mTEXR(nullptr)
, mTIND(nullptr)
#if !RSTL_PRIME1
, mXTAD(nullptr)
, mFXBR(nullptr)
, mFXBO(nullptr)
#endif

, mPMOP(nullptr)
, mPMRT(nullptr)
, mPMSC(nullptr)
, mPMCL(nullptr)
#if !RSTL_PRIME1
, mPMOV(nullptr)
#endif

, mVEL1(nullptr)
, mVEL2(nullptr)
, mVEL3(nullptr)
, mVEL4(nullptr)
, mNCSY(nullptr)
, mCSSD(nullptr)
, mNDSY(nullptr)
, mPISY(nullptr)
, mSISY(nullptr)
#if !RSTL_PRIME1
, mKSSM(nullptr)
#endif

, mSSSD(nullptr)
, mSSPO(nullptr)
, mSESD(nullptr)
, mSEPO(nullptr)
, mLTYP(nullptr)
, mLCLR(nullptr)
, mLINT(nullptr)
, mLOFF(nullptr)
, mLDIR(nullptr)
, mLFOT(nullptr)
, mLFOR(nullptr)
, mLSLA(nullptr)
, mADV1(nullptr)
, mADV2(nullptr)
, mADV3(nullptr)
, mADV4(nullptr)
, mADV5(nullptr)
, mADV6(nullptr)
, mADV7(nullptr)
, mADV8(nullptr)
#if !RSTL_PRIME1
, mADV9(nullptr)
, mVAV1(nullptr)
, mVAV2(nullptr)
, mVAV3(nullptr)
, mDFLG(0)
#endif
{}

CGenDescription::~CGenDescription() {
  for (int i = 0; i < ARRAY_SIZE(skGenOffsets); ++i) {
    delete *element_from_desc(this, skGenOffsets[i]);
  }
#if !RSTL_PRIME1
  delete mKSSM;
#endif

}

