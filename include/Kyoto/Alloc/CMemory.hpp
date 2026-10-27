#ifndef _CMEMORY
#define _CMEMORY

#include "Kyoto/Alloc/CCallStack.hpp"
#include "Kyoto/Alloc/IAllocator.hpp"
#include "types.h"

#include "rstl/RstlVersions.h"

extern "C" const char kUnknownType[];

class COsContext;
class CMemory {
  static IAllocator* mpAllocator;
  static bool mInitialized;

public:
  static void Startup(COsContext& ctx);
  static void Shutdown();
  static void SetAllocator(COsContext& ctx, IAllocator& allocator);
  static IAllocator* GetAllocator() { return mpAllocator; }
  static void* Alloc(size_t len, IAllocator::EHint hint = IAllocator::kHI_None,
                     IAllocator::EScope scope = IAllocator::kSC_Unk1,
                     IAllocator::EType type = IAllocator::kTP_Heap,
                     const CCallStack& callstack = CCallStack(-1, "\?\?(\?\?)"));
  static void Free(const void* ptr);
  static void SetOutOfMemoryCallback(IAllocator::FOutOfMemoryCb callback, const void* context);
  static IAllocator::SMetrics GetMetrics(bool unk1, bool unk2);
  static void OffsetFakeStatics(int);
  // Guessed name and result.
  static int EnumAllocations(IAllocator::FEnumAllocationsCb callback, const void* context, bool b);
};

#if defined(__MWERKS__) || defined(CLANGD)
void* operator new(size_t sz, const char*, const char*);
void* operator new[](size_t sz, const char*, const char*);
#if RSTL_KYOTO_INLINE_GLOBAL_NEW
// TODO remove

inline void* operator new(size_t sz) { return operator new(sz, "??(??)", nullptr); }
inline void* operator new[](size_t sz) { return operator new[](sz, "??(??)", nullptr); }
#endif

// placement new
inline void* operator new(size_t n, void* ptr) { return ptr; };
#else

#include <new>

#endif

#ifdef __MWERKS__
#ifdef WEAK_DELETE_HACK
__declspec(weak)
#else
inline
#endif
void operator delete(void* ptr) {
  CMemory::Free(ptr);
}
#ifdef WEAK_ARRAY_DELETE_HACK
__declspec(weak)
#else
inline
#endif
void operator delete[](void* ptr) {
  CMemory::Free(ptr);
}
#define NEW new ("??(??)", nullptr)
// Allocations without a source position ("??(??)", escaped so it is not read as trigraphs).
#define rs_new new ("\?\?(\?\?)", nullptr)
// Allocations that record their source position, as Prime's and Echoes' rs_new does with
// __FILE__ and __LINE__. The line is explicit here so the original line numbers survive in the
// strings.
#define RS_NEW_STRINGIZE_IMPL(x) #x
#define RS_NEW_STRINGIZE(x) RS_NEW_STRINGIZE_IMPL(x)
#define rs_new_line(line) rs_new_line_in(__FILE__, line)
// Same, for sources whose original file name differs from ours (Main.cpp, DolphinCFont.cpp).
#define rs_new_line_in(file, line) new (file "(" RS_NEW_STRINGIZE(line) ") : ", nullptr)
// Raw block rounded up to the allocator's alignment, for buffers that are read into directly.
// Records the source position the way rs_new_line_in does.
#define rs_aligned_new_line_in(file, line, size)                                                   \
  CMemory::Alloc(size, IAllocator::kHI_RoundUpLen, IAllocator::kSC_Unk1, IAllocator::kTP_Heap,     \
                 CCallStack(-1, file "(" RS_NEW_STRINGIZE(line) ") : ", kUnknownType))
#define rs_aligned_new_line(size, line) rs_aligned_new_line_in(__FILE__, line, size)
#else
#if RSTL_KYOTO_INLINE_GLOBAL_NEW
__attribute__((weak)) void operator delete(void* ptr) { CMemory::Free(ptr); }
__attribute__((weak)) void operator delete[](void* ptr) { CMemory::Free(ptr); }
#endif
#define NEW new
#define rs_new new
#define rs_new_line(line) new
#define rs_new_line_in(file, line) new
#define rs_aligned_new_line_in(file, line, size) CMemory::Alloc(size, IAllocator::kHI_RoundUpLen)
#define rs_aligned_new_line(size, line) CMemory::Alloc(size, IAllocator::kHI_RoundUpLen)
#endif

#endif // _CMEMORY
