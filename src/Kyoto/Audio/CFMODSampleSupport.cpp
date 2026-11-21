#include "rstl/RstlVersions.h"
#if RSTL_KYOTO_FMOD_AUDIO
#include "Kyoto/Audio/CFMODSampleSupport.hpp"

#include "string.h"


FMOD::Sound* CFMODSampleSupport::CreateSoundFromMemory(FMOD::System* system, const void* data,
                                                       int length, FMOD_MODE mode,
                                                       const SObjectTag& tag) {
  FMOD_CREATESOUNDEXINFO info;
  memset(&info, 0, sizeof(info));
  info.cbsize = sizeof(info);
  info.length = length;
  FMOD::Sound* sound = nullptr;
  system->createSound(static_cast< const char* >(data), mode | FMOD_HARDWARE | FMOD_OPENMEMORY,
                      &info, &sound);
  return sound;
}
#endif
