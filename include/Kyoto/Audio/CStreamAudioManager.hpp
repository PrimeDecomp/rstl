#ifndef _CSTREAMAUDIOMANAGER
#define _CSTREAMAUDIOMANAGER
#include "rstl/RstlVersions.h"

#include "types.h"

#include "rstl/string.hpp"

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

class CStreamAudioManager {
public:
  enum ESoftwareChannel {
    kSC_Default = 0,
    kSC_OneShot = 1,
  };

  static void Update(float dt);
#if RSTL_PRIME1
  static void AddDebugMessages();
#endif

  static void StopAll();
  static void FadeBackIn(float time);
  static void StopOneShot();
  static void StopSfx();
  static void SetMusicVolume(uint vol);
  static void SetSfxVolume(uint vol);
  static void SetMusicUnmute(bool unmute);
  static bool GetMusicUnmute() { return mEnable; }
  static bool GetSfxUnmute() { return mForegroundEnable; }
  static void SetSfxUnmute(bool unmute) { mForegroundEnable = unmute; }

  static void FadeInSoftwareAudio(ESoftwareChannel chan, float fadeTime);
  static void FadeOutSoftwareAudio(ESoftwareChannel chan, float fadeTime);

  static void PlaySoftwareAudio(ESoftwareChannel chan, const rstl::string& fileName, float fadeIn,
                                float fadeOut, int volume, bool music);
  static void StopSoftwareAudio(ESoftwareChannel chan, const rstl::string& fileName);
#if !RSTL_PRIME1
  static rstl::string GetSoftwareAudioFileName(ESoftwareChannel chan);
#endif

  static void SetDefaultAudio(const rstl::string& fileName, float fadeIn, float fadeOut,
                              unsigned char volume);
  static void SetCurrentAudio(const rstl::string& fileName, float fadeIn, float fadeOut,
                              unsigned char volume);

  static void FadeInHardwareAudio(float fadeTime);
  static void FadeOutHardwareAudio(float fadeTime);

private:
  static void HandleMusicUnmute();
  static void StopAudio();
  static void StopAllStreams();
  static void InternalSetVolume(float vol);
  static void UpdateSoftwareChannels(float dt);
  static void UpdateSoftwareChannel(ESoftwareChannel chan, float dt);
#if !RSTL_PRIME1
  static bool IsSoftwareChannelFree(ESoftwareChannel chan);
#endif

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
