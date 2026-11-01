#include "Kyoto/Audio/CStaticAudioPlayer.hpp"

// Echoes renamed this player to CRSFAudio and moved its AI callbacks to CAIInterruptManager.
#define CRSFAudioClass CStaticAudioPlayer
typedef CStaticAudioPlayer CAICallbackOwner;
#include "DolphinCRSFAudio.inc"
