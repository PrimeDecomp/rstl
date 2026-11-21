#ifndef _CAUDIOCHANNEL
#define _CAUDIOCHANNEL

#include "rstl/static_assert.hpp"
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


class CVector3f;

class CAudioChannel { // Guessed name
public:
  CAudioChannel(FMOD::Channel* channel);
  operator bool() const { return mChannel != nullptr; }
  bool IsPlaying() const;                                                            // Guessed name
  void Stop();                                                                       // Guessed name
  void SetMuted(bool muted);                                                         // Guessed name
  void SetPaused(bool paused);                                                       // Guessed name
  float GetAudibility() const;                                                       // Guessed name
  void SetVolume(const float& volume);                                               // Guessed name
  void SetPlaybackFrequency(int frequency);                                          // Guessed name
  int GetSampleBaseFrequency() const;                                                // Guessed name
  void SetPosition(const CVector3f& position);                                       // Guessed name
  void SetPositionAndVelocity(const CVector3f& position, const CVector3f& velocity); // Guessed name
  void SetMinimumMaximumDistance(float minimum, float maximum);                      // Guessed name
  void SetDistanceRolloffPoints(FMOD_VECTOR* points, int count);                     // Guessed name
  void SetSpeakerMixCoordinates(float pan, float surroundPan);                       // Guessed name

private:
  FMOD::Channel* mChannel;
};
CHECK_SIZEOF(CAudioChannel, 0x4)


#endif // _CAUDIOCHANNEL
