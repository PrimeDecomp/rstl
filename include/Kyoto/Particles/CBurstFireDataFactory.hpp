#ifndef _CBURSTFIREDATAFACTORY
#define _CBURSTFIREDATAFACTORY

#include "Kyoto/CFactoryMgr.hpp"

class CBurstFireDescription;
class CInputStream;
class CSimplePool;
class CVParamTransfer;

class CBurstFireDataFactory {
public:
  // Guessed name
  static CBurstFireDescription* GetDescription(CInputStream& in, CSimplePool* pool);

private:
  // Guessed name
  static CBurstFireDescription* CreateDescription(CInputStream& in, CSimplePool* pool);
  // Guessed name
  static bool ParseProperties(CBurstFireDescription* desc, CInputStream& in, CSimplePool* pool);
};

// Guessed name
CFactoryFnReturn FBurstFireDataFactory(const SObjectTag& tag, const rstl::auto_ptr< uchar >& buffer,
                                       int size, const CVParamTransfer& transfer);

#endif // _CBURSTFIREDATAFACTORY
