#ifndef _CVPARAMTRANSFER
#define _CVPARAMTRANSFER
#include "rstl/RstlVersions.h"

#include "Kyoto/IObj.hpp"
#include "rstl/rc_ptr.hpp"

class IVParamObj : public IObj {};

template < typename T >
class TObjOwnerParam : public IVParamObj {
public:
  ~TObjOwnerParam() {}
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  TObjOwnerParam(const T& data) : mData(data) {}
  const T& GetData() const { return mData; }
  const T& operator*() const { return GetData(); }
#else
  TObjOwnerParam(T data) : mData(data) {}
  T GetData() const { return static_cast<T>(mData); }
#endif

private:
  T mData;
};

class CVParamTransfer {
public:
  CVParamTransfer() {}
  template < typename T >
  CVParamTransfer(TObjOwnerParam< T >* obj) : mObj(obj) {}
  static CVParamTransfer Null() { return CVParamTransfer(); }

  const IVParamObj& operator*() const { return *mObj; }
#if !RSTL_PRIME1
  const rstl::rc_ptr< IVParamObj >& GetObj() const { return mObj; }

private:
#endif

  rstl::rc_ptr< IVParamObj > mObj;
};

#endif // _CVPARAMTRANSFER
