#ifndef _CSWOOSHDESCRIPTION
#define _CSWOOSHDESCRIPTION
#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"
#endif

#if RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#endif
#include "rstl/types.h"

class CColorElement;
class CIntElement;
class CModVectorElement;
class CRealElement;
class CUVElement;
class CVectorElement;
class CSwooshDescription {
public:
  CSwooshDescription();
  ~CSwooshDescription();

  CIntElement* mPSLT;
  CRealElement* mTIME;
  CRealElement* mLRAD;
  CRealElement* mRRAD;
  CIntElement* mLENG;
  CColorElement* mCOLR;
  CIntElement* mSIDE;
#if RSTL_CORRUPTION
  CRealElement* mZROT;
  CRealElement* mYROT;
  CRealElement* mXROT;
#endif
  CRealElement* mIROT;
  CRealElement* mROTM;
  CVectorElement* mPOFS;
  CVectorElement* mIVEL;
  CVectorElement* mNPOS;
  CModVectorElement* mVELM;
  CModVectorElement* mVLM2;
  CIntElement* mSPLN;
  CUVElement* mTEXR;
  CIntElement* mTSPN;
  bool mLLRD : 1;
  bool mCROS : 1;
  bool mVLS1 : 1;
  bool mVLS2 : 1;
  bool mSROT : 1;
  bool mWIRE : 1;
  bool mTEXW : 1;
#if !RSTL_PRIME1
  bool mCLTX : 1;
#endif

  bool mAALP : 1;
  bool mZBUF : 1;
  bool mORNT : 1;
  bool mCRND : 1;
#if !RSTL_PRIME1
  int mDFLG;
#endif

};
#if RSTL_CORRUPTION
CHECK_SIZEOF(CSwooshDescription, 0x58)
#else
#if !RSTL_PRIME1
CHECK_SIZEOF(CSwooshDescription, 0x4c)
#endif
#endif

#endif // _CSWOOSHDESCRIPTION
