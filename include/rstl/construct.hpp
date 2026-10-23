#ifndef _RSTL_CONSTRUCT
#define _RSTL_CONSTRUCT

#include "rstl/RstlVersions.h"
#include "rstl/iterator.hpp"
#include "types.h"

#if RSTL_VERSION == RSTL_G2MEAB
#include "Corruption/Kyoto/Alloc/CMemory.hpp"
#else
#include "Kyoto/Alloc/CMemory.hpp"
#endif

// Release rstl keeps its precondition checks as empty statements. They still count
// toward MWCC's inline size limit, which decides where uninitialized_copy is outlined.
#define RSTL_PRECONDITION(cond) ((void)0)

#if RSTL_EXTERN_CONSTRUCT_FUNCTIONS
#define RSTL_CONSTRUCT_INLINE inline
#define RSTL_CONSTRUCT_DECL
#else
#define RSTL_CONSTRUCT_INLINE static inline
#define RSTL_CONSTRUCT_DECL static
#endif

// The pointer parameter of `destroy(T*)` is `const` only where the target declares it so; it
// changes how MWCC inlines the callers.
#if RSTL_DESTROY_CONST_PARAM
#define RSTL_DESTROY_PTR(T) T* const
#else
#define RSTL_DESTROY_PTR(T) T*
#endif

#define RSTL_DECLARE_TRIVIALLY_DESTRUCTIBLE(T)                                                     \
  template <>                                                                                      \
  struct is_trivially_destructible< T > {                                                          \
    enum { value = true };                                                                         \
  };

  
#define RSTL_DECLARE_ASSIGNMENT_CONSTRUCTION(T)                                                    \
  template <>                                                                                      \
  struct use_assignment_for_construction< T > {                                                    \
    enum { value = true };                                                                         \
  };

// This describes rstl's copy policy, not trivial default construction.
#define RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(T)                                                    \
  RSTL_DECLARE_TRIVIALLY_DESTRUCTIBLE(T)                                                           \
  RSTL_DECLARE_ASSIGNMENT_CONSTRUCTION(T)

#define ECHOES_RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(T)                                             \
  END_ECHOES_NAMESPACE                                                                             \
  namespace rstl {                                                                                 \
  RSTL_DECLARE_TRIVIALLY_DESTRUCTIBLE(ECHOES_NAMESPACE T)                                          \
  RSTL_DECLARE_ASSIGNMENT_CONSTRUCTION(ECHOES_NAMESPACE T)                                         \
  }                                                                                                \
  START_ECHOES_NAMESPACE

