#include "rstl/RstlVersions.h"
#include "Kyoto/Particles/CSwooshDescription.hpp"
#include "Kyoto/Particles/CColorElement.hpp"
#include "Kyoto/Particles/CIntElement.hpp"
#include "Kyoto/Particles/CModVectorElement.hpp"
#include "Kyoto/Particles/CRealElement.hpp"
#include "Kyoto/Particles/CUVElement.hpp"
#include "Kyoto/Particles/CVectorElement.hpp"

CSwooshDescription::CSwooshDescription()
: mPSLT(nullptr)
, mTIME(nullptr)
, mLRAD(nullptr)
, mRRAD(nullptr)
, mLENG(nullptr)
, mCOLR(nullptr)
, mSIDE(nullptr)
, mIROT(nullptr)
, mROTM(nullptr)
, mPOFS(nullptr)
, mIVEL(nullptr)
, mNPOS(nullptr)
, mVELM(nullptr)
, mVLM2(nullptr)
, mSPLN(nullptr)
, mTEXR(nullptr)
, mTSPN(nullptr)
, mLLRD(false)
, mCROS(true)
, mVLS1(false)
, mVLS2(false)
, mSROT(false)
, mWIRE(false)
, mTEXW(false)
#if !RSTL_PRIME1
, mCLTX(false)
#endif

, mAALP(false)
, mZBUF(false)
, mORNT(false)
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
, mCRND(false) {}
#else
, mCRND(false)
, mDFLG(0) {}
#endif

CSwooshDescription::~CSwooshDescription() {
  delete mPSLT;
  delete mTIME;
  delete mLRAD;
  delete mRRAD;
  delete mLENG;
  delete mCOLR;
  delete mSIDE;
  delete mIROT;
  delete mROTM;
  delete mPOFS;
  delete mIVEL;
  delete mNPOS;
  delete mVELM;
  delete mVLM2;
  delete mSPLN;
  delete mTEXR;
  delete mTSPN;
}

