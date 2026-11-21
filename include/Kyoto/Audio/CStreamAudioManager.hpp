#ifndef _CSTREAMAUDIOMANAGER
#define _CSTREAMAUDIOMANAGER
#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/RstlVersions.h"
#endif

#if RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#endif
#include "rstl/types.h"

#include "rstl/string.hpp"
// Guessed name
struct SDSPStreamCacheEntry {
  rstl::string mFileName;
  int mPlayState;
  int mVolume;
  float mFadeIn;
  float mFadeOut;
  int mHandle;
  float mFadeFactor;
  bool mMusic;

  SDSPStreamCacheEntry();
  SDSPStreamCacheEntry(int playState, const rstl::string& fileName, int volume, float fadeIn,
                       float fadeOut,
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
                       float fadeFactor,
#endif
                       int handle, bool music);
};
#if RSTL_CORRUPTION
CHECK_SIZEOF(SDSPStreamCacheEntry, 0x2C)
#endif

class CStreamAudioManager {
public:
#if !RSTL_CORRUPTION
  // Guessed names for the balanced Wii audio-suppression API.
  static void PushMute();
  static void PopMute();

#endif
  enum ESoftwareChannel {
    kSC_Default = 0,
    kSC_OneShot = 1,
  };

  static void Update(float dt);
#if !RSTL_CORRUPTION
  static void AddDebugMessages();

#endif
  static void StopAll();
  static void FadeBackIn(float time);
  static void StopOneShot();
  static void StopSfx();
  static void SetMusicVolume(uint vol);
  static void SetSfxVolume(uint vol);
#if !RSTL_CORRUPTION
  static int GetSfxVolume() { return mForegroundVolume; }
#endif
  static void SetMusicUnmute(bool unmute);
  static bool GetMusicUnmute() { return mEnable; }
  static bool GetSfxUnmute() { return mForegroundEnable; }
  static void SetSfxUnmute(bool unmute) { mForegroundEnable = unmute; }

  static void FadeInSoftwareAudio(ESoftwareChannel chan, float fadeTime);
  static void FadeOutSoftwareAudio(ESoftwareChannel chan, float fadeTime);

  static void PlaySoftwareAudio(ESoftwareChannel chan, const rstl::string& fileName, float fadeIn,
                                float fadeOut, int volume, bool music);
  static void StopSoftwareAudio(ESoftwareChannel chan, const rstl::string& fileName);
  static rstl::string GetSoftwareAudioFileName(ESoftwareChannel chan);

  static void SetDefaultAudio(const rstl::string& fileName, float fadeIn, float fadeOut,
                              unsigned char volume);
  static void SetCurrentAudio(const rstl::string& fileName, float fadeIn, float fadeOut,
                              unsigned char volume);

  static void FadeInHardwareAudio(float fadeTime); // Guessed name
  static void FadeOutHardwareAudio(float fadeTime); // Guessed name

private:
  static void HandleMusicUnmute();
  static void StopAudio();
  static void StopAllStreams();
  static void InternalSetVolume(float vol);
  static void UpdateSoftwareChannels(float dt);
  static void UpdateSoftwareChannel(ESoftwareChannel chan, float dt);
  static bool IsSoftwareChannelFree(ESoftwareChannel chan);

  static void StopStreaming(int idx);
  static int GetTargetDSPVolume(int fileVol, bool music);
  static void PlayTrack(const rstl::string& fileName);

  static rstl::string mDefaultAudioFile;
  static rstl::string mCurrentAudioFile;
  static rstl::string mNewAudioFile;

  static int mCurrentState;
  static float mCurrentVolume;
  static float mTargetVolume;
  static int mGlobalVolume;
#if !RSTL_PRIME1
  static int mCurrentGlobalVolume;
#endif

  static int mForegroundVolume;
  static bool mEnable;
  static bool mForegroundEnable;
  static float mVolumeIncrement;
  static float mVolumeIncrement2;
};

#endif // _CSTREAMAUDIOMANAGER
