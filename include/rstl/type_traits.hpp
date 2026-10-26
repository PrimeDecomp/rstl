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
struct type_traits< unsigned int > {
  typedef const unsigned int& vector_fill_type;
  typedef unsigned int vector_resize_type;
  typedef const unsigned int& reserved_vector_fill_type;
};

template <>
struct type_traits< bool > {
  typedef bool vector_fill_type;
  typedef const bool& vector_resize_type;
  typedef bool reserved_vector_fill_type;
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

template < typename T >
struct pointer_traits {
  typedef T* get_type;

  typedef T& dereference_type;
#if RSTL_SINGLE_PTR_PLAIN_ACCESS
  typedef const T& dereference_type_const;
#else
  typedef T& dereference_type_const;
#endif

#if RSTL_SINGLE_PTR_PLAIN_ACCESS
  typedef T* arrow_type;
#else
  typedef T* const arrow_type;
#endif
};

template <>
struct pointer_traits< void > {
  typedef void* get_type;

  typedef void* dereference_type;
#if RSTL_SINGLE_PTR_PLAIN_ACCESS
  typedef const void* dereference_type_const;
#else
  typedef void* dereference_type_const;
#endif

#if RSTL_SINGLE_PTR_PLAIN_ACCESS
  typedef void* arrow_type;
#else
  typedef void* const arrow_type;
#endif
};

} // namespace rstl

#endif // _RSTL_TYPE_TRAITS
