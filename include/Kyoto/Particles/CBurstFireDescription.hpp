#ifndef _CBURSTFIREDESCRIPTION
#define _CBURSTFIREDESCRIPTION

#include "rstl/static_assert.hpp"
#include "Kyoto/Particles/IElement.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/types.h"


// Guessed name
class CBurstFireDescription {
public:
  CBurstFireDescription();
  ~CBurstFireDescription();

  rstl::single_ptr< CRealElement > mOVLH;
  rstl::single_ptr< CRealElement > mOVLW;
  rstl::single_ptr< CRealElement > mSHVR;
  rstl::single_ptr< CIntElement > mBTMI;
  rstl::reserved_vector< rstl::single_ptr< CRealElement >, 8 > mBurstCH;
  rstl::reserved_vector< rstl::single_ptr< CRealElement >, 8 > mBurstVL;
};
CHECK_SIZEOF(CBurstFireDescription, 0x58)


#endif // _CBURSTFIREDESCRIPTION
