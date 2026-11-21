#ifndef _TSUBANIMTYPETOKEN
#define _TSUBANIMTYPETOKEN

#include "Kyoto/TToken.hpp"

class CAllFormatsAnimSource;

template < typename T >
class TSubAnimTypeToken {
public:
  explicit TSubAnimTypeToken(const TLockedToken< CAllFormatsAnimSource >& token);
  const T* operator->() const { return mSource; }

  const T& operator*() const { return *mSource; }

  operator const CToken&() const { return mToken; }

private:
  // Source-format selection and construction belong to CAllFormatsAnimSource.
  TLockedToken< CAllFormatsAnimSource > mToken;
  const T* mSource;
};

#endif // _TSUBANIMTYPETOKEN
