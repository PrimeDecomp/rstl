#ifndef _CSTREAMPRELOADEDTOKEN
#define _CSTREAMPRELOADEDTOKEN

#include "rstl/string.hpp"
#include "types.h"

class CStreamPreloadedData;

// The original name of this shared, reference-counted file handle is unknown.
class CStreamPreloadedToken {
public:
  CStreamPreloadedToken(const rstl::string& path);
  CStreamPreloadedToken(const CStreamPreloadedToken& other);
  ~CStreamPreloadedToken();

  void operator=(const CStreamPreloadedToken& other);
#if RSTL_VERSION == RSTL_G2MEAB
  bool IsLoaded() const;
#else
  bool IsReady() const; // Guessed name
#endif
  void Read(void* dest, int offset, int length) const;
  // Guessed name
  int GetReferenceCount() const;

private:
  CStreamPreloadedData* mData;
};
CHECK_SIZEOF(CStreamPreloadedToken, 0x4)

#endif // _CSTREAMPRELOADEDTOKEN
