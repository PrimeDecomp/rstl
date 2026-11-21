#include "Kyoto/Audio/CAudioManager.hpp"

#include "Kyoto/Audio/CAudioVoice.hpp"
#include "FMOD/fmod.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/CRandom16.hpp"
#include <string.h>


rstl::reserved_vector< CAudioVoice, 96 > CAudioManager::mVoices;
FMOD::System* CAudioManager::mpFMODSystem = nullptr;
static CRandom16 sRandom(55);                                     // Guessed name
static int sVoiceContext = 0;                                     // Guessed name
static uint sDebugOptions = 0;                                    // Guessed name
static int sARAMOffset = 0;                                       // Guessed name
static int sARAMSize = 0;                                         // Guessed name
static FMOD_SPEAKERMODE sSpeakerMode = FMOD_SPEAKERMODE_PROLOGIC; // Guessed name

// Guessed name
static void* AllocateAudioMemory(uint size) {
  return rs_new_line_in("CAudioManager.cpp", 79) uchar[size];
}

// Guessed name
static void* ReallocateAudioMemory(void* memory, uint size) {
  void* result = rs_new_line_in("CAudioManager.cpp", 84) uchar[size];
  memcpy(result, memory, size);
  CMemory::Free(memory);
  return result;
}

// Guessed name
static void FreeAudioMemory(void* memory) { CMemory::Free(memory); }

void CAudioManager::InitializeWithMemoryCallbacks(int aramOffset, int aramSize) {
  FMOD_Memory_Initialize(nullptr, 0, AllocateAudioMemory, ReallocateAudioMemory, FreeAudioMemory);
  sARAMOffset = aramOffset;
  sARAMSize = aramSize;
  InitializeSystem();
}

void CAudioManager::RestartSystem() {
  if (mpFMODSystem != nullptr) {
    Shutdown();
  }
  InitializeSystem();
}

void CAudioManager::InitializeSystem() {
  FMOD_System_Create(reinterpret_cast< FMOD_SYSTEM** >(&mpFMODSystem));
  InitializeSystemHook(0);
  int aramSettings[2] = {0, 0};
  aramSettings[0] = sARAMOffset;
  aramSettings[1] = sARAMSize;
  mpFMODSystem->setSpeakerMode(sSpeakerMode);
  mpFMODSystem->setHardwareChannels(0, 32, 0, 0);
  mpFMODSystem->init(128, FMOD_INIT_3D_RIGHTHANDED | FMOD_INIT_SOFTWARE_DISABLE, aramSettings);
  mpFMODSystem->set3DNumListeners(1);
  mpFMODSystem->set3DSettings(1.f, 1.f, 1.f);

  for (int i = 0; i < 96; ++i) {
    mVoices.push_back(CAudioVoice());
  }
}

void CAudioManager::Shutdown() {
  StopAllVoices();
  mVoices.clear();
  mpFMODSystem->release();
  mpFMODSystem = nullptr;
}

void CAudioManager::Update(float dt) {
  mpFMODSystem->update();
  for (rstl::reserved_vector< CAudioVoice, 96 >::iterator it = mVoices.begin(); it != mVoices.end();
       ++it) {
    if (it->HasAllocatedHandle()) {
      it->Update(dt);
    }
  }
}

void CAudioManager::SetListenerVectors(const CVector3f& position, const CVector3f& velocity,
                                       const CVector3f& forward, const CVector3f& up) {
  FMOD_VECTOR fmodPosition = {0.f, 0.f, 0.f};
  FMOD_VECTOR fmodVelocity = {0.f, 0.f, 0.f};
  FMOD_VECTOR fmodForward = {0.f, 0.f, 0.f};
  FMOD_VECTOR fmodUp = {0.f, 0.f, 0.f};
  fmodPosition.x = position.GetX();
  fmodPosition.y = position.GetY();
  fmodPosition.z = position.GetZ();
  fmodVelocity.x = velocity.GetX();
  fmodVelocity.y = velocity.GetY();
  fmodVelocity.z = velocity.GetZ();
  fmodForward.x = forward.GetX();
  fmodForward.y = forward.GetY();
  fmodForward.z = forward.GetZ();
  fmodUp.x = up.GetX();
  fmodUp.y = up.GetY();
  fmodUp.z = up.GetZ();
  mpFMODSystem->set3DListenerAttributes(0, &fmodPosition, &fmodVelocity, &fmodForward, &fmodUp);
}

