#ifndef _TANIMSOURCEINFO
#define _TANIMSOURCEINFO

#include "Kyoto/Animation/IAnimSourceInfo.hpp"
#include "Kyoto/Animation/TSubAnimTypeToken.hpp"



template < typename T >
class TAnimSourceInfo : public IAnimSourceInfo {
public:
  explicit TAnimSourceInfo(const TSubAnimTypeToken< T >& source) : mSource(source) {}

  // IAnimSourceInfo
  CCharAnimTime GetAnimationDuration() const override { return mSource->GetAnimationDuration(); }
  bool HasScaleData() const override { return mSource->HasScaleData(); }
  ~TAnimSourceInfo() override {}

private:
  TSubAnimTypeToken< T > mSource;
};


#endif // _TANIMSOURCEINFO
