#ifndef _CMETAANIMFACTORY
#define _CMETAANIMFACTORY

#include "rstl/rc_ptr.hpp"

class CInputStream;

class IMetaAnim;
class CMetaAnimFactory {
public:
  static rstl::rc_ptr< IMetaAnim > CreateMetaAnim(CInputStream& in);
};

#endif // _CMETAANIMFACTORY
