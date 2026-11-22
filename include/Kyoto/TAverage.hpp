#ifndef _TAVERAGE
#define _TAVERAGE
#include "rstl/RstlVersions.h"

#include "rstl/types.h"

#include "rstl/optional_object.hpp"
#include "rstl/vector.hpp"

template < typename T >
T GetAverageValue(const T* ptr, int count) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  T ret = *ptr;
  for (const T* cur = ptr + 1; cur < ptr + count; ++cur) {
    ret += *cur;
  }
  const float scale = 1.f / count;
  return ret * scale;
#else
  const T* end = ptr + count;
  T sum = *ptr++;
  for (; ptr < end; ++ptr) {
    sum = sum + *ptr;
  }
  return sum * (1.f / count);
#endif
}

template < typename T >
class TAverage
#if !RSTL_KYOTO_AVERAGE_MEMBER
: rstl::vector< T >
#endif
{
public:
  TAverage() {}
  explicit TAverage(int capacity) { Values().reserve(capacity); }
  TAverage(int capacity, const T& value
#if RSTL_PRIME1
                         = T()
#endif
  );

  void AddValue(const T& value);
  rstl::optional_object< T > GetAverage() const {
    if (Values().empty()) {
      return rstl::optional_object_null();
    } else {
      return GetAverageValue(Values().data(), Values().size());
    }
  }

private:
#if RSTL_KYOTO_AVERAGE_MEMBER
  rstl::vector< T > mValues;
  rstl::vector< T >& Values() { return mValues; }
  const rstl::vector< T >& Values() const { return mValues; }
#else
  rstl::vector< T >& Values() { return *this; }
  const rstl::vector< T >& Values() const { return *this; }
#endif
};

template < typename T >
inline TAverage< T >::TAverage(int capacity, const T& value) {
  Values().resize(capacity, value);
}

template < typename T >
inline void TAverage< T >::AddValue(const T& value) {
  if (Values().size() == Values().capacity()) {
    // TODO ?
    Values().mCount -= 1;
  }
  Values().insert(Values().begin(), value);
}

#endif // _TAVERAGE