namespace rstl {
template < typename T >
struct is_trivially_destructible {
  enum { value = false };
};

#if !RSTL_PRIME1
template < typename T >
struct range_destruction_checks { // Guessed name
  enum { value = false };
};
#endif

// Opt in only when assignment implements the required copy behavior. A trivial
// destructor alone does not establish that the type is copyable or assignable.
template < typename T >
struct use_assignment_for_construction {
  enum { value = false };
};

template < typename T, bool UseAssignment >
struct construction_policy {
  static void construct(void* dest, const T& src) {
    RSTL_PRECONDITION(dest != nullptr);
    new (dest) T(src);
  }
};

template < typename T >
struct construction_policy< T, true > {
  static void construct(void* dest, const T& src) { *static_cast< T* >(dest) = src; }
};

// Bitwise construction copies the object as doubles in an out-of-line routine (one weak
// instance per size), skipping both the copy constructor and the placement-new null check.
template < int N >
void bitwise_copy(void* dest, const void* src);

template < int N >
void bitwise_copy(void* dest, const void* src) {
  for (int i = 0; i < N; ++i) {
    static_cast< double* >(dest)[i] = static_cast< const double* >(src)[i];
  }
}

// A trailing 4-byte remainder is copied inline after the double-word block.
#define RSTL_DECLARE_BITWISE_CONSTRUCTION(T)                                                       \
  template <>                                                                                      \
  struct use_assignment_for_construction< T > {                                                    \
    enum { value = true };                                                                         \
  };                                                                                               \
  template <>                                                                                      \
  struct construction_policy< T, true > {                                                          \
    static void construct(void* dest, const T& src) {                                              \
      bitwise_copy< sizeof(T) / sizeof(double) >(dest, &src);                                      \
      if (sizeof(T) % sizeof(double) != 0) {                                                       \
        static_cast< uint* >(dest)[sizeof(T) / sizeof(uint) - 1] =                                 \
            reinterpret_cast< const uint* >(&src)[sizeof(T) / sizeof(uint) - 1];                   \
      }                                                                                            \
    }                                                                                              \
  };

#define ECHOES_RSTL_DECLARE_BITWISE_CONSTRUCTION(T)                                                \
  END_ECHOES_NAMESPACE                                                                             \
  namespace rstl {                                                                                 \
  RSTL_DECLARE_TRIVIALLY_DESTRUCTIBLE(ECHOES_NAMESPACE T)                                          \
  RSTL_DECLARE_BITWISE_CONSTRUCTION(ECHOES_NAMESPACE T)                                            \
  }                                                                                                \
  START_ECHOES_NAMESPACE

#if RSTL_TRIVIAL_POINTERS
template < typename T >
struct is_trivially_destructible< T* > {
  enum { value = true };
};

RSTL_DECLARE_TRIVIALLY_DESTRUCTIBLE(uint)
#endif

#if RSTL_TRIVIAL_SCALARS
template < typename T >
struct use_assignment_for_construction< T* > {
  enum { value = true };
};

RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(char)
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(signed char)
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(uchar)
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(bool)
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(float)
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(int)
RSTL_DECLARE_ASSIGNMENT_CONSTRUCTION(uint)
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(unsigned long)
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(short)
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(ushort)
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(unsigned long long)
#endif

template < typename T >
RSTL_CONSTRUCT_INLINE void construct(void* dest, const T& src) {
#if RSTL_CONSTRUCTION_POLICY
  construction_policy< T, use_assignment_for_construction< T >::value >::construct(dest, src);
#else
  new (dest) T(src);
#endif
}

#if RSTL_DIRECT_INT_CONSTRUCT
// Scalars skip the policy dispatch, which keeps copies of scalar containers small enough to
// inline (CWeaponMgr::Add's pair copy on Echoes).
template <>
inline void construct< int >(void* dest, const int& src) {
  *static_cast< int* >(dest) = src;
}
#endif

#if RSTL_SCALAR_TRAITS
template <>
inline void construct< uint >(void* dest, const uint& src) {
  *static_cast< uint* >(dest) = src;
}

template < typename T >
inline void construct(void* dest, T* const& src) {
  *static_cast< T** >(dest) = src;
}

template <>
inline void construct< bool >(void* dest, const bool& src) {
  *static_cast< bool* >(dest) = src;
}

template <>
inline void construct< float >(void* dest, const float& src) {
  *static_cast< float* >(dest) = src;
}
#endif

template < typename T >
struct destruction_policy { // Guessed name
  static void destroy(T* in) {
    RSTL_PRECONDITION(in != nullptr);
    if (is_trivially_destructible< T >::value) {
      return;
    }
    in->~T();
  }
};

template < typename T >
RSTL_CONSTRUCT_INLINE void destroy(RSTL_DESTROY_PTR(T) in) {
#if RSTL_CONSTRUCTION_POLICY
  destruction_policy< T >::destroy(in);
#else
#if RSTL_DESTROY_PRECONDITION
  RSTL_PRECONDITION(in != nullptr);
#endif
  in->~T();
#endif
}

// Declared before the range overloads that call it. Where the construct functions are
// external, leaving `inline` off this declaration makes out-of-line copies weak, so
// translation units share one copy.
template < typename It >
#if RSTL_PRIME1
RSTL_CONSTRUCT_INLINE void destroy_impl(It begin, It end) {
  if (is_trivially_destructible< typename iterator_traits< It >::value_type >::value) {
    return;
  }
  It cur = begin;
  for (; cur != end; ++cur) {
    destroy(&*cur);
  }
}
#else
RSTL_CONSTRUCT_DECL void destroy_impl(It begin, It end);
#endif

template < typename It >
RSTL_CONSTRUCT_INLINE void destroy(It begin, It end) {
#if RSTL_RANGE_PRECONDITIONS
  RSTL_PRECONDITION(begin <= end);
  destroy_impl(begin, end);
#else
  if (is_trivially_destructible< typename iterator_traits< It >::value_type >::value) {
    return;
  }
  It cur = begin;
#if RSTL_DESTROY_RANGE_GUARD
  if (begin != end)
#endif
    for (; cur != end; ++cur) {
      destroy(&*cur);
    }
#endif
}

#if RSTL_POINTER_RANGE_OVERLOADS
// Unlike iterator ranges, pointer ranges skip the precondition check and the guard.
template < typename T >
RSTL_CONSTRUCT_INLINE void destroy(T* begin, T* end) {
  destroy_impl(begin, end);
}
#endif

#if !RSTL_PRIME1
template < typename It >
RSTL_CONSTRUCT_INLINE void destroy_impl(It begin, It end) {
  if (range_destruction_checks< typename iterator_traits< It >::value_type >::value) {
    // Retain the two release statements; the original checks are unknown.
    ((void)0);
    ((void)0);
  }
  if (is_trivially_destructible< typename iterator_traits< It >::value_type >::value) {
    return;
  }
  It cur = begin;
  for (; cur != end; ++cur) {
    destroy(&*cur);
  }
}
#endif

template < typename It, typename T >
static inline T uninitialized_copy(It begin, It end, T out) {
#if RSTL_RANGE_PRECONDITIONS
  RSTL_PRECONDITION(begin <= end);
#endif
  RSTL_PRECONDITION(begin == end || out != nullptr);
  T tmp = out;
  It cur = begin;
#if RSTL_PREINCREMENT_COPY_LOOPS
  for (; cur != end; ++cur, ++tmp) {
#else
  for (; cur != end; tmp += 1, ++cur) {
#endif
    construct(tmp, *cur);
  }

  return tmp;
}

#if RSTL_RANGE_PRECONDITIONS
template < typename S, typename T >
static inline T uninitialized_copy(S* begin, S* end, T out) {
  T tmp = out;
  S* cur = begin;
  for (; cur != end; ++cur, ++tmp) {
    construct(tmp, *cur);
  }

  return tmp;
}
#endif

template < typename S, typename D >
static inline D uninitialized_copy_n(S src, int n, D dest) {
#if RSTL_COPY_N_COUNTER_FIRST
  int remaining = n;
#endif
  S it = src;
  D cur = dest;
#if RSTL_COPY_N_COUNTER_FIRST
  for (; remaining != 0; --remaining, ++it, ++cur) {
#elif RSTL_COUNTED_COPY_N_LOOP
  for (int remaining = n; remaining != 0; --remaining, ++it, ++cur) {
#elif RSTL_PREINCREMENT_COPY_LOOPS
  for (int i = 0; i != n; ++it, ++i, ++cur) {
#else
  for (int i = 0; i < n; ++cur, ++i, ++it) {
#endif
    construct(&*cur, *it);
  }

  return cur;
}

#if RSTL_POINTER_FILL_OVERLOAD
template < typename T >
static inline void uninitialized_fill_n(T** dest, int n, T* const& value) {
  for (int i = 0; i < n; ++i, ++dest) {
    *dest = value;
  }
}
#endif

template < typename D, typename S >
#if RSTL_COUNTED_FILL_LOOPS
static inline void uninitialized_fill_n(D cur, int n, const S& value) {
#else
static inline void uninitialized_fill_n(D dest, int n, const S& value) {
  D cur = dest;
#endif
  for (int i = 0; i < n; ++i, ++cur) {
#if RSTL_FILL_USES_CONSTRUCT
    construct(&*cur, value);
#else
    void* ptr = &*cur;
    new (ptr) S(value);
#endif
  }
}
} // namespace rstl

#endif // _RSTL_CONSTRUCT
