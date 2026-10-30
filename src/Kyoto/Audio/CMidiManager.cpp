#include "rstl/RstlVersions.h"
#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Audio/CMidiManager.hpp"

#include "Kyoto/Audio/CAudioSys.hpp"

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#include "Kyoto/CFactoryFnReturn.hpp"
#else
#include "Kyoto/CFactoryMgr.hpp"
#endif

#include "Kyoto/Streams/CInputStream.hpp"

rstl::reserved_vector< CMidiManager::CMidiWrapper, 3 > CMidiManager::mMidiWrappers;

CMidiManager::CMidiWrapper::CMidiWrapper() : mSysHandle(0), mAvailable(true) {}

const CSfxHandle& CMidiManager::CMidiWrapper::GetManagerHandle() const { return mMidiHandle; }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
const u32 CMidiManager::CMidiWrapper::GetAudioSysHandle() const { return mSysHandle; }
#else
const uint CMidiManager::CMidiWrapper::GetAudioSysHandle() const { return mSysHandle; }
#endif

const bool CMidiManager::CMidiWrapper::IsAvailable() const { return mAvailable; }

const short CMidiManager::CMidiWrapper::GetSongId() const { return mSongId; }

void CMidiManager::CMidiWrapper::SetAvailable(const bool v) { mAvailable = v; }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CMidiManager::CMidiWrapper::SetAudioSysHandle(const u32 handle) { mSysHandle = handle; }
#else
void CMidiManager::CMidiWrapper::SetAudioSysHandle(const uint handle) { mSysHandle = handle; }
#endif

void CMidiManager::CMidiWrapper::SetMidiHandle(const CSfxHandle& handle) { mMidiHandle = handle; }

void CMidiManager::CMidiWrapper::SetSongId(const short id) { mSongId = id; }

CSfxHandle CMidiManager::Play(const CMidiData& data, unsigned short fadeTime, bool stopExisting,
                              short volume) {
  bool foundExisting = false;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  u32 sysHandle = 0;
#else
  uint sysHandle = 0;
#endif

  CSfxHandle handle = LocateHandle();
  if (!handle) {
    return CSfxHandle();
  }
  CMidiWrapper& wrapper = mMidiWrappers[handle.GetIndex()];
  wrapper.SetAvailable(false);
  wrapper.SetMidiHandle(handle);
  if (stopExisting) {
    for (int i = 0; i < mMidiWrappers.size(); ++i) {
      if (mMidiWrappers[i].IsAvailable()) {
        continue;
      }

      if (data.GetSongId() == mMidiWrappers[i].GetSongId()) {
        foundExisting = true;
        sysHandle = mMidiWrappers[i].GetAudioSysHandle();
        mMidiWrappers[i].SetAvailable(true);
      } else {
        Stop(mMidiWrappers[i].GetManagerHandle(), fadeTime);
      }
    }
  }

  if (foundExisting) {
    wrapper.SetAudioSysHandle(sysHandle);
    wrapper.SetSongId(data.GetSongId());
  } else {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    u32 sysHandle = CAudioSys::SeqPlayEx(data.GetGroupId(), data.GetSongId(), data.GetData(), nullptr, 0);
#else
    uint sysHandle = CAudioSys::SeqPlayEx(data.GetGroupId(), data.GetSongId(), data.GetData(), nullptr, 0);
#endif

    if (fadeTime != 0) {
      CAudioSys::SeqVolume(0, 0, sysHandle, 0);
    }
    CAudioSys::SeqVolume(volume, fadeTime, sysHandle, 0);
    wrapper.SetAudioSysHandle(sysHandle);
    wrapper.SetSongId(data.GetSongId());
  }

  return handle;
}

void CMidiManager::Stop(const CSfxHandle& handle, ushort fadeTime) {
  if (!handle) {
    return;
  }

  if (handle != mMidiWrappers[handle.GetIndex()].GetManagerHandle()) {
    return;
  }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  u32 sysHandle = mMidiWrappers[handle.GetIndex()].GetAudioSysHandle();
#else
  uint sysHandle = mMidiWrappers[handle.GetIndex()].GetAudioSysHandle();
#endif

  if (fadeTime == 0) {
    CAudioSys::SeqStop(sysHandle);
  } else {
    CAudioSys::SeqVolume(0, fadeTime, sysHandle, 1);
  }

  mMidiWrappers[handle.GetIndex()].SetAvailable(true);
}

void CMidiManager::StopAll() {
  for (int i = 0; i < mMidiWrappers.size(); ++i) {
    if (!mMidiWrappers[i].IsAvailable()) {
      Stop(mMidiWrappers[i].GetManagerHandle(), 0);
    }
  }
}

CSfxHandle CMidiManager::LocateHandle() {
  for (int i = 0; i < mMidiWrappers.size(); ++i) {
    if (mMidiWrappers[i].IsAvailable()) {
      return CSfxHandle(i);
    }
  }

  if (mMidiWrappers.size() == mMidiWrappers.capacity()) {
    return CSfxHandle();
  }

  mMidiWrappers.push_back(CMidiWrapper());
  return CSfxHandle(mMidiWrappers.size() - 1);
}

CMidiManager::CMidiData::CMidiData(CInputStream& in)
: mSongId(-1), mGroupId(-1), mAgscId(-1) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  in.ReadLong();
  mSongId = in.ReadLong();
  mGroupId = in.ReadLong();
  mAgscId = in.ReadLong();
#else
  in.ReadInt32();
  mSongId = in.ReadInt32();
  mGroupId = in.ReadInt32();
  mAgscId = in.ReadInt32();
#endif

  int len = in.ReadInt32();
  mData = rs_new uchar[len];
  in.Get(mData.get(), len);
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
const CFactoryFnReturn FMidiDataFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer&) {
#else
CFactoryFnReturn FMidiDataFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer&) {
#endif

  return rs_new CMidiManager::CMidiData(in);
}

