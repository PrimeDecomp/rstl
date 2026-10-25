#ifndef _RSTL_TYPE_TRAITS
#define _RSTL_TYPE_TRAITS

#include "rstl/RstlVersions.h"

namespace rstl {

// Guessed name. Keep argument passing independent of copy and destruction policies.
template < typename T >
struct type_traits {
  typedef const T& vector_fill_type;
  typedef const T& vector_resize_type;
  typedef const T& reserved_vector_fill_type;
};

#if RSTL_VERSION == RSTL_G2MEAB
template <>
struct type_traits< bool > {
  typedef bool vector_fill_type;
  typedef const bool& vector_resize_type;
  typedef const bool& reserved_vector_fill_type;
};

template <>
struct type_traits< signed char > {
  typedef const signed char& vector_fill_type;
  typedef signed char vector_resize_type;
  typedef const signed char& reserved_vector_fill_type;
};

template <>
struct type_traits< unsigned char > {
  typedef const unsigned char& vector_fill_type;
  typedef const unsigned char& vector_resize_type;
  typedef unsigned char reserved_vector_fill_type;
};

template <>
struct type_traits< unsigned long > {
  typedef const unsigned long& vector_fill_type;
  typedef const unsigned long& vector_resize_type;
  typedef unsigned long reserved_vector_fill_type;
};

#endif

} // namespace rstl

#endif // _RSTL_TYPE_TRAITS
