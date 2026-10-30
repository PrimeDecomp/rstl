#ifndef _CFACTORYFNRETURN
#define _CFACTORYFNRETURN
#include "rstl/RstlVersions.h"
#include "Kyoto/TToken.hpp"

class CFactoryFnReturn {
public:
#if RSTL_PRIME1
  CFactoryFnReturn() {}
#endif

  template < typename T >
  CFactoryFnReturn(T* ptr) : obj(TToken< T >::GetIObjObjectFor(ptr).release()) {}

  const rstl::auto_ptr< IObj >& GetObjForTransfer() const { return obj; }

private:
  rstl::auto_ptr< IObj > obj;
};

#endif // _CFACTORYFNRETURN
