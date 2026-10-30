#include "rstl/RstlVersions.h"
#include "Kyoto/Audio/CAudioGroupSet.hpp"
#include "Kyoto/Basics/CBasics.hpp"

#include "Kyoto/Audio/CAudioSys.hpp"
#include "Kyoto/CARAMManager.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/RAssertDolphin.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"

// TODO: verify that this Prime/Echoes difference is real
#include "rstl/auto_ptr.hpp"
#include "rstl/StringExtras.hpp"
#include "rstl/string.hpp"
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
#include "dolphin/os.h"
#include "musyx/musyx.h"
#endif

#include "string.h"

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CAudioGroupSet::CAudioGroupSet(const TLockedToken< CAudioGrpSetLoc >& group)
: mBaseDir(group->GetBaseDirName())
, mGroupSetName(group->GetGroupSetName())
, mGroupSetTok(group) {}

CAudioGroupSet::~CAudioGroupSet() {}

void CAudioGroupSet::Reload() {}

void CAudioGroupSet::FreeSampleBuffer() { mGroupSetTok.data()->FreeSampleBuffer(); }

CAudioGrpSetLoc::CAudioGrpSetLoc(const rstl::auto_ptr< uchar >& data, int length)
: mData(data.release())
, mAramSize(0)
, mPool(nullptr)
, mProject(nullptr)
, mSampleDir(nullptr)
, mSamples(nullptr) {
  uint readPosition;
  const uint poolSize = ReadHeader(data.get(), length, readPosition);
  CAudioSys::GetVerbose();

  uint projectSize;
  const uint projectOffset = readPosition + poolSize;
#if TARGET_LITTLE_ENDIAN
  projectSize = CBasics::SwapBytes(*reinterpret_cast< uint* >(data.get() + projectOffset));
#else
  projectSize = *reinterpret_cast< uint* >(data.get() + projectOffset);
#endif
  CAudioSys::GetVerbose();

  const uint sampOffset = 4 + projectOffset + projectSize;
#if TARGET_LITTLE_ENDIAN
  const uint sampSize = CBasics::SwapBytes(*reinterpret_cast< uint* >(data.get() + sampOffset));
#else
  const uint sampSize = *reinterpret_cast< uint* >(data.get() + sampOffset);
#endif
  CAudioSys::GetVerbose();
  mAramSize = sampSize;

  uint sdirSize;
  const uint sdirOffset = 4 + sampOffset + sampSize;
#if TARGET_LITTLE_ENDIAN
  sdirSize = CBasics::SwapBytes(*reinterpret_cast< uint* >(data.get() + sdirOffset));
#else
  sdirSize = *reinterpret_cast< uint* >(data.get() + sdirOffset);
#endif
  CAudioSys::GetVerbose();

  mGroupData = rstl::auto_ptr< uchar >(static_cast< uchar* >(
      CMemory::Alloc(poolSize + projectSize + sdirSize + 8, IAllocator::kHI_RoundUpLen)));
  uchar* ptr = mData.get();
  mPool = mGroupData.get();
  memcpy(mPool, ptr + readPosition, poolSize);

  uint roundedPoolSize = ((poolSize + 3) & ~3);
  mProject = mGroupData.get() + roundedPoolSize;
  memcpy(mProject, ptr + (projectOffset + 4), projectSize);

  uint roundedProjectSize = ((projectSize + 3) & ~3);
  roundedProjectSize = roundedPoolSize + roundedProjectSize;
  mSampleDir = mGroupData.get() + roundedProjectSize;
  memcpy(mSampleDir, ptr + (sdirOffset + 4), sdirSize);
  mSamples = &ptr[sampOffset + 4];
#else
extern "C" {
u32 sndStackGetSize(void);
u32 sndStackAdd(void* buffer, const void* aramAddr, u32 size);
bool sndStackSetCurrent(u32 id);
void sndStackRemove(u32 id);
#endif

}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CAudioGrpSetLoc::FreeSampleBuffer() const {
  mData = nullptr;
  mSamples = nullptr;
#else
bool CAudioGrpSetLoc::sInSinglePlayer;
static uchar* sUploadBuffer;

static void* UploadCallback(u32 offset, u32 bytes) {
  memcpy(sUploadBuffer, reinterpret_cast< void* >(offset), bytes);
  DCFlushRange(sUploadBuffer, bytes);
  return sUploadBuffer;
#endif

}

// TODO: verify that this Prime/Echoes difference is real
RSTL_PRIME1_CONST CFactoryFnReturn FAudioGroupSetLocDataFactory(const SObjectTag& tag,

                                                    const rstl::auto_ptr< uchar >& data, int length,
                                                    const CVParamTransfer& xfer) {
  return rs_new CAudioGrpSetLoc(data, length);
}

#if !RSTL_PRIME1
CAudioGrpSetLoc::CAudioGrpSetLoc(const rstl::auto_ptr< uchar >& data, int length)
: mStackId(-2), mAramAlloc(CARAMManager::GetInvalidAlloc()) {
  CMemoryInStream in(data.get(), length);
  if (in.ReadInt32() <= 1) {
    rstl::string name(in);
    const uint groupId = static_cast< ushort >(in.ReadUint16());
    const int poolSize = in.ReadInt32();
    const int projectSize = in.ReadInt32();
    const int sampleDirSize = in.ReadInt32();
    const int sampleSize = in.ReadInt32();

    mPool = rs_new uchar[poolSize];
    in.ReadBytes(mPool.get(), poolSize);
    mProject = rs_new uchar[projectSize];
    in.ReadBytes(mProject.get(), projectSize);
    mSampleDir = rs_new uchar[sampleDirSize];
    in.ReadBytes(mSampleDir.get(), sampleDirSize);

    uchar* samples = data.get() + in.GetReadPosition();
    const int alignedSize = (sampleSize + 31) & ~31;
    mAramAlloc = CARAMManager::Alloc(alignedSize, 1);
    if (mAramAlloc == CARAMManager::GetInvalidAlloc()) {
      return;
    }

    mStackBuffer = rs_new uchar[sndStackGetSize()];
    mStackId = sndStackAdd(mStackBuffer.get(), mAramAlloc, alignedSize);
    if (mStackId == -1) {
      CARAMManager::Free(mAramAlloc, 1);
      mAramAlloc = CARAMManager::GetInvalidAlloc();
      return;
    }

    uchar uploadBuffer[0x1000 + 31];
    sUploadBuffer = reinterpret_cast< uchar* >(reinterpret_cast< uint >(uploadBuffer + 31) & ~31);
    sndSetSampleDataUploadCallback(UploadCallback, 0x1000);
    if (!sndStackSetCurrent(mStackId)) {
      CARAMManager::Free(mAramAlloc, 1);
      mAramAlloc = CARAMManager::GetInvalidAlloc();
      return;
    }

    if (!sndPushGroup(mProject.get(), groupId, samples, mSampleDir.get(), mPool.get())) {
      CARAMManager::Free(mAramAlloc, 1);
      mAramAlloc = CARAMManager::GetInvalidAlloc();
      sndStackRemove(mStackId);
      sndStackSetCurrent(-2);
      mStackId = -2;
      return;
    }

    sndSetSampleDataUploadCallback(nullptr, 0);
    sndStackSetCurrent(-2);

    if (sInSinglePlayer) {
      if (CStringExtras::IndexOfSubstring(name, rstl::string_l("_MP")) != -1 ||
          CStringExtras::IndexOfSubstring(name, rstl::string_l("Multiplayer")) != -1) {
        char* str = static_cast< char* >(__alloca(name.size() + 1));
        strcpy(str, name.data());
        rs_debugger_printf("Multi sound %s in single", str);
      }
    }
  }
}

CAudioGrpSetLoc::~CAudioGrpSetLoc() {
  if (mStackId != -2 && mStackId != -1) {
    sndStackSetCurrent(mStackId);
    sndPopGroup();
    sndStackRemove(mStackId);
    sndStackSetCurrent(-2);
  }
  if (mAramAlloc != CARAMManager::GetInvalidAlloc()) {
    CARAMManager::Free(mAramAlloc, 1);
  }
}
#endif