float CAudioManager::GetTotalCpuPercent() {
  float dsp, stream, update, total;
  mpFMODSystem->getCPUUsage(&dsp, &stream, &update, &total);
  return total;
}

float CAudioManager::GetDspCpuPercent() {
  float dsp, stream, update, total;
  mpFMODSystem->getCPUUsage(&dsp, &stream, &update, &total);
  return dsp;
}

float CAudioManager::GetStreamCpuPercent() {
  float dsp, stream, update, total;
  mpFMODSystem->getCPUUsage(&dsp, &stream, &update, &total);
  return stream;
}

float CAudioManager::GetUpdateCpuPercent() {
  float dsp, stream, update, total;
  mpFMODSystem->getCPUUsage(&dsp, &stream, &update, &total);
  return update;
}

int CAudioManager::GetFMODAllocatedBytes() {
  int current, maximum;
  FMOD_Memory_GetStats(&current, &maximum);
  return current;
}

int CAudioManager::GetChannelsPlaying() {
  int channels;
  mpFMODSystem->getChannelsPlaying(&channels);
  return channels;
}

void CAudioManager::InitializeSystemHook(int value) {}

CAudioHandle CAudioManager::AllocateVoice() {
  for (rstl::reserved_vector< CAudioVoice, 96 >::iterator it = mVoices.begin(); it != mVoices.end();
       ++it) {
    if (!it->HasAllocatedHandle()) {
      CAudioHandle handle(it - mVoices.begin());
      mVoices[handle.GetVoiceIndex()].SetHandle(handle);
      return handle;
    }
  }
  return CAudioHandle();
}

CAudioVoice* CAudioManager::LookupVoice(const CAudioHandle& handle) {
  return FindVoiceForHandle(handle);
}

CAudioVoice* CAudioManager::FindVoiceForHandle(const CAudioHandle& handle) {
  if (!handle) {
    return nullptr;
  }
  int index = handle.GetVoiceIndex();
  if (index >= 0 && index < mVoices.size()) {
    CAudioVoice& voice = mVoices[index];
    if (voice.GetHandle() == handle) {
      return &voice;
    }
  }
  return nullptr;
}

CRandom16& CAudioManager::GetRandom() { return sRandom; }

void CAudioManager::SetSfxVolume(float volume) {}

void CAudioManager::SetSpeakerModeIndex(int mode) {
  switch (mode) {
  case 0:
    sSpeakerMode = FMOD_SPEAKERMODE_MONO;
    break;
  case 1:
    sSpeakerMode = FMOD_SPEAKERMODE_STEREO;
    break;
  case 2:
    sSpeakerMode = FMOD_SPEAKERMODE_PROLOGIC;
    break;
  }
}

int CAudioManager::GetSpeakerModeIndex() {
  switch (sSpeakerMode) {
  case FMOD_SPEAKERMODE_MONO:
    return 0;
  case FMOD_SPEAKERMODE_STEREO:
    return 1;
  case FMOD_SPEAKERMODE_PROLOGIC:
    return 2;
  default:
    return -1;
  }
}

void CAudioManager::StopAllVoices() {
  for (rstl::reserved_vector< CAudioVoice, 96 >::iterator it = mVoices.begin(); it != mVoices.end();
       ++it) {
    if (it->HasAllocatedHandle()) {
      it->StopWithFade();
    }
  }
}

void CAudioManager::StopContextVoices(int context) {
  for (rstl::reserved_vector< CAudioVoice, 96 >::iterator it = mVoices.begin(); it != mVoices.end();
       ++it) {
    if (it->HasAllocatedHandle() && context == it->GetContext()) {
      it->StopWithFade();
    }
  }
}

