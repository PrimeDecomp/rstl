#include "Kyoto/Animation/CSegIdList.hpp"

#include "Kyoto/Animation/CCharAnimMemoryMetrics.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

CSegIdList::CSegIdList(CInputStream& in)
: mList(in) {
  CCharAnimMemoryMetrics::AddToTotalSize(mList.capacity(), CCharAnimMemoryMetrics::kASS_Two);
}

