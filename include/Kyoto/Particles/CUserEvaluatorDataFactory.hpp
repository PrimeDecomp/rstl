#ifndef _CUSEREVALUATORDATAFACTORY
#define _CUSEREVALUATORDATAFACTORY

#include "Kyoto/CFactoryMgr.hpp"

class CUserEvaluatorDescription;
class CInputStream;
class CSimplePool;
class CVParamTransfer;

class CUserEvaluatorDataFactory {
public:
  // Guessed name
  static CUserEvaluatorDescription* GetDescription(CInputStream& in, CSimplePool* pool);

private:
  // Guessed name
  static CUserEvaluatorDescription* CreateDescription(CInputStream& in, CSimplePool* pool);
  // Guessed name
  static bool ParseProperties(CUserEvaluatorDescription* desc, CInputStream& in, CSimplePool* pool);
};

// Guessed name
CFactoryFnReturn FUserEvaluatorDataFactory(const SObjectTag& tag,
                                           const rstl::auto_ptr< uchar >& buffer, int size,
                                           const CVParamTransfer& transfer);

#endif // _CUSEREVALUATORDATAFACTORY
