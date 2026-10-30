#include "rstl/RstlVersions.h"
#include "Kyoto/Audio/CAudioSys.hpp"
#include "rstl/algorithm.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Audio/CStaticAudioPlayer.hpp"

#include "Kyoto/CDvdFile.hpp"
#include "Kyoto/CDvdRequest.hpp"

#include "rstl/list.hpp"
#include "rstl/math.hpp"

#include "dolphin/ai.h"
#include "dolphin/os.h"

static CStaticAudioPlayer* sCurrentPlayer = nullptr;

CStaticAudioPlayer::CStaticAudioPlayer(const rstl::string& filepath, const int loopStart,
                                       const int loopEnd)
: mFilepath(filepath)
, mRsfRem(-1)
, mCurSamp(0)
, mLoopStartSamp(loopStart & ~1)
, mLoopEndSamp(loopEnd & ~1)
, mCurBuf(0)
, mDmaLeft((uchar*)CMemory::Alloc(640, IAllocator::kHI_RoundUpLen))
, mDmaRight((uchar*)CMemory::Alloc(640, IAllocator::kHI_RoundUpLen))
, mVolume(32768) {
  CDvdFile dvdFile(filepath.data());
  mRsfRem = dvdFile.GetFileSize();
  mRsfLength = mRsfRem;
  int bufferCount = ((mRsfRem - 1) + 0x20000) / 0x20000;
  mBuffers.reserve(bufferCount);
  mDvdRequests.reserve(bufferCount);

  for (int i = mRsfRem; i > 0; i -= 0x20000) {
    uint uVar1 = 0x20000;
    if (i <= 0x20000) {
      uVar1 = (i + 31) & ~31;
    }

    rstl::auto_ptr< uchar > buf((uchar*)CMemory::Alloc(uVar1, IAllocator::kHI_RoundUpLen));
    mBuffers.push_back(buf);
    mDvdRequests.push_back(dvdFile.SyncRead(buf.get(), uVar1));
  }
}

CStaticAudioPlayer::~CStaticAudioPlayer() { StopMixOut(); }

const bool CStaticAudioPlayer::IsReady() const {
  return !mDvdRequests.empty() ? mDvdRequests.back()->IsComplete() : true;
}

void CStaticAudioPlayer::StartMixOut() {
  if (sCurrentPlayer == this) {
    return;
  }

  mDvdRequests.clear();
  mCurSamp = 0;
  g72x_init_state(&mLeftState);
  g72x_init_state(&mRightState);
  sCurrentPlayer = this;
  RunDMACallback(MixCallback);
}

void CStaticAudioPlayer::StopMixOut() {
  if (sCurrentPlayer == this) {
    CancelDMACallback(MixCallback);
    sCurrentPlayer = NULL;
  }
}

void CStaticAudioPlayer::MixCallback() { sCurrentPlayer->DoMix(); }

void CStaticAudioPlayer::DoMix() {
  u32 aiStart = OSCachedToPhysical(AIGetDMAStartAddr());
  mCurBuf ^= 1;
  uintptr_t buf =
      reinterpret_cast< uintptr_t >(mCurBuf != 0 ? mDmaRight.get() : mDmaLeft.get());

  AIInitDMA(buf, 0x280);
  u32 cookie = OSEnableInterrupts();
  if (aiStart != 0) {
    DCInvalidateRange((void*)aiStart, 0x280);
  }

  Decode((ushort*)buf, (ushort*)aiStart, 160);
  DCFlushRange((void*)buf, 0x280);
  OSRestoreInterrupts(cookie);
}

#if RSTL_VERSION == RSTL_GM8E_02 || RSTL_VERSION == RSTL_GM8J_00
void MixStereoToMono(short* buf, int numSamples);
#endif

void CStaticAudioPlayer::Decode(const ushort* bufIn, ushort* bufOut, const int numSamples) {
  int curSamp = mCurSamp / 2;
  int loopEndSamp = mLoopEndSamp / 2;
  int loopStartSamp = mLoopStartSamp / 2;
  DecodeMonoAndMix(const_cast< ushort* >(bufIn), bufOut, numSamples, curSamp, loopEndSamp,
                   loopStartSamp, mVolume, mLeftState);

  int halfLen = mRsfLength / 2;
  DecodeMonoAndMix(const_cast< ushort* >(bufIn + 1), bufOut + 1, numSamples, curSamp + halfLen,
                   loopEndSamp + halfLen, loopStartSamp + halfLen, mVolume, mRightState);
#if RSTL_VERSION == RSTL_GM8E_02 || RSTL_VERSION == RSTL_GM8J_00
  if (CAudioSys::GetSurroundMode() == CAudioSys::kSM_Mono) {
    MixStereoToMono(reinterpret_cast< short* >(const_cast< ushort* >(bufIn)), numSamples);
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

#if RSTL_VERSION == RSTL_GM8E_02 || RSTL_VERSION == RSTL_GM8J_00
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

void CStaticAudioPlayer::DecodeMonoAndMix(ushort* bufInArg, ushort* bufOutArg, int numSamples,
                                          int curSampleArg, int sampleEnd, int sampleStart, int vol,
                                          g72x_state& state) {
  int remBytes = numSamples / 2;
  long curSample = curSampleArg;
  ushort* bufOut = bufOutArg;
  ushort* bufIn = bufInArg;
  while (remBytes != 0) {
    int thisBytes = remBytes;
    int curBuf = curSample / 0x20000;
    int remainingInBuffer = ((curBuf + 1) * 0x20000) - curSample;
    thisBytes = rstl::min_val(thisBytes, remainingInBuffer);

    int remTillLoop = sampleEnd - curSample;
    thisBytes = rstl::min_val(thisBytes, remTillLoop);

    uchar* byte = mBuffers[curBuf].get() + (curSample - (curBuf * 0x20000));
    short clamped1;
    for (int i = 0; i < thisBytes; ++i) {
      int samp1 =
          reinterpret_cast< short* >(bufOut)[0] + ((vol * g721_decoder(*byte & 0xf, &state)) >> 15);
      int samp2 =
          reinterpret_cast< short* >(bufOut)[2] + ((vol * g721_decoder(*byte >> 4, &state)) >> 15);

      if (samp1 < -0x8000) {
        clamped1 = -0x8000;
      } else if (samp1 > 0x7fff) {
        clamped1 = 0x7fff;
      } else {
        clamped1 = samp1;
      }
      bufIn[0] = clamped1;

      short clamped2;
      if (samp2 < -0x8000) {
        clamped2 = -0x8000;
      } else if (samp2 > 0x7fff) {
        clamped2 = 0x7fff;
      } else {
        clamped2 = samp2;
      }
      bufIn[2] = clamped2;

      bufIn += 4;
      ++byte;
      bufOut += 4;
    }

    curSample += thisBytes;
    remBytes -= thisBytes;
    if (curSample == sampleEnd) {
      curSample = sampleStart;
    }
  }
}

void CStaticAudioPlayer::SetVolume(uchar vol) {
  if (static_cast< uchar >(vol) > 127) {
    vol = 127;
  }
  mVolume = CAudioSys::kVolumeTable[static_cast< uchar >(vol)];
}