void CAudioManager::SetContextPaused(int context, bool paused) {
  for (rstl::reserved_vector< CAudioVoice, 96 >::iterator it = mVoices.begin(); it != mVoices.end();
       ++it) {
    if (it->HasAllocatedHandle() && context == it->GetContext()) {
      it->SetPaused(paused);
    }
  }
}

void CAudioManager::SetVoiceContext(int context) {
  if (context == sVoiceContext) {
    return;
  }
  if (context == 1 && sVoiceContext == 0) {
    StopContextVoices(0);
    SetContextPaused(1, false);
  } else if (context == 0 && sVoiceContext == 1) {
    SetContextPaused(1, true);
  } else {
    StopAllVoices();
  }
  sVoiceContext = context;
}

int CAudioManager::GetVoiceContext() { return sVoiceContext; }

void CAudioManager::UpdateVoiceIdSets(int currentArea,
                                      const rstl::reserved_vector< int, 16 >& visibleAreas,
                                      const rstl::reserved_vector< int, 16 >& otherAreas) {
  for (rstl::reserved_vector< CAudioVoice, 96 >::iterator it = mVoices.begin(); it != mVoices.end();
       ++it) {
    if (!it->HasAllocatedHandle() || it->GetAreaId() == -1) {
      continue;
    }
    int area = it->GetAreaId();
    bool visible = false;
    bool alive = false;
    for (rstl::reserved_vector< int, 16 >::const_iterator a = visibleAreas.begin();
         a != visibleAreas.end(); ++a) {
      if (area == *a) {
        visible = true;
        alive = true;
        break;
      }
    }
    if (!visible) {
      for (rstl::reserved_vector< int, 16 >::const_iterator a = otherAreas.begin();
           a != otherAreas.end(); ++a) {
        if (area == *a) {
          alive = true;
          break;
        }
      }
    }
    if (!alive) {
      it->StopWithFade();
    } else {
      bool muted = !visible;
      if (muted != it->IsMuted()) {
        it->SetMuted(muted);
      }
    }
  }
}

void CAudioManager::SetMuted(bool muted) {}

rstl::vector< rstl::string > CAudioManager::BuildVoiceDebugStrings() {
  rstl::vector< rstl::string > result;
  result.reserve(mVoices.size());
  for (rstl::reserved_vector< CAudioVoice, 96 >::iterator it = mVoices.begin(); it != mVoices.end();
       ++it) {
    if (it->HasAllocatedHandle()) {
      rstl::string line = CBasics::Stringize("%s %3.0f%% %3.3f %c%c%02d", it->GetSoundName().data(),
                                             100.f * it->GetMaximumChildAudibility(),
                                             it->GetElapsedSeconds(), it->IsLooping() ? 'L' : 'O',
                                             it->IsSpatial() ? '3' : '2', it->GetAreaId());
      result.push_back(line);
    }
  }
  return result;
}

rstl::vector< CAudioManager::SSoundDebugRecord > CAudioManager::BuildSpatialVoiceDebugRecords() {
  rstl::vector< SSoundDebugRecord > result;
  result.reserve(mVoices.size());
  for (rstl::reserved_vector< CAudioVoice, 96 >::iterator it = mVoices.begin(); it != mVoices.end();
       ++it) {
    if (it->HasAllocatedHandle() && it->IsSpatial()) {
      rstl::pair< float, float > distances = it->GetMinimumMaximumDistance();
      result.push_back(SSoundDebugRecord(it->GetPosition(), distances.first, distances.second,
                                         it->GetSoundName()));
    }
  }
  return result;
}

void CAudioManager::SetDebugOption(int option, bool enabled) {
  sDebugOptions = enabled ? sDebugOptions | (1 << option) : sDebugOptions & ~(1 << option);
}

bool CAudioManager::GetDebugOption(int option) { return (sDebugOptions & (1 << option)) != 0; }

int CAudioManager::AddLowPassAreaFilter(int frequency, float duration) { return 0; }

int CAudioManager::AddCameraLowPassFilter(int frequency, float duration) { return 0; }

void CAudioManager::RemoveCameraLowPassFilter(int id) {}
