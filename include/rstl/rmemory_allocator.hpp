#ifndef _RSTL_RMEMORY_ALLOCATOR
#define _RSTL_RMEMORY_ALLOCATOR

#include "rstl/RstlVersions.h"

#include "types.h"

#include "Kyoto/Alloc/CMemory.hpp"

#if !defined(__MWERKS__) && !defined(CLANGD)
#include <new>
#endif
#if defined(TARGET_PC)
#include <type_traits>
#endif

namespace rstl {

#if RSTL_ALLOCATOR_EXTERN_ALLOCATE
struct rmemory_allocator {
  rmemory_allocator() {}
  rmemory_allocator(const rmemory_allocator&) {}
#if RSTL_ALLOCATOR_GLOBAL_ALLOCATE
  static void* global_allocate(int size);
  static void* allocate(int size) { return global_allocate(size); }
#else
  static void* allocate(int size);
#endif

#if RSTL_ALLOCATOR_LEGACY_SPELLING
  template < typename T >
  static void allocate(T*& output, int size) {
    output = reinterpret_cast< T* >(allocate(size * sizeof(T)));
  }
  template < typename T >
  static T* allocate2(int count) {
    int size = count * sizeof(T);
    return size == 0 ? 0 : reinterpret_cast< T* >(new uchar[size]);
  }
#else
  template < typename T >
  static void allocate(T*& out, int count) {
    int size = count * sizeof(T);
    out = reinterpret_cast< T* >(allocate(size));
  }
  // TODO: this fixes a regswap in vector::reserve
  template < typename T >
  static T* allocate2(int count) {
    int size = count * sizeof(T);
    if (size == 0) {
      return nullptr;
    } else {
      return reinterpret_cast< T* >(new uchar[size]);
    }
  }
#endif
  template < typename T >
  static void deallocate(T* ptr) {
    delete[] reinterpret_cast< uchar* >(ptr);
  }
};
#else
struct rmemory_allocator {
  rmemory_allocator() {}
  rmemory_allocator(const rmemory_allocator&) {}

#if defined(__MWERKS__) && RSTL_OUTLINED_ALLOCATE
  static void* allocate(int size);
#endif

  template < typename T >
  static void allocate(T*& out, int count) {
#if defined(__MWERKS__) || defined(CLANGD)
    int size = count * sizeof(T);
#else
    if (count < 0 || static_cast< size_t >(count) > static_cast< size_t >(-1) / sizeof(T)) {
      throw std::bad_alloc();
    }
    const size_t size = static_cast< size_t >(count) * sizeof(T);
#if defined(__cpp_aligned_new)
    if (alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
      out = size == 0 ? nullptr
                      : static_cast< T* >(
                            ::operator new[](size, static_cast< std::align_val_t >(alignof(T))));
      return;
    }
#endif
#endif
#if defined(__MWERKS__) && RSTL_OUTLINED_ALLOCATE
    out = reinterpret_cast< T* >(allocate(size));
#else
    if (size == 0) {
      out = nullptr;
    } else {
      out = reinterpret_cast< T* >(rs_new uchar[size]);
    }
#endif
  }

  template < typename T >
  static void deallocate(T* ptr) {
#if !defined(__MWERKS__) && defined(__cpp_aligned_new)
    if (alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
      ::operator delete[](ptr, std::align_val_t(alignof(T)));
      return;
    }
#endif
    if (ptr != nullptr) {
      delete[] reinterpret_cast< uchar* >(ptr);
    }
  }
};
#endif

struct aligned_allocator {
  aligned_allocator() {}
  aligned_allocator(const aligned_allocator&) {}

#if !RSTL_ALIGNED_ALLOCATE_INLINE
  static void* allocate(int size) {
    return size == 0 ? nullptr : CMemory::Alloc(size, IAllocator::kHI_RoundUpLen);
  }
#endif

#if !defined(__MWERKS__) && !defined(CLANGD)
  template < typename T >
  static void allocate(T*& out, int count) {
    if (count < 0 || static_cast< size_t >(count) > (static_cast< size_t >(-1) - 31) / sizeof(T)) {
      throw std::bad_array_new_length();
    }
    const size_t size = (static_cast< size_t >(count) * sizeof(T) + 31) & ~size_t{31};
    constexpr size_t alignment = alignof(T) > 32 ? alignof(T) : 32;
    out =
        size == 0
            ? nullptr
            : static_cast< T* >(::operator new[](size, static_cast< std::align_val_t >(alignment)));
  }
#elif RSTL_ALLOCATOR_LEGACY_SPELLING
  template < typename T >
  static void allocate(T*& output, int count) {
    const int size = count * sizeof(T);
    output = size == 0 ? 0 : static_cast< T* >(CMemory::Alloc(size, IAllocator::kHI_RoundUpLen));
  }
#else
  template < typename T >
  static void allocate(T*& out, int count) {
#if RSTL_ALIGNED_ALLOCATE_INLINE
    const int size = count * sizeof(T);
    out = size == 0 ? nullptr
                    : static_cast< T* >(CMemory::Alloc(size, IAllocator::kHI_RoundUpLen));
#else
    out = static_cast< T* >(allocate(count * sizeof(T)));
#endif
  }
#endif

  template < typename T >
  static void deallocate(T* ptr) {
#if defined(__MWERKS__) || defined(CLANGD)
    delete[] reinterpret_cast< uchar* >(ptr);
#else
    constexpr size_t alignment = alignof(T) > 32 ? alignof(T) : 32;
    ::operator delete[](ptr, static_cast< std::align_val_t >(alignment));
#endif
  }
};

#if defined(TARGET_PC)
template < typename T >
struct pointer_deleter {
  static void destroy(T* ptr) {
    using Value = std::remove_cv_t< T >;
    // Metaforce allocates buffers of these scalar types with operator new[].
    if constexpr (std::is_same_v< Value, char > || std::is_same_v< Value, unsigned char > ||
                  std::is_same_v< Value, unsigned short > ||
                  std::is_same_v< Value, unsigned int > || std::is_same_v< Value, float >) {
      ::operator delete[](const_cast< void* >(static_cast< const volatile void* >(ptr)));
    } else {
      delete ptr;
    }
  }
};
#endif
} // namespace rstl

#endif // _RSTL_RMEMORY_ALLOCATOR
