#ifndef _CELECTRICDESCRIPTION
#define _CELECTRICDESCRIPTION
#include "rstl/RstlVersions.h"

#include "Kyoto/Particles/CGenDescription.hpp"
#include "Kyoto/Particles/CParticleDataFactory.hpp"
#include "Kyoto/Particles/CSwooshDescription.hpp"
#include "types.h"

#include "Kyoto/TToken.hpp"
#include "rstl/optional_object.hpp"

class CColorElement;
class CEmitterElement;
class CGenDescription;

class CIntElement;
class CRealElement;
class CSwooshDescription;
class CUVElement;

class CElectricDescription {
public:
  CElectricDescription();
  ~CElectricDescription();

  CIntElement* mLIFE;
  CIntElement* mSLIF;
  CRealElement* mGRAT;
  CIntElement* mSCNT;
  CIntElement* mSSEG;
  CColorElement* mCOLR;
  CEmitterElement* mIEMT;
  CEmitterElement* mFEMT;
  CRealElement* mAMPL;
  CRealElement* mAMPD;
  CRealElement* mLWD1;
  CRealElement* mLWD2;
  CRealElement* mLWD3;
  CColorElement* mLCL1;
  CColorElement* mLCL2;
  CColorElement* mLCL3;
#if !RSTL_PRIME1
  CUVElement* mTEXR;
  int mDFLG;
#endif

  rstl::optional_object< TCachedToken< CSwooshDescription > > mSSWH;
  rstl::optional_object< TCachedToken< CGenDescription > > mGPSM;
  rstl::optional_object< TCachedToken< CGenDescription > > mEPSM;
  bool mZERY;
};
CHECK_SIZEOF(CElectricDescription, RSTL_PRIME1 ? 0x74 : 0x7c)

#endif // _CELECTRICDESCRIPTION
