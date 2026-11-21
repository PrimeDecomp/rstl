#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Audio/CAudioSoundEffect.hpp"

#include "Kyoto/Audio/CAudioManager.hpp"
#include "Kyoto/Audio/CAudioVoice.hpp"
#include "Kyoto/CFactoryFnReturn.hpp"
#include "Kyoto/CVParamTransfer.hpp"
#include "Kyoto/IObjectStore.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"


CAudioSoundEffect::CAudioSoundEffect(CInputStream& in, IObjectStore& pool, const SObjectTag& tag)
: x0_(in.ReadUint32())
, mVersion(in.ReadUint32())
, xC_(0.f)
, mName(in)
, x20_(in)
, mDescriptions()
, mCharacterSoundPriority(0.f)
, mLastSelectedDescriptionIndex(-1)
, mSelfToken(pool.GetObj(tag)) {
  mDuration = in.ReadFloat();
  const int blockSize = in.ReadInt32();
  int consumed = 0;
  if (mVersion > 1) {
    consumed = 4;
    mCharacterSoundPriority = in.ReadFloat();
  }
  for (int i = 0; i < blockSize - consumed; ++i) {
    in.ReadUint8();
  }
  const uint flags = in.ReadUint32();
  mLooping = (flags & 1) != 0;
  mExclusiveCharacterSound = (flags & 2) != 0;
  const int count = in.ReadInt32();
  mDescriptions.reserve(count);
  for (int i = 0; i < count; ++i) {
    int recordSize = 0;
    if (mVersion > 1) {
      recordSize = in.ReadInt32();
    }
    const int start = in.GetReadPosition();
    mDescriptions.push_back(CAudioSampleDescription(in, pool));
    if (mVersion > 1) {
      const int consumed = in.GetReadPosition() - start;
      for (int j = 0; j < recordSize - consumed; ++j) {
        in.ReadUint8();
      }
    }
  }
}

CAudioHandle CAudioSoundEffect::PlayPanned(float volume, float pan) {
  return PlayPanned(-1, volume, pan);
}

CAudioHandle CAudioSoundEffect::PlaySpatial(const CVector3f& position, float volume) {
  return PlaySpatial(-1, position, volume);
}

CAudioHandle CAudioSoundEffect::PlayPanned(int areaId, float volume, float pan) {
  if (CAudioManager::GetDebugOption(0)) {
    return CAudioHandle();
  }
  CAudioHandle handle = CAudioManager::AllocateVoice();
  if (handle) {
    CAudioVoice* voice = CAudioManager::LookupVoice(handle);
    const float clampedVolume = CMath::Clamp(0.f, volume, 1.f);
    voice->Play(mSelfToken, clampedVolume, CMath::Clamp(-1.f, pan, 1.f), areaId, false);
  }
  return handle;
}

CAudioHandle CAudioSoundEffect::PlaySpatial(int areaId, const CVector3f& position, float volume) {
  if (CAudioManager::GetDebugOption(0)) {
    return CAudioHandle();
  }
  CAudioHandle handle = CAudioManager::AllocateVoice();
  if (handle) {
    CAudioVoice* voice = CAudioManager::LookupVoice(handle);
    voice->PlaySpatial(mSelfToken, position, CMath::Clamp(0.f, volume, 1.f), areaId);
  }
  return handle;
}

bool CAudioSoundEffect::IsLooping() const { return mLooping; }

const rstl::string& CAudioSoundEffect::GetName() const { return mName; }

bool CAudioSoundEffect::IsExclusiveCharacterSound() const { return mExclusiveCharacterSound; }

float CAudioSoundEffect::GetCharacterSoundPriority() const { return mCharacterSoundPriority; }

void CAudioSoundEffect::SetLastSelectedDescriptionIndex(int index) {
  mLastSelectedDescriptionIndex = index;
}

int CAudioSoundEffect::GetLastSelectedDescriptionIndex() const {
  return mLastSelectedDescriptionIndex;
}

CAudioSoundEffect::~CAudioSoundEffect() {}

// Guessed name
CFactoryFnReturn AudioSoundEffectFactory(const SObjectTag& tag, const rstl::auto_ptr< uchar >& data,
                                         int size, const CVParamTransfer& param) {
  CMemoryInStream in(data.get(), size);
  const rstl::rc_ptr< IVParamObj > obj = param.GetObj();
  IObjectStore* pool = static_cast< TObjOwnerParam< IObjectStore* >* >(obj.GetPtr())->GetData();
  return rs_new_line(187) CAudioSoundEffect(in, *pool, tag);
}
