#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CSegStatementSet.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Animation/CSegIdList.hpp"
#include "Kyoto/Animation/CSegStatement.hpp"

#include "dolphin/os/OSCache.h"

namespace {
const int kSegmentCount = 100;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
enum { kSegmentSetSize = kSegmentCount * sizeof(CSegStatement) };
volatile int sFreeSegments = 31;
#else
const int kSegmentSetSize = kSegmentCount * sizeof(CSegStatement);
const int kCacheSlotCount = 3;
int sFreeSegments = (1 << kCacheSlotCount) - 1;
#endif

inline void* AllocateSegment() {
  LCQueueWait(0);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const int freeMask = sFreeSegments;
  if (freeMask) {
    for (__REGISTER uint i = 0; i < 5; ++i) {
      const uint bit = 1 << i;
      if (freeMask & bit) {
        int mask = sFreeSegments;
        mask ^= bit;
        sFreeSegments = mask;

        __REGISTER uint offset;
        __REGISTER char* base;
#ifdef __MWERKS__
        asm {
          mulli offset, i, kSegmentSetSize
          lis base, LC_BASE_PREFIX
        }
#else
        offset = i * kSegmentSetSize;
        base = static_cast< char* >(LCGetBase());
#endif
        return base + offset;
#else
  if (sFreeSegments) {
    for (uint i = 0; i < kCacheSlotCount; ++i) {
      if ((sFreeSegments & (1 << i)) != 0) {
        sFreeSegments ^= 1 << i;
        char* base = static_cast< char* >(LCGetBase());
        base += i * kSegmentSetSize;
        return base;
#endif

      }
    }
  }

  return CMemory::Alloc(kSegmentSetSize);
}

inline void FreeSegment(CSegStatement* seg) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#ifdef __MWERKS__
  __REGISTER char* base;
  asm { lis base, LC_BASE_PREFIX }
#else
  char* base = static_cast< char* >(LCGetBase());
#endif
#else
  char* base = static_cast< char* >(LCGetBase());
#endif

  char* ptr = reinterpret_cast< char* >(seg);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (ptr >= base && ptr < base + 5 * kSegmentSetSize) {
#else
  if (ptr >= base && ptr < base + kCacheSlotCount * kSegmentSetSize) {
#endif

    int index = (ptr - base) / kSegmentSetSize;
    sFreeSegments |= 1 << index;
  } else {
    CMemory::Free(seg);
  }
}
} // namespace

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CSegStatementSet::CSegStatementSet(void* ptr) : mSegData(static_cast< CSegStatement* >(ptr)) {
#else
CSegStatementSet::CSegStatementSet(void* storage)
: mSegData(static_cast< CSegStatement* >(storage)) {
#endif

  for (int i = 0; i < kSegmentCount; ++i) {
    new (&mSegData[i]) CSegStatement;
#if RSTL_PRIME1
  }
}

void CSegStatementSet::Add(const CSegIdList& list, const CCharLayoutInfo& info,
                           const CSegStatementSet& set, float weight) {
  const int count = list.size();
  for (int i = 0; i < count; ++i) {
    const CSegId& id = list[i];
    CSegStatement result;
    const CSegStatement& prior = GetData(id);
    const CSegStatement& other = set.GetData(id);
    result.Set(prior.Orientation() *
               CQuaternion::Slerp(CQuaternion::NoRotation(), other.Orientation(), weight));
    if (other.OffsetValid() && prior.OffsetValid()) {
      result.Set(prior.Offset() + (other.Offset() - info.GetFromParentUnrotated(id)) * weight);
    }
    Set(id, result);
#endif

  }
}

CStackSegStatementSet::CStackSegStatementSet() : CSegStatementSet(AllocateSegment()) {}

CStackSegStatementSet::~CStackSegStatementSet() { FreeSegment(mSegData); }

