#ifndef _TRESERVEDAVERAGE
#define _TRESERVEDAVERAGE
#include "rstl/RstlVersions.h"
#include "rstl/RstlVersions.h"

#include "rstl/types.h"

#include "Kyoto/TAverage.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/reserved_vector.hpp"

#if !RSTL_PRIME1
template < typename T >
T GetMaxValue(const T* values, int count) {
  const T* end = values + count;
  T maximum = *values++;
  for (; values < end; ++values) {
    if (maximum < *values) {
      maximum = *values;
    }
  }
  return maximum;
}
#endif

template < typename T, int N >
class TReservedAverage : public rstl::reserved_vector< T, N > {
public:
  TReservedAverage() {}
  TReservedAverage(const T& value) { this->resize(N, value); }
#if RSTL_CORRUPTION
  void AddValue(const T& value) {
  if (this->size() < N) {
#else
  void AddValue(const T& value);
  rstl::optional_object< T > GetAverage() const;
  rstl::optional_object< T > GetMax() const;
  rstl::optional_object< T > GetEntry(int idx) const;
};

template < typename T, int N >
inline void TReservedAverage< T, N >::AddValue(const T& value) {
#if RSTL_PRIME1
  RS_ASSERT(this->size() < this->capacity(), "rstl precondition");
  RS_ASSERT(this->size() < this->capacity(), "rstl precondition");
  RS_ASSERT(this->size() < this->capacity(), "rstl precondition");
  if (this->size() < this->capacity()) {
#else
  if (this->size() < N) {
#endif
#endif
    this->push_back(value);
  }
  for (int i = this->size() - 1; i > 0; --i) {
#if RSTL_CORRUPTION
      this->operator[](i) = this->operator[](i - 1);
#else
    this->operator[](i) = this->operator[](i - 1u);
#endif
  }
  this->operator[](0) = value;
}
#if RSTL_CORRUPTION
  rstl::optional_object< T > GetAverage() const {
  if (this->empty()) {
    return rstl::optional_object_null();
  }
    return GetAverageValue(this->data(), this->size());
  }
  rstl::optional_object< T > GetMax() const {
    if (this->empty()) {
      return rstl::optional_object_null();
    }

  return GetMaxValue(this->data(), this->size());
}
  rstl::optional_object< T > GetEntry(int idx) const {
#else

template < typename T, int N >
inline rstl::optional_object< T > TReservedAverage< T, N >::GetAverage() const {
  if (this->empty()) {
    return rstl::optional_object_null();
#if RSTL_PRIME1
  } else {
    return GetAverageValue(this->data(), this->size());
  }
#else
  }
  return GetAverageValue(this->data(), this->size());
#endif
}

#if !RSTL_PRIME1
template < typename T, int N >
inline rstl::optional_object< T > TReservedAverage< T, N >::GetMax() const {
  if (this->empty()) {
    return rstl::optional_object_null();
  }
  return GetMaxValue(this->data(), this->size());
}
#endif

template < typename T, int N >
inline rstl::optional_object< T > TReservedAverage< T, N >::GetEntry(int idx) const {
#endif
  if (idx >= this->size()) {
    return rstl::optional_object_null();
  } else {
    return rstl::optional_object< T >(this->operator[](idx));
  }
}
#if RSTL_CORRUPTION
};
#endif

#endif // _TRESERVEDAVERAGE
