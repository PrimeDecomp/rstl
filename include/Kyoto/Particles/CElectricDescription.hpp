#ifndef _CELECTRICDESCRIPTION
#define _CELECTRICDESCRIPTION
#include "rstl/RstlVersions.h"

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#include "Kyoto/Particles/CGenDescription.hpp"
#include "Kyoto/Particles/CParticleDataFactory.hpp"
#include "Kyoto/Particles/CSwooshDescription.hpp"
#else
#include "types.h"
#endif

#include "Kyoto/TToken.hpp"
#include "rstl/optional_object.hpp"

class CColorElement;
class CEmitterElement;
#if !RSTL_PRIME1
class CGenDescription;
#endif

class CIntElement;
class CRealElement;
#if !RSTL_PRIME1
class CSwooshDescription;
class CUVElement;
#endif

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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CHECK_SIZEOF(CElectricDescription, 0x74)
#else
CHECK_SIZEOF(CElectricDescription, 0x7c)
#endif

#endif // _CELECTRICDESCRIPTION
