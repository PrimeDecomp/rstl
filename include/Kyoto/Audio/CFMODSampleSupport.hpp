#ifndef _CFMODSAMPLESUPPORT
#define _CFMODSAMPLESUPPORT

#include "rstl/types.h"
#include "FMOD/fmod.hpp"


struct SObjectTag;

class CFMODSampleSupport { // Guessed name
public:
  // Guessed name
  static FMOD::Sound* CreateSoundFromMemory(FMOD::System* system, const void* data, int length,
                                            FMOD_MODE mode, const SObjectTag& tag);
};


#endif // _CFMODSAMPLESUPPORT
