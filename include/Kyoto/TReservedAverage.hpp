#ifndef _TRESERVEDAVERAGE
#define _TRESERVEDAVERAGE
#include "rstl/RstlVersions.h"

#include "types.h"

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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  void AddValue(const T& value);
  rstl::optional_object< T > GetAverage() const;
  rstl::optional_object< T > GetEntry(int idx) const;
};

template < typename T, int N >
inline
void TReservedAverage< T, N >::AddValue(const T& value) {
  RS_ASSERT(this->size() < this->capacity(), "rstl precondition");
  RS_ASSERT(this->size() < this->capacity(), "rstl precondition");
  RS_ASSERT(this->size() < this->capacity(), "rstl precondition");
  if (this->size() < this->capacity()) {
#else
  void AddValue(const T& value) {
    if (this->size() < N) {
#endif

    this->push_back(value);
  }
  for (int i = this->size() - 1; i > 0; --i) {
    this->operator[](i) = this->operator[](i - 1u);
  }
  this->operator[](0) = value;
}
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
template < typename T, int N >
inline rstl::optional_object< T > TReservedAverage< T, N >::GetAverage() const {
#else
  rstl::optional_object< T > GetAverage() const {
#endif

  if (this->empty()) {
    return rstl::optional_object_null();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  } else {
#else
    }
#endif

    return GetAverageValue(this->data(), this->size());
  }
#if !RSTL_PRIME1
  rstl::optional_object< T > GetMax() const {
    if (this->empty()) {
      return rstl::optional_object_null();
#endif

}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
template < typename T, int N >
inline rstl::optional_object< T > TReservedAverage< T, N >::GetEntry(int idx) const {
#else
    return GetMaxValue(this->data(), this->size());
  }
  rstl::optional_object< T > GetEntry(int idx) const {
#endif

  if (idx >= this->size()) {
    return rstl::optional_object_null();
  } else {
    return rstl::optional_object< T >(this->operator[](idx));
  }
}
#if !RSTL_PRIME1
};
#endif

#endif // _TRESERVEDAVERAGE
