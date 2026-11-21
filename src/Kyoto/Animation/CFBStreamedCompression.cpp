#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/RstlVersions.h"
#endif
#include "Kyoto/Animation/CFBStreamedCompression.hpp"

#if RSTL_CORRUPTION
#include "Kyoto/Alloc/Assert.hpp"
#else
#include "Kyoto/Animation/CAnimPOIData.hpp"

#endif
#include "Kyoto/Animation/CFBStreamedAnimReader.hpp"
#if !RSTL_CORRUPTION
#include "Kyoto/IObjectStore.hpp"

#endif
#include "Kyoto/Math/CloseEnough.hpp"
#if !RSTL_CORRUPTION
#include "Kyoto/SObjectTag.hpp"
#endif

rstl::auto_ptr< uint > CFBStreamedCompression::GetRotationsAndOffsets(uint words,
                                                                      CInputStream& in) {
  rstl::auto_ptr< uint > data(rs_new_line(14) uint[words]);
  void* cursor = data.get();
  CStandardMultiFormatHeader* mainHeader = static_cast< CStandardMultiFormatHeader* >(cursor);
  new (mainHeader) CStandardMultiFormatHeader(in);
  cursor = const_cast< void* >(mainHeader->AfterEnd());
  CFBStreamedCompressionTimeHeader* timeHeader =
      static_cast< CFBStreamedCompressionTimeHeader* >(cursor);
  new (timeHeader) CFBStreamedCompressionTimeHeader(in);
  cursor = const_cast< void* >(timeHeader->AfterEnd());
#if !RSTL_CORRUPTION
#if RSTL_PRIME1
  in.Get< uint >();
#endif
#endif
  CFBStreamedPerChannelHeaderList* channels =
      static_cast< CFBStreamedPerChannelHeaderList* >(cursor);
  new (channels) CFBStreamedPerChannelHeaderList(in);
  const CFBStreamedPerChannelHeader& first = *channels->begin();
#if RSTL_CORRUPTION
  uchar* next = const_cast< uchar* >(channels->AfterEnd());
  cursor = next;
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  cursor = const_cast< uchar* >(channels->AfterEnd());
#else
  uchar* next = const_cast< uchar* >(channels->AfterEnd());
  cursor = next;
#endif
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

#if RSTL_CORRUPTION
CFBStreamedCompression::CFBStreamedCompression(CInputStream& in, IObjectStore&)
#else
CFBStreamedCompression::CFBStreamedCompression(CInputStream& in, IObjectStore& store)
#endif
: mScratchSize(in.Get< uint >())
#if RSTL_CORRUPTION
, mVersionNumber(in.ReadInt8())
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
, mEvnt(in.Get< uint >())
, mEvntToken(nullptr)
#else
, mVersionNumber(in.ReadInt8())
#endif
#endif
, mRotsAndOffs(GetRotationsAndOffsets(mScratchSize / 4 + 1, in).release())
, mRootOffset(0.f, 0.f, 0.f) {
#if RSTL_CORRUPTION
  RS_VERIFY_THROW(55, mVersionNumber <= kCurrentVersionNumber, kException_InvalidVersion,
                  "Invalid compressed animation version!  Re-cook animation with current tools.");
  {
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (mEvnt != 0) {
    mEvntToken = rs_new TLockedToken< CAnimPOIData >(store.GetObj(SObjectTag('EVNT', mEvnt)));
  }
#else
  {
#endif
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
#if RSTL_CORRUPTION
      if (channel->GetSegId() == CSegId(0)) {
#else
    if (channel->GetSegId() ==
#if RSTL_PRIME1
        CSegId::Root()
#else
        CSegId(0)
#endif
    ) {
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
#if RSTL_CORRUPTION
  CCharAnimMemoryMetrics::AddToTotalSize(mScratchSize, CCharAnimMemoryMetrics::kASS_Two);
}
#else
#if !RSTL_PRIME1
  CCharAnimMemoryMetrics::AddToTotalSize(mScratchSize, CCharAnimMemoryMetrics::kASS_Two);
}
#endif
#endif

CFBStreamedCompression::~CFBStreamedCompression() {
#if RSTL_CORRUPTION
  CCharAnimMemoryMetrics::SubtractFromTotalSize(mScratchSize, CCharAnimMemoryMetrics::kASS_Two);
#else
#if !RSTL_PRIME1
  CCharAnimMemoryMetrics::SubtractFromTotalSize(mScratchSize, CCharAnimMemoryMetrics::kASS_Two);
#endif
#endif
}

#if RSTL_CORRUPTION
CCharAnimTime CFBStreamedCompression::GetAnimationDuration() const {
  return MainHeader().GetMaxTime();
}

const uchar CStandardMultiFormatHeader::kCurrentVersionNumber = 1;
#else
#if !RSTL_CFB_ANIMATION_DURATION_IN_READER
CCharAnimTime CFBStreamedCompression::GetAnimationDuration() const {
  return MainHeader().GetMaxTime();
}
#endif
#endif
