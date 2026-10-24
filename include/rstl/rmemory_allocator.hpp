#ifndef _RSTL_RMEMORY_ALLOCATOR
#define _RSTL_RMEMORY_ALLOCATOR

#include "rstl/RstlVersions.h"

#include "types.h"

#include "Kyoto/Alloc/CMemory.hpp"

namespace rstl {

#if RSTL_ALLOCATOR_EXTERN_ALLOCATE
struct rmemory_allocator {
  rmemory_allocator() {}
  rmemory_allocator(const rmemory_allocator&) {}
  static void* allocate(int size);

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
                      : static_cast< T* >(::operator new[](size, std::align_val_t(alignof(T))));
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

#if RSTL_ALLOCATOR_LEGACY_SPELLING
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
    delete[] reinterpret_cast< uchar* >(ptr);
  }
};
} // namespace rstl

#endif // _RSTL_RMEMORY_ALLOCATOR
