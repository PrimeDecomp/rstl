#include "rstl/RstlVersions.h"
#if RSTL_CORRUPTION
#include "Kyoto/Audio/CAudioManager.hpp"
#include "Kyoto/Alloc/Assert.hpp"
#endif
#include "Kyoto/Audio/DolphinCRSFAudio.hpp"
#if !RSTL_CORRUPTION
#include "rstl/RstlVersions.h"
#endif
#include "Kyoto/Audio/CAIInterruptManager.hpp"
#if !RSTL_CORRUPTION

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#endif
#include "Kyoto/Audio/CAudioSys.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/Basics/CBasics.hpp"
#include <Kyoto/Alloc/CMemory.hpp>
#endif

#include "Kyoto/CDvdFile.hpp"
#include "Kyoto/CDvdRequest.hpp"

#include "rstl/math.hpp"

#include "dolphin/ai.h"
#include "dolphin/os.h"
#include "stdint.h"

#if !RSTL_CORRUPTION
#if IS_TARGET_PC
#include <algorithm>
#include "Metaforce/Audio.hpp"
#endif
#endif

#if !RSTL_CORRUPTION
#if RSTL_PRIME1
enum { kRSFBufferSize = 0x20000 };
#else
enum { kRSFBufferSize = 0x4000 };
#endif
#endif

static CRSFAudio* sPlayingAudio = nullptr;
#if !RSTL_CORRUPTION
static int sMuteCount = 0;
#endif

