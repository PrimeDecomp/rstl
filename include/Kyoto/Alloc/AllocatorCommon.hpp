#ifndef _ALLOCATORCOMMON
#define _ALLOCATORCOMMON

#include <stddef.h> // For size_t
#include <stdint.h>

static const int kAllocatorPointerSize = sizeof(void*);
static const int kAllocatorPointerBits = kAllocatorPointerSize * 8;

template < size_t PointerSize >
struct PatternExpander;

template <>
struct PatternExpander< 4 > {
  typedef unsigned int Type;
  typedef size_t MaskType;
  static const Type Multiplier = 0x01010101U;
  static const MaskType TopNybbleMask = 0xF0000000U;
};

template <>
struct PatternExpander< 8 > {
  typedef unsigned long long Type;
  typedef size_t MaskType;
  static const Type Multiplier = 0x0101010101010101ULL;
  static const MaskType TopNybbleMask = 0xF000000000000000ULL;
};

#define EXPAND_PATTERN(byte_val)                                                                   \
  (static_cast< PatternExpander< kAllocatorPointerSize >::Type >(byte_val) *                       \
   PatternExpander< kAllocatorPointerSize >::Multiplier)

// The post guard (0xEF) follows a block's data: it is the first word of the next block's header,
// and the prototype also fills the slack after the data with it. The prior guard (0xEA) is the last
// word of a header, just before the data. The prototype's assert strings and its 0xEF fill check
// name them this way.
static const intptr_t kAllocatorPostGuard = EXPAND_PATTERN(0xEF);
static const intptr_t kAllocatorPriorGuard = EXPAND_PATTERN(0xEA);
static const intptr_t kAllocatorPointerTopNybbleMask =
    PatternExpander< sizeof(void*) >::TopNybbleMask;

#endif // _ALLOCATORCOMMON
