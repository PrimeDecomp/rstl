#ifndef _CMETATRANSFACTORY
#define _CMETATRANSFACTORY

#include "rstl/rc_ptr.hpp"

class CInputStream;

class IMetaTrans;
class CMetaTransFactory {
public:
  static rstl::rc_ptr< IMetaTrans > CreateMetaTrans(CInputStream& in);
};

#endif // _CMETATRANSFACTORY
