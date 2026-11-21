#ifndef _TAVERAGE
#define _TAVERAGE
#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/RstlVersions.h"
#endif

#include "rstl/types.h"

#include "rstl/optional_object.hpp"
#include "rstl/vector.hpp"

template < typename T >
T GetAverageValue(const T* ptr, int count) {
#if RSTL_CORRUPTION
  const T* end = ptr + count;
  T sum = *ptr++;
  for (; ptr < end; ++ptr) {
    sum = sum + *ptr;
  }
  return sum * (1.f / count);
#else
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
#endif
}

template < typename T >
#if RSTL_CORRUPTION
class TAverage : rstl::vector< T > {
#else
class TAverage
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
: rstl::vector< T >
#endif
{
#endif
public:
  TAverage() {}
#if RSTL_CORRUPTION
  explicit TAverage(int capacity) { this->reserve(capacity); }
  TAverage(int capacity, const T& value);
#else
#if !RSTL_PRIME1
  explicit TAverage(int capacity) { mValues.reserve(capacity); }
#endif
  TAverage(int capacity, const T& value
#if RSTL_PRIME1
                         = T()
#endif
  );
#endif

  void AddValue(const T& value);
  rstl::optional_object< T > GetAverage() const {
#if RSTL_CORRUPTION
    if (this->empty()) {
#else
    if (
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
        this->empty()
#else
        mValues.empty()
#endif
    ) {
#endif
      return rstl::optional_object_null();
    } else {
#if RSTL_CORRUPTION
      return GetAverageValue(this->data(), this->size());
#else
      return GetAverageValue(
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
          this->data(), this->size()
#else
          mValues.data(), mValues.size()
#endif
      );
#endif
    }
  }
#if !RSTL_CORRUPTION
#if !RSTL_PRIME1
private:
  rstl::vector< T > mValues;
#endif
#endif
};

template < typename T >
#if RSTL_CORRUPTION
TAverage< T >::TAverage(int capacity, const T& value) {
  this->resize(capacity, value);
#else
inline TAverage< T >::TAverage(int capacity, const T& value) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  this->resize(capacity, value);
#else
  mValues.resize(capacity, value);
#endif
#endif
}

template < typename T >
#if !RSTL_CORRUPTION
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
inline
#endif
#endif
void TAverage< T >::AddValue(const T& value) {
#if RSTL_CORRUPTION
  if (this->size() == this->capacity()) {
#else
  if (
#endif
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_CORRUPTION
    this->mCount -= 1;
  }
  this->insert(this->begin(), value);
#else
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
#endif
  }
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_CORRUPTION
#if RSTL_PRIME1
  this->insert(this->begin(), value);
#else
  mValues.insert(mValues.begin(), value);
#endif
}
#endif

#endif // _TAVERAGE
