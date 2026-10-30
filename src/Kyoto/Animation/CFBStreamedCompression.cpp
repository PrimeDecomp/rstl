#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CFBStreamedCompression.hpp"

#if RSTL_PRIME1
#include "Kyoto/Animation/CAnimPOIData.hpp"
#endif

#include "Kyoto/Animation/CFBStreamedAnimReader.hpp"
#if RSTL_PRIME1
#include "Kyoto/IObjectStore.hpp"
#endif

#include "Kyoto/Math/CloseEnough.hpp"
#if RSTL_PRIME1
#include "Kyoto/SObjectTag.hpp"
#endif

rstl::auto_ptr< uint > CFBStreamedCompression::GetRotationsAndOffsets(uint words,
                                                                      CInputStream& in) {
  rstl::auto_ptr< uint > data(rs_new uint[words]);
  void* cursor = data.get();
  CStandardMultiFormatHeader* mainHeader = static_cast< CStandardMultiFormatHeader* >(cursor);
  new (mainHeader) CStandardMultiFormatHeader(in);
  cursor = const_cast< void* >(mainHeader->AfterEnd());
  CFBStreamedCompressionTimeHeader* timeHeader =
      static_cast< CFBStreamedCompressionTimeHeader* >(cursor);
  new (timeHeader) CFBStreamedCompressionTimeHeader(in);
  cursor = const_cast< void* >(timeHeader->AfterEnd());
#if RSTL_PRIME1
  in.Get< uint >();
#endif

  CFBStreamedPerChannelHeaderList* channels =
      static_cast< CFBStreamedPerChannelHeaderList* >(cursor);
  new (channels) CFBStreamedPerChannelHeaderList(in);
  const CFBStreamedPerChannelHeader& first = *channels->begin();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  cursor = const_cast< uchar* >(channels->AfterEnd());
#else
  uchar* next = const_cast< uchar* >(channels->AfterEnd());
  cursor = next;
#endif

  uint wordCount = static_cast< uint >(
      static_cast< float >(
          channels->GetSumOfBitCounts() * first.GetRotationBitStorage().GetWidth() + 31) /
      32.f);
  for (uint i = 0; i < wordCount; ++i) {
    TLoadedVal< uint >::Write(cursor, in.Get< uint >());
    cursor = static_cast< uchar* >(cursor) + sizeof(uint);
  }
  return data;
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CFBStreamedCompression::CFBStreamedCompression(CInputStream& in, IObjectStore& store)
#else
CFBStreamedCompression::CFBStreamedCompression(CInputStream& in, IObjectStore&)
#endif

: mScratchSize(in.Get< uint >())
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
, mEvnt(in.Get< uint >())
, mEvntToken(nullptr)
#else
, mVersionNumber(in.ReadInt8())
#endif

, mRotsAndOffs(GetRotationsAndOffsets(mScratchSize / 4 + 1, in).release())
, mRootOffset(0.f, 0.f, 0.f) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (mEvnt != 0) {
    mEvntToken = rs_new TLockedToken< CAnimPOIData >(store.GetObj(SObjectTag('EVNT', mEvnt)));
  }
#else
  {
#endif

  const CStandardMultiFormatHeader& mainHeader = MainHeader();
  const CFBStreamedCompressionTimeHeader& timeHeader = TimeHeader(mainHeader);
  const CFBStreamedPerChannelHeaderList& channels = GetPerChannelHeaderList(timeHeader);

  const CFBStreamedPerChannelHeader* firstChannel = &*channels.begin();
  const uint* bytes = GetBytes(channels);
  uint keyframes = GetNumKeyframes();
  CMemoryInputToBitLevelLoader input(bytes);
  CBitLevelLoader< CMemoryInputToBitLevelLoader > loader(input);
  uint rootIndex = 0;
  for (CFBStreamedPerChannelHeaderList::const_iterator channel(firstChannel, channels.size());
       channel != channels.end(); ++channel) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    if (channel->GetSegId() == CSegId::Root()) {
#else
      if (channel->GetSegId() == CSegId(0)) {
#endif

      break;
    }
    ++rootIndex;
  }

  CFBStreamedAnimReaderTotals totals(*this);
  totals.CalculateDown();
  CVector3f previous = totals.GetVector(rootIndex);
  float distance = 0.f;
  for (uint i = 0; i < keyframes; ++i) {
    totals.IncrementInto(loader, *this, totals);
    totals.CalculateDown();
    CVector3f current = totals.GetVector(rootIndex);
    CVector3f difference = current - previous;
    previous = current;
    float delta = difference.Magnitude();
    if (!close_enough(delta, 0.f)) {
      distance += delta;
    }
  }
  mAverageVelocity = distance / GetAnimationDuration().GetSeconds();
}
#if !RSTL_PRIME1
  CCharAnimMemoryMetrics::AddToTotalSize(mScratchSize, CCharAnimMemoryMetrics::kASS_Two);
}
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CFBStreamedCompression::~CFBStreamedCompression() {}

CCharAnimTime CFBStreamedCompression::GetAnimationDuration() const {
  return MainHeader().GetMaxTime();
#else
CFBStreamedCompression::~CFBStreamedCompression() {
  CCharAnimMemoryMetrics::SubtractFromTotalSize(mScratchSize, CCharAnimMemoryMetrics::kASS_Two);
#endif

}