CRSFAudio::CRSFAudio(const rstl::string& audioFile, const int loopStart, const int loopEnd)
: mFilepath(audioFile)
, mRsfRem(-1)
, mCurSamp(0)
, mLoopStartSamp(loopStart & ~1)
, mLoopEndSamp(loopEnd & ~1)
, mCurBuf(0)
#if RSTL_CORRUPTION
, mDmaBufferA(static_cast< uchar* >(rs_aligned_new_line(640, 69)))
, mDmaBufferB(static_cast< uchar* >(rs_aligned_new_line(640, 70)))
#else
, mDmaBufferA(static_cast< uchar* >(CMemory::Alloc(640, IAllocator::kHI_RoundUpLen)))
, mDmaBufferB(static_cast< uchar* >(CMemory::Alloc(640, IAllocator::kHI_RoundUpLen)))
#endif
, mVolume(32768) {
#if RSTL_CORRUPTION
  // clang-format off
  RS_VERIFY_THROW(74, CDvdFile::FileExists( audioFile.c_str() ), false, CBasics::Stringize("RSFAudio: File %s does not exist!", audioFile.c_str()));
  // clang-format on
#endif
  CDvdFile dvdFile(audioFile.data());
  mRsfRem = dvdFile.GetFileSize();
  mRsfLength = mRsfRem;
#if RSTL_CORRUPTION
  int bufferCount = ((mRsfRem - 1) + 0x4000) / 0x4000;
#else
#if !RSTL_PRIME1
  uint bufferSize;
#endif
  int bufferCount = ((mRsfRem - 1) + kRSFBufferSize) / kRSFBufferSize;
#endif
  mBuffers.reserve(bufferCount);
  mDvdRequests.reserve(bufferCount);

#if RSTL_CORRUPTION
  for (int i = mRsfRem; i > 0; i -= 0x4000) {
    uint bufferSize = 0x4000;
    if (i <= 0x4000) {
#else
  for (int i = mRsfRem; i > 0; i -= kRSFBufferSize) {
#if RSTL_PRIME1
    uint bufferSize = kRSFBufferSize;
#else
    bufferSize = kRSFBufferSize;
#endif
    if (i <= kRSFBufferSize) {
#endif
      bufferSize = (i + 31) & ~31;
    }

#if RSTL_CORRUPTION
    rstl::auto_ptr< uchar > buf(static_cast< uchar* >(rs_aligned_new_line(bufferSize, 89)));
    mBuffers.push_back_unsafe(buf);
    mDvdRequests.push_back_unsafe(dvdFile.SyncRead(buf.get(), bufferSize));
#else
    rstl::auto_ptr< uchar > buf(
        static_cast< uchar* >(CMemory::Alloc(bufferSize, IAllocator::kHI_RoundUpLen)));
#if RSTL_PRIME1
    mBuffers.push_back(buf);
    mDvdRequests.push_back(dvdFile.SyncRead(buf.get(), bufferSize));
#else
    mBuffers.push_back_unsafe(buf);
    mDvdRequests.push_back_unsafe(dvdFile.SyncRead(buf.get(), bufferSize));
#endif
#endif
  }
}

CRSFAudio::~CRSFAudio() { Stop(); }

const bool CRSFAudio::IsFullyLoaded() const {
  return !mDvdRequests.empty() ? mDvdRequests.back()->IsComplete() : true;
}

#if RSTL_CORRUPTION
void CRSFAudio::Play() {
  RS_VERIFY_THROW(156, IsFullyLoaded(), false,
                  "Trying to play RSF Audio before it's fully loaded!");
  RS_VERIFY_THROW(157, !sPlayingAudio || sPlayingAudio == this, false,
                  "Trying to play two RSF Audios in a row!");
#else
void CRSFAudio::Play() {
#if IS_TARGET_PC
  const metaforce::AudioLockGuard lock;
#endif
#endif
  if (sPlayingAudio == this) {
    return;
  }

#if RSTL_CORRUPTION
  mDvdRequests = rstl::vector< rstl::auto_ptr< CDvdRequest > >();
#else
#if RSTL_PRIME1
  mDvdRequests.clear();
#else
  mDvdRequests = rstl::vector< rstl::auto_ptr< CDvdRequest > >();
#endif
#endif
  mCurSamp = 0;
  g72x_init_state(&mLeftState);
  g72x_init_state(&mRightState);
#if !RSTL_CORRUPTION
#if IS_TARGET_PC
  mAudioPhase = 0;
  mAudioPairRead = 2;
  mAudioPrimed = false;
#endif
#endif
  sPlayingAudio = this;
  CAIInterruptManager::RunDMACallback(MixCallback);
}

#if RSTL_CORRUPTION
void CRSFAudio::Stop() {
#else
void CRSFAudio::Stop() {
#if IS_TARGET_PC
  const metaforce::AudioLockGuard lock;
#endif
#endif
  if (sPlayingAudio == this) {
    CAIInterruptManager::CancelDMACallback(MixCallback);
    sPlayingAudio = nullptr;
  }
}

#if RSTL_CORRUPTION
void CRSFAudio::MixCallback() { sPlayingAudio->DoMix(); }
#else
void CRSFAudio::MixCallback(IF_TARGET_PC(short* output, size_t frames, u32 rate, u32 channels)) {
  sPlayingAudio->DoMix(IF_TARGET_PC(output, frames, rate, channels));
}
#endif

#if RSTL_CORRUPTION
void CRSFAudio::DoMix() {
  const ushort* aiStart = static_cast< const ushort* >(OSPhysicalToCached(AIGetDMAStartAddr()));
  mCurBuf ^= 1;
  uintptr_t buf =
      reinterpret_cast< uintptr_t >(mCurBuf != 0 ? mDmaBufferB.get() : mDmaBufferA.get());

  AIInitDMA(buf, 0x280);
  u32 cookie = OSEnableInterrupts();
  if (aiStart != 0) {
    DCInvalidateRange(const_cast< ushort* >(aiStart), 0x280);
  }

  Decode(reinterpret_cast< ushort* >(buf), aiStart, 160);
  DCFlushRange(reinterpret_cast< void* >(buf), 0x280);
  OSRestoreInterrupts(cookie);
#else
void CRSFAudio::DoMix(IF_TARGET_PC(short* output, size_t frames, u32 rate, u32 channels)) {
#if IS_TARGET_PC
  // RSF stores two 32 kHz G.721 channels, with two samples packed in each byte.
  // Keep decoding pairs even when the output rate requires fractional frames.
  const auto readFrame = [this](short* frame) {
    if (mAudioPairRead == 2) {
      std::fill_n(mAudioPair, 4, short(0));
      ushort* pair = reinterpret_cast< ushort* >(mAudioPair);
      Decode(pair, pair, 2);
      mAudioPairRead = 0;
    }
    frame[0] = mAudioPair[mAudioPairRead * 2];
    frame[1] = mAudioPair[mAudioPairRead * 2 + 1];
    ++mAudioPairRead;
  };
  if (!mAudioPrimed) {
    readFrame(mAudioHistory[0]);
    readFrame(mAudioHistory[1]);
    mAudioPrimed = true;
  }

  for (size_t frame = 0; frame < frames; ++frame, output += channels) {
    for (u32 channel = 0; channel < 2; ++channel) {
      // The original DMA buffer is R,L; native output uses FL,FR.
      const s32 a = mAudioHistory[0][1 - channel];
      const s32 b = mAudioHistory[1][1 - channel];
      const s32 sample = a + (s64(b - a) * mAudioPhase) / rate;
      if (mVolume != 0) {
        output[channel] = std::clamp(s32(output[channel]) + sample, -32768, 32767);
      }
    }
    mAudioPhase += 32000;
    while (mAudioPhase >= rate) {
      mAudioPhase -= rate;
      mAudioHistory[0][0] = mAudioHistory[1][0];
      mAudioHistory[0][1] = mAudioHistory[1][1];
      readFrame(mAudioHistory[1]);
    }
  }
#else
  const ushort* aiStart = static_cast< const ushort* >(OSPhysicalToCached(AIGetDMAStartAddr()));
  mCurBuf ^= 1;
  uintptr_t buf =
      reinterpret_cast< uintptr_t >(mCurBuf != 0 ? mDmaBufferB.get() : mDmaBufferA.get());

  AIInitDMA(buf, 0x280);
  u32 cookie = OSEnableInterrupts();
  if (aiStart != 0) {
    DCInvalidateRange(const_cast< ushort* >(aiStart), 0x280);
  }

  Decode(reinterpret_cast< ushort* >(buf), aiStart, 160);
  DCFlushRange(reinterpret_cast< void* >(buf), 0x280);
  OSRestoreInterrupts(cookie);
#endif
#endif
}

#if RSTL_CORRUPTION
static void MixToMono(ushort* data, int numSamples); // Guessed name
#else
#if !RSTL_PRIME1
static void MixToMono(ushort* data, int numSamples); // Guessed name
#elif RSTL_VERSION == RSTL_GM8E_02 || RSTL_VERSION == RSTL_GM8J_00
void MixStereoToMono(short* buf, int numSamples);
#endif
#endif

#if RSTL_CORRUPTION
void CRSFAudio::Decode(ushort* out, const ushort* in, int numSamples) {
#else
void CRSFAudio::Decode(ushort* out, const ushort* in, const int numSamples) {
#if RSTL_RSF_AUDIO_MUTE_PASSTHROUGH
  if (sMuteCount > 0) {
    CBasics::CopyMemory(out, in, 2 * numSamples * sizeof(*out));
    return;
  }
#endif

#endif
  int curSamp = mCurSamp / 2;
  int loopEndSamp = mLoopEndSamp / 2;
  int loopStartSamp = mLoopStartSamp / 2;
  DecodeMonoAndMix(out, in, numSamples, curSamp, loopEndSamp, loopStartSamp, mVolume, mLeftState);

  int halfLen = mRsfLength / 2;
  DecodeMonoAndMix(out + 1, in + 1, numSamples, curSamp + halfLen, loopEndSamp + halfLen,
                   loopStartSamp + halfLen, mVolume, mRightState);

#if RSTL_CORRUPTION
  if (CAudioManager::GetSpeakerModeIndex() == 0) {
    MixToMono(out, numSamples);
  }
#else
#if !RSTL_PRIME1
  if (CAudioSys::GetSurroundMode() == CAudioSys::kSM_Mono) {
    MixToMono(out, numSamples);
  }
#elif RSTL_VERSION == RSTL_GM8E_02 || RSTL_VERSION == RSTL_GM8J_00
  if (CAudioSys::GetSurroundMode() == CAudioSys::kSM_Mono) {
    MixStereoToMono(reinterpret_cast< short* >(out), numSamples);
  }
#endif
#endif

#if RSTL_CORRUPTION
  int remSamples = numSamples;
#else
  long remSamples = numSamples;
#endif
  while (remSamples != 0) {
#if !RSTL_CORRUPTION
    int rs = remSamples;
#endif
    int remTillLoop = mLoopEndSamp - mCurSamp;
#if RSTL_CORRUPTION
    int rs = remSamples;
    int consumed = rstl::min_val(rs, remTillLoop);
    mCurSamp += consumed;
    remSamples -= consumed;
#else
    rs = rstl::min_val(rs, remTillLoop);
    mCurSamp += rs;
    remSamples -= rs;
#endif
    if (mCurSamp == mLoopEndSamp) {
      mCurSamp = mLoopStartSamp;
    }
  }
}

#if RSTL_CORRUPTION
static void MixToMono(ushort* data, int numSamples) {
  short* samples = reinterpret_cast< short* >(data);
  for (int i = 0; i < numSamples * 2; i += 2) {
    int sample = (samples[0] + samples[1]) / 2;
    short clamped;
    if (sample < -32768) {
      clamped = -32768;
    } else if (sample > 32767) {
      clamped = 32767;
    } else {
      clamped = sample;
    }
    samples[0] = clamped;
    samples[1] = clamped;
    samples += 2;
  }
}
#else
#if !RSTL_PRIME1
static void MixToMono(ushort* data, int numSamples) {
  short* samples = reinterpret_cast< short* >(data);
  for (int i = 0; i < numSamples * 2; i += 2) {
    int sample = (samples[0] + samples[1]) / 2;
    short clamped;
    if (sample < -32768) {
      clamped = -32768;
    } else if (sample > 32767) {
      clamped = 32767;
    } else {
      clamped = sample;
    }
    samples[0] = clamped;
    samples[1] = clamped;
    samples += 2;
  }
}
#elif RSTL_VERSION == RSTL_GM8E_02 || RSTL_VERSION == RSTL_GM8J_00
void MixStereoToMono(short* buf, int numSamples) {
  for (int i = 0; i < numSamples * 2; i += 2) {
    int avg = (buf[i] + buf[i + 1]) / 2;
    short sample;
    if (avg < -0x8000) {
      sample = -0x8000;
    } else if (avg > 0x7fff) {
      sample = 0x7fff;
    } else {
      sample = avg;
    }
    buf[i] = sample;
    buf[i + 1] = sample;
  }
}
#endif
#endif

void CRSFAudio::DecodeMonoAndMix(ushort* out, const ushort* in, int numSamples, int startSample,
#if RSTL_CORRUPTION
                                 int sampleEnd, int sampleStart, int vol, g72x_state& state) {
#else
                                 int sampleEnd, const int sampleStart, int vol, g72x_state& state) {
#endif
  int remBytes = numSamples / 2;
#if RSTL_CORRUPTION
  int curSample = startSample;
#else
  long curSample = startSample;
#endif
  const ushort* inCursor = in;
  ushort* outCursor = out;
  while (remBytes != 0) {
#if RSTL_CORRUPTION
    int rb = remBytes;
    int curBuf = curSample / 0x4000;
    int thisBytes = ((curBuf + 1) * 0x4000) - curSample;
    thisBytes = rstl::min_val(rb, thisBytes);
#else
    int thisBytes = remBytes;
    int curBuf = curSample / kRSFBufferSize;
    int remainingInBuffer = ((curBuf + 1) * kRSFBufferSize) - curSample;
    thisBytes = rstl::min_val(thisBytes, remainingInBuffer);
#endif

    int remTillLoop = sampleEnd - curSample;
    thisBytes = rstl::min_val(thisBytes, remTillLoop);

#if RSTL_CORRUPTION
    uchar* byte = mBuffers[curBuf].get() + (curSample - (curBuf * 0x4000));
#else
    uchar* byte = mBuffers[curBuf].get() + (curSample - (curBuf * kRSFBufferSize));
#endif
    int i = 0;
    short clamped1;
    while (i < thisBytes) {
      int samp1 = reinterpret_cast< const short* >(inCursor)[0] +
                  ((vol * g721_decoder(*byte & 0xf, &state)) >> 15);
      int samp2 = reinterpret_cast< const short* >(inCursor)[2] +
                  ((vol * g721_decoder(*byte >> 4, &state)) >> 15);

      if (samp1 < -0x8000) {
        clamped1 = -0x8000;
      } else if (samp1 > 0x7fff) {
        clamped1 = 0x7fff;
      } else {
        clamped1 = samp1;
      }
      outCursor[0] = clamped1;

      short clamped2;
      if (samp2 < -0x8000) {
        clamped2 = -0x8000;
      } else if (samp2 > 0x7fff) {
        clamped2 = 0x7fff;
      } else {
        clamped2 = samp2;
      }
      outCursor[2] = clamped2;

      outCursor += 4;
      ++byte;
      inCursor += 4;
      ++i;
    }

    curSample += thisBytes;
    remBytes -= thisBytes;
    if (curSample == sampleEnd) {
      curSample = sampleStart;
    }
  }
}

void CRSFAudio::SetVolume(uchar vol) {
#if !RSTL_CORRUPTION
#if IS_TARGET_PC
  const metaforce::AudioLockGuard lock;
#endif
#endif
  if (static_cast< uchar >(vol) > 127) {
    vol = 127;
  }
  mVolume = CAudioSys::kVolumeTable[static_cast< uchar >(vol)];
}
#if !RSTL_CORRUPTION

void CRSFAudio::PushMute() { ++sMuteCount; }

void CRSFAudio::PopMute() { --sMuteCount; }
#endif
