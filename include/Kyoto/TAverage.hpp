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
class TAverage : rstl::vector< T > {
public:
  TAverage() {}
  explicit TAverage(int capacity) { this->reserve(capacity); }
  TAverage(int capacity, const T& value
#if RSTL_PRIME1
                         = T()
#endif
  );

  void AddValue(const T& value);
  rstl::optional_object< T > GetAverage() const {
    if (this->empty()) {
      return rstl::optional_object_null();
    } else {
      return GetAverageValue(this->data(), this->size());
    }
  }
};

template < typename T >
inline TAverage< T >::TAverage(int capacity, const T& value) {
  this->resize(capacity, value);
}

template < typename T >
inline void TAverage< T >::AddValue(const T& value) {
  if (this->size() == this->capacity()) {
    // TODO ?
    this->mCount -= 1;
  }
  this->insert(this->begin(), value);
}

#endif // _TAVERAGE
