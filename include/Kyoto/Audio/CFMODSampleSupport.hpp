#ifndef _CFMODSAMPLESUPPORT
#define _CFMODSAMPLESUPPORT

#include "rstl/types.h"
#include "rstl/RstlVersions.h"
#if RSTL_KYOTO_FMOD_AUDIO
#include "FMOD/fmod.hpp"
#else
namespace FMOD {
class Channel;
class Sound;
class System;
} // namespace FMOD
struct FMOD_VECTOR;
typedef unsigned int FMOD_MODE;
#endif


struct SObjectTag;

class CFMODSampleSupport { // Guessed name
public:
  // Guessed name
  static FMOD::Sound* CreateSoundFromMemory(FMOD::System* system, const void* data, int length,
                                            FMOD_MODE mode, const SObjectTag& tag);
};


#endif // _CFMODSAMPLESUPPORT
