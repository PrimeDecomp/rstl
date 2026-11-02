#ifndef _DOLPHINCRSFAUDIO
#define _DOLPHINCRSFAUDIO

#include "rstl/RstlVersions.h"

#include "types.h"

#if RSTL_PRIME1
#include "dolphin/hw_regs.h"
#endif

#include "Kyoto/Audio/g721.h"
#include "rstl/auto_ptr.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

class CDvdRequest;

class CRSFAudio {
public:
  CRSFAudio(const rstl::string& audioFile, const int loopStart, const int loopEnd);
  ~CRSFAudio();

  const bool IsFullyLoaded() const;
  void Play();
  void Stop();

  // Guessed names for the balanced Wii audio-suppression API.
  static void PushMute();
  static void PopMute();

#if defined(TARGET_PC)
  static void MixCallback(short* output, size_t frames, u32 rate, u32 channels);
  void DoMix(short* output, size_t frames, u32 rate, u32 channels);
#else
  static void MixCallback();
  void DoMix();
#endif

  void Decode(ushort* out, const ushort* in, int numSamples);
  void DecodeMonoAndMix(ushort* out, const ushort* in, int numSamples, int startSample,
                        int sampleEnd, int sampleStart, int vol, g72x_state& state);
  void SetVolume(uchar vol);

private:
  rstl::string mFilepath;
  int mRsfRem;
  int mRsfLength;
  int mCurSamp;
  int mLoopStartSamp;
  int mLoopEndSamp;
  int mCurBuf;
  rstl::auto_ptr< uchar > mDmaBufferA;
  rstl::auto_ptr< uchar > mDmaBufferB;
  rstl::vector< rstl::auto_ptr< CDvdRequest > > mDvdRequests;
  rstl::vector< rstl::auto_ptr< uchar > > mBuffers;
  g72x_state mLeftState;
  g72x_state mRightState;
  ushort mVolume;
#if defined(TARGET_PC)
  u32 m_audioPhase = 0;
  short m_audioHistory[2][2] = {};
  short m_audioPair[4] = {};
  u32 m_audioPairRead = 2;
  bool m_audioPrimed = false;
#endif
};

CHECK_SIZEOF(CRSFAudio, 0xC4)

#endif // _DOLPHINCRSFAUDIO
