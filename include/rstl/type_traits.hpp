#ifndef _RSTL_TYPE_TRAITS
#define _RSTL_TYPE_TRAITS

#include "rstl/RstlVersions.h"

namespace rstl {

// Guessed name. Ordinary append has a separate float argument policy.
template < typename T >
struct push_argument {
  typedef const T& type;
};

#if RSTL_SCALAR_FILL_BY_VALUE
template <>
struct push_argument< float > {
  typedef float type;
};
#endif

#if RSTL_SCALAR_FILL_BY_VALUE
// Guessed name. Builtin values and pointers are passed by value; other types by const reference.
template < typename T >
struct fill_argument {
  typedef const T& type;
};

template < typename T >
struct fill_argument< T* > {
  typedef T* type;
};

#define RSTL_FILL_ARGUMENT_BY_VALUE(T)                                                               template <>                                                                                        struct fill_argument< T > {                                                                          typedef T type;                                                                                  };
RSTL_FILL_ARGUMENT_BY_VALUE(bool)
RSTL_FILL_ARGUMENT_BY_VALUE(char)
RSTL_FILL_ARGUMENT_BY_VALUE(signed char)
RSTL_FILL_ARGUMENT_BY_VALUE(unsigned char)
RSTL_FILL_ARGUMENT_BY_VALUE(short)
RSTL_FILL_ARGUMENT_BY_VALUE(unsigned short)
RSTL_FILL_ARGUMENT_BY_VALUE(int)
RSTL_FILL_ARGUMENT_BY_VALUE(unsigned int)
RSTL_FILL_ARGUMENT_BY_VALUE(long)
RSTL_FILL_ARGUMENT_BY_VALUE(unsigned long)
RSTL_FILL_ARGUMENT_BY_VALUE(long long)
RSTL_FILL_ARGUMENT_BY_VALUE(unsigned long long)
RSTL_FILL_ARGUMENT_BY_VALUE(float)
RSTL_FILL_ARGUMENT_BY_VALUE(double)
#undef RSTL_FILL_ARGUMENT_BY_VALUE

// Guessed name. Keep argument passing independent of copy and destruction policies.
template < typename T >
struct type_traits {
  typedef typename push_argument< T >::type vector_push_type;
  typedef typename fill_argument< T >::type vector_fill_type;
  typedef typename fill_argument< T >::type vector_resize_type;
#if RSTL_VECTOR_INSERT_SCALAR_BY_VALUE
  typedef typename fill_argument< T >::type vector_insert_type;
#else
  typedef const T& vector_insert_type;
#endif
  typedef typename fill_argument< T >::type reserved_vector_fill_type;
};
#else
// Guessed name. Keep argument passing independent of copy and destruction policies.
template < typename T >
struct type_traits {
  typedef typename push_argument< T >::type vector_push_type;
  typedef const T& vector_fill_type;
  typedef const T& vector_resize_type;
  typedef const T& vector_insert_type;
  typedef const T& reserved_vector_fill_type;
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
