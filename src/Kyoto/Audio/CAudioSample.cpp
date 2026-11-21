#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Audio/CAudioSample.hpp"

#include "Kyoto/Audio/CAudioManager.hpp"
#include "Kyoto/Audio/CFMODSampleSupport.hpp"
#include "Kyoto/Alloc/Assert.hpp"
#include "Kyoto/CFactoryFnReturn.hpp"
#include "Kyoto/CVParamTransfer.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"


CAudioSample::CAudioSample(const rstl::auto_ptr< uchar >& data, int size, const SObjectTag& tag)
: mSound(nullptr), mMode(0), mName() {
  FMOD::System* system = CAudioManager::mpFMODSystem;
  CMemoryInStream in(data.get(), size);
  if (in.ReadInt32() != 'CSMP') {
    RS_WARNING_PRINT("Invalid audio type\n");
    return;
  }
  in.ReadInt32();
  uchar flags = 0;
  bool done = false;
  while (!done) {
    const int chunk = in.ReadInt32();
    const int chunkSize = in.ReadInt32();
    switch (chunk) {
    case 'NAME': {
      mName = rstl::string(in);
      for (int i = 0; i < chunkSize - static_cast< int >(mName.size()) - 1; ++i) {
        in.ReadUint8();
      }
      break;
    }
    case 'INFO': {
      x8_ = in.ReadUint8();
      flags = in.ReadUint8();
      in.ReadUint16();
      in.ReadFloat();
      in.ReadFloat();
      for (int i = 0; i < chunkSize - 12; ++i) {
        in.ReadUint8();
      }
      break;
    }
    case 'DATA': {
      const void* sampleData = data.get() + in.GetReadPosition();
      mMode = FMOD_3D;
      mMode |= FMOD_3D_CUSTOMROLLOFF;
      mMode |= (flags & 1) ? FMOD_LOOP_NORMAL : 0;
      mSound = CFMODSampleSupport::CreateSoundFromMemory(system, sampleData, chunkSize, mMode, tag);
      mSound->set3DMinMaxDistance(0.0001f, 1.f);
      done = true;
      break;
    }
    default:
      for (int i = 0; i < chunkSize; ++i) {
        in.ReadUint8();
      }
      break;
    }
  }
}

CAudioSample::~CAudioSample() {
  if (mSound != nullptr && CAudioManager::mpFMODSystem != nullptr) {
    mSound->release();
  }
}

CAudioChannel CAudioSample::CreateSpatialChannel(const CVector3f& position, float minimum,
                                                 float maximum, bool paused) const {
  FMOD::Sound* sound = mSound;
  if (sound == nullptr) {
    return CAudioChannel(nullptr);
  }
  FMOD::System* system = CAudioManager::mpFMODSystem;
  FMOD::Channel* channel = nullptr;
  sound->setMode(FMOD_3D);
  system->playSound(FMOD_CHANNEL_FREE, mSound, true, &channel);
  CAudioChannel result(channel);
  result.SetMinimumMaximumDistance(minimum, maximum);
  result.SetPosition(position);
  result.SetPaused(paused);
  return result;
}

CAudioChannel CAudioSample::CreatePannedChannel(bool paused) const {
  FMOD::Sound* sound = mSound;
  if (sound == nullptr) {
    return CAudioChannel(nullptr);
  }
  FMOD::System* system = CAudioManager::mpFMODSystem;
  FMOD::Channel* channel = nullptr;
  sound->setMode(FMOD_2D);
  system->playSound(FMOD_CHANNEL_FREE, mSound, true, &channel);
  channel->setPaused(paused);
  return CAudioChannel(channel);
}

// Guessed name
CFactoryFnReturn AudioSampleFactory(const SObjectTag& tag, const rstl::auto_ptr< uchar >& data,
                                    int size, const CVParamTransfer& param) {
  return rs_new_line(227) CAudioSample(data, size, tag);
}
