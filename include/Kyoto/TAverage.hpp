#ifndef _TAVERAGE
#define _TAVERAGE
#include "rstl/RstlVersions.h"

#include "types.h"

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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
: rstl::vector< T >
#endif
{
public:
  TAverage() {}
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  TAverage(int capacity, const T& value = T());
#else
  explicit TAverage(int capacity) { mValues.reserve(capacity); }
  TAverage(int capacity, const T& value);
#endif

  void AddValue(const T& value);
  rstl::optional_object< T > GetAverage() const {
    if (
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
        this->empty()
#else
        mValues.empty()
#endif
    ) {
      return rstl::optional_object_null();
    } else {
      return GetAverageValue(
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
          this->data(), this->size()
#else
          mValues.data(), mValues.size()
#endif
      );
    }
  }
#if !RSTL_PRIME1
private:
  rstl::vector< T > mValues;
#endif
};

template < typename T >
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
inline
#endif
TAverage< T >::TAverage(int capacity, const T& value) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  this->resize(capacity, value);
#else
  mValues.resize(capacity, value);
#endif
}

template < typename T >
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
inline
#endif
void TAverage< T >::AddValue(const T& value) {
  if (
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      this->size() == this->capacity()
#else
      mValues.size() == mValues.capacity()
#endif
  ) {
    // TODO ?
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    this->mCount -= 1;
#else
    mValues.mCount -= 1;
#endif
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  this->insert(this->begin(), value);
#else
  mValues.insert(mValues.begin(), value);
#endif
}

#endif // _TAVERAGE
