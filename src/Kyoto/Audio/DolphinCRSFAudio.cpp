#include "Kyoto/Audio/DolphinCRSFAudio.hpp"
#include "rstl/RstlVersions.h"
#if RSTL_PRIME1
typedef CRSFAudio CAICallbackOwner;
#else
#include "Kyoto/Audio/CAIInterruptManager.hpp"
typedef CAIInterruptManager CAICallbackOwner;
#endif

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Audio/CAudioSys.hpp"

#include "Kyoto/CDvdFile.hpp"
#include "Kyoto/CDvdRequest.hpp"

#include "rstl/math.hpp"

#include "dolphin/ai.h"
#include "dolphin/os.h"
#include "stdint.h"

#if RSTL_PRIME1
enum { kRSFBufferSize = 0x20000 };
#else
enum { kRSFBufferSize = 0x4000 };
#endif

static CRSFAudio* sPlayingAudio = nullptr;

CRSFAudio::CRSFAudio(const rstl::string& audioFile, const int loopStart, const int loopEnd)
: mFilepath(audioFile)
, mRsfRem(-1)
, mCurSamp(0)
, mLoopStartSamp(loopStart & ~1)
, mLoopEndSamp(loopEnd & ~1)
, mCurBuf(0)
, mDmaBufferA(static_cast< uchar* >(CMemory::Alloc(640, IAllocator::kHI_RoundUpLen)))
, mDmaBufferB(static_cast< uchar* >(CMemory::Alloc(640, IAllocator::kHI_RoundUpLen)))
, mVolume(32768) {
  CDvdFile dvdFile(audioFile.data());
  mRsfRem = dvdFile.GetFileSize();
  mRsfLength = mRsfRem;
#if !RSTL_PRIME1
  uint bufferSize;
#endif
  int bufferCount = ((mRsfRem - 1) + kRSFBufferSize) / kRSFBufferSize;
  mBuffers.reserve(bufferCount);
  mDvdRequests.reserve(bufferCount);

  for (int i = mRsfRem; i > 0; i -= kRSFBufferSize) {
#if RSTL_PRIME1
    uint bufferSize = kRSFBufferSize;
#else
    bufferSize = kRSFBufferSize;
#endif
    if (i <= kRSFBufferSize) {
      bufferSize = (i + 31) & ~31;
    }

    rstl::auto_ptr< uchar > buf(
        static_cast< uchar* >(CMemory::Alloc(bufferSize, IAllocator::kHI_RoundUpLen)));
#if RSTL_PRIME1
    mBuffers.push_back(buf);
    mDvdRequests.push_back(dvdFile.SyncRead(buf.get(), bufferSize));
#else
    mBuffers.push_back_unsafe(buf);
    mDvdRequests.push_back_unsafe(dvdFile.SyncRead(buf.get(), bufferSize));
#endif
  }
}

CRSFAudio::~CRSFAudio() { StopMixOut(); }

const bool CRSFAudio::IsFullyLoaded() const {
  return !mDvdRequests.empty() ? mDvdRequests.back()->IsComplete() : true;
}

void CRSFAudio::StartMixOut() {
  if (sPlayingAudio == this) {
    return;
  }

#if RSTL_PRIME1
  mDvdRequests.clear();
#else
  mDvdRequests = rstl::vector< rstl::auto_ptr< CDvdRequest > >();
#endif
  mCurSamp = 0;
  g72x_init_state(&mLeftState);
  g72x_init_state(&mRightState);
  sPlayingAudio = this;
  CAICallbackOwner::RunDMACallback(MixCallback);
}

void CRSFAudio::StopMixOut() {
  if (sPlayingAudio == this) {
    CAICallbackOwner::CancelDMACallback(MixCallback);
    sPlayingAudio = nullptr;
  }
}

void CRSFAudio::MixCallback() { sPlayingAudio->DoMix(); }

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

  Decode(reinterpret_cast< ushort* >(buf), const_cast< ushort* >(aiStart), 160);
  DCFlushRange(reinterpret_cast< void* >(buf), 0x280);
  OSRestoreInterrupts(cookie);
}

#if !RSTL_PRIME1
static void MixToMono(ushort* data, int numSamples); // Guessed name
#elif RSTL_VERSION == RSTL_GM8E_02 || RSTL_VERSION == RSTL_GM8J_00
void MixStereoToMono(short* buf, int numSamples);
#endif

#if RSTL_PRIME1
void CRSFAudio::Decode(const ushort* out, ushort* in, const int numSamples) {
#else
void CRSFAudio::Decode(ushort* out, const ushort* in, int numSamples) {
#endif
  int curSamp = mCurSamp / 2;
  int loopEndSamp = mLoopEndSamp / 2;
  int loopStartSamp = mLoopStartSamp / 2;
  DecodeMonoAndMix(const_cast< ushort* >(out), in, numSamples, curSamp, loopEndSamp, loopStartSamp,
                   mVolume, mLeftState);

  int halfLen = mRsfLength / 2;
  DecodeMonoAndMix(const_cast< ushort* >(out + 1), in + 1, numSamples, curSamp + halfLen,
                   loopEndSamp + halfLen, loopStartSamp + halfLen, mVolume, mRightState);

#if !RSTL_PRIME1
  if (CAudioSys::GetSurroundMode() == CAudioSys::kSM_Mono) {
    MixToMono(out, numSamples);
  }
#elif RSTL_VERSION == RSTL_GM8E_02 || RSTL_VERSION == RSTL_GM8J_00
  if (CAudioSys::GetSurroundMode() == CAudioSys::kSM_Mono) {
    MixStereoToMono(reinterpret_cast< short* >(const_cast< ushort* >(out)), numSamples);
  }
#endif

  long remSamples = numSamples;
  while (remSamples != 0) {
    int rs = remSamples;
    int remTillLoop = mLoopEndSamp - mCurSamp;
    rs = rstl::min_val(rs, remTillLoop);
    mCurSamp += rs;
    remSamples -= rs;
    if (mCurSamp == mLoopEndSamp) {
      mCurSamp = mLoopStartSamp;
    }
  }
}

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

#if RSTL_PRIME1
void CRSFAudio::DecodeMonoAndMix(ushort* out, ushort* in, int numSamples, int startSample,
                                 int sampleEnd, const int sampleStart, int vol, g72x_state& state) {
#else
void CRSFAudio::DecodeMonoAndMix(ushort* out, const ushort* in, int numSamples, int startSample,
                                 int sampleEnd, const int sampleStart, int vol, g72x_state& state) {
#endif
  int remBytes = numSamples / 2;
  long curSample = startSample;
  const ushort* inCursor = in;
  ushort* outCursor = out;
  while (remBytes != 0) {
    int thisBytes = remBytes;
    int curBuf = curSample / kRSFBufferSize;
    int remainingInBuffer = ((curBuf + 1) * kRSFBufferSize) - curSample;
    thisBytes = rstl::min_val(thisBytes, remainingInBuffer);

    int remTillLoop = sampleEnd - curSample;
    thisBytes = rstl::min_val(thisBytes, remTillLoop);

    uchar* byte = mBuffers[curBuf].get() + (curSample - (curBuf * kRSFBufferSize));
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
  if (static_cast< uchar >(vol) > 127) {
    vol = 127;
  }
  mVolume = CAudioSys::kVolumeTable[static_cast< uchar >(vol)];
}
