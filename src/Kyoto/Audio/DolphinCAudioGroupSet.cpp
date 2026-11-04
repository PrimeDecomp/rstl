#include "Kyoto/Audio/CAudioGroupSet.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Audio/CAudioSys.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Basics/RAssertDolphin.hpp"
#include "Kyoto/CARAMManager.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"

#include "dolphin/os.h"
#include "musyx/musyx.h"
#include "musyx/synthdata.h"
#include "rstl/RstlVersions.h"
#include "rstl/StringExtras.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/string.hpp"

#include "string.h"

#if defined(TARGET_PC)
#include "Metaforce/Audio.hpp"
#include "Metaforce/Common.hpp"

namespace {
constexpr borealis::Log Log{"CAudioGroupSet"};
}
#endif

bool CAudioGrpSetLoc::sInSinglePlayer;
static uchar* sUploadBuffer;

static void* UploadCallback(u32 offset, u32 bytes) {
  memcpy(sUploadBuffer, reinterpret_cast< void* >(offset), bytes);
  DCFlushRange(sUploadBuffer, bytes);
  return sUploadBuffer;
}

CAudioGroupSet::CAudioGroupSet(const TLockedToken< CAudioGrpSetLoc >& group)
: mBaseDir(group->GetBaseDirName()), mGroupSetName(group->GetGroupSetName()), mGroupSetTok(group) {}

CAudioGroupSet::~CAudioGroupSet() {}

void CAudioGroupSet::Reload() {}

void CAudioGroupSet::FreeSampleBuffer() { mGroupSetTok.data()->FreeSampleBuffer(); }

#if !RSTL_PRIME1
RSTL_PRIME1_CONST CFactoryFnReturn FAudioGroupSetLocDataFactory(const SObjectTag& tag,
                                                                const rstl::auto_ptr< uchar >& data,
                                                                int length,
                                                                const CVParamTransfer& xfer) {
  return rs_new CAudioGrpSetLoc(data, length);
}
#endif

CAudioGrpSetLoc::CAudioGrpSetLoc(const rstl::auto_ptr< uchar >& data, int length)
#if RSTL_PRIME1
: mData(data.release())
, mAramSize(0)
, mPool(nullptr)
, mProject(nullptr)
, mSampleDir(nullptr)
, mSamples(nullptr) {
#if defined(TARGET_PC)
  metaforce::AudioGroupView view;
  REQUIRE(length >= 0 && metaforce::ReadAudioGroup({mData.get(), size_t(length)}, view),
          "Invalid AGSC wrapper ({} bytes)", length);
  SND_PC_ASSET_ERROR error{};
  REQUIRE(sndPCValidateGroup(&view.assets, &error), "Invalid AGSC {} at {}+{}: {}", view.name,
          error.section ? error.section : "", error.offset, error.reason ? error.reason : "");
  mBaseDirName = rstl::string(view.baseDirectory.data(), view.baseDirectory.size());
  mGroupSetName = rstl::string(view.name.data(), view.name.size());
  const size_t poolSize = (view.assets.pool.size + 3) & ~size_t(3);
  const size_t projectSize = (view.assets.project.size + 3) & ~size_t(3);
  mGroupData = rstl::auto_ptr< uchar >(static_cast< uchar* >(CMemory::Alloc(
      poolSize + projectSize + view.assets.directory.size, IAllocator::kHI_RoundUpLen)));
  mPool = mGroupData.get();
  mProject = mPool + poolSize;
  mSampleDir = mProject + projectSize;
  memcpy(mPool, view.assets.pool.data, view.assets.pool.size);
  memcpy(mProject, view.assets.project.data, view.assets.project.size);
  memcpy(mSampleDir, view.assets.directory.data, view.assets.directory.size);
  mSamples = const_cast< uchar* >(static_cast< const uchar* >(view.assets.samples.data));
  mAramSize = view.assets.samples.size;
#else
  uint readPosition;
  const uint poolSize = ReadHeader(data.get(), length, readPosition);
  CAudioSys::GetVerbose();

  uint projectSize;
  const uint projectOffset = readPosition + poolSize;
  projectSize = BE_SWAP(*reinterpret_cast< uint* >(data.get() + projectOffset));
  CAudioSys::GetVerbose();

  const uint sampOffset = 4 + projectOffset + projectSize;
  const uint sampSize = BE_SWAP(*reinterpret_cast< uint* >(data.get() + sampOffset));
  CAudioSys::GetVerbose();
  mAramSize = sampSize;

  uint sdirSize;
  const uint sdirOffset = 4 + sampOffset + sampSize;
  sdirSize = BE_SWAP(*reinterpret_cast< uint* >(data.get() + sdirOffset));
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
#endif
}

#else
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
    mStackId = sndStackAdd(mStackBuffer.get(), reinterpret_cast< u32 >(mAramAlloc), alignedSize);
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
#endif

#if RSTL_PRIME1
void CAudioGrpSetLoc::FreeSampleBuffer() const {
  mData = nullptr;
  mSamples = nullptr;
}

RSTL_PRIME1_CONST CFactoryFnReturn FAudioGroupSetLocDataFactory(const SObjectTag& tag,
                                                                const rstl::auto_ptr< uchar >& data,
                                                                int length,
                                                                const CVParamTransfer& xfer) {
  return rs_new CAudioGrpSetLoc(data, length);
}
#endif

#if !RSTL_PRIME1
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
