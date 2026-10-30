#include "rstl/RstlVersions.h"
#include "Kyoto/Particles/CIntElement.hpp"

#include "Kyoto/Basics/CCast.hpp"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CParticleDataFactory.hpp"
#include "Kyoto/Particles/CParticleElectric.hpp"

#include "Kyoto/Particles/CParticleGlobals.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

#include "rstl/math.hpp"

#if !RSTL_PRIME1
// Unreferenced; every nearby particle TU constructs one of these, likely from a shared header.
static const CVector3f skOneVector(1.f, 1.f, 1.f);

// Guessed name
static inline int GetKeyframeIndex(int frame, bool loop, int loopStart, int loopEnd) {
  if (loop) {
    if (frame >= loopEnd) {
      frame -= loopStart;
      frame = frame % (loopEnd - loopStart);
      frame += loopStart;
    }
  } else {
    frame = rstl::min_val(frame, loopEnd - 1);
  }
  return frame;
}

// Guessed name
static inline int GetKeyframeTime(float value, float start, float rate) {
  return rstl::max_val(0, CCast::ToInt32(rate * (value - start)));
}

CIEParticleCreationTime::~CIEParticleCreationTime() {}
#endif

CIEConstant::CIEConstant(int val) : mVal(val) {}

CIEConstant::~CIEConstant() {}

bool CIEConstant::GetValue(int frame, int& valOut) const {
  valOut = mVal;
  return false;
}

CIEDeath::CIEDeath(CIntElement* a, CIntElement* b) : mA(a), mB(b) {}

CIEDeath::~CIEDeath() {
  delete mA;
  delete mB;
}

bool CIEDeath::GetValue(int frame, int& valOut) const {
  int b;
  mA->GetValue(frame, valOut);
  mB->GetValue(frame, b);
  return frame >= b ? TRUE : FALSE;
}

CIEAdd::CIEAdd(CIntElement* a, CIntElement* b) : mA(a), mB(b) {}

CIEAdd::~CIEAdd() {
  delete mA;
  delete mB;
}

bool CIEAdd::GetValue(int frame, int& valOut) const {
  int a, b;
  mA->GetValue(frame, a);
  mB->GetValue(frame, b);
  valOut = a + b;
  return false;
}

CIEMultiply::CIEMultiply(CIntElement* a, CIntElement* b) : mA(a), mB(b) {}

CIEMultiply::~CIEMultiply() {
  delete mA;
  delete mB;
}

bool CIEMultiply::GetValue(int frame, int& valOut) const {
  int a, b;
  mA->GetValue(frame, a);
  mB->GetValue(frame, b);
  valOut = a * b;
  return false;
}

#if !RSTL_PRIME1
CIEDivide::CIEDivide(CIntElement* a, CIntElement* b) : mDividend(a), mDivisor(b) {}

CIEDivide::~CIEDivide() {
  delete mDividend;
  delete mDivisor;
}

bool CIEDivide::GetValue(int frame, int& valOut) const {
  int a, b;
  mDividend->GetValue(frame, a);
  mDivisor->GetValue(frame, b);
  if (b != 0) {
    valOut = a / b;
  } else {
    valOut = a;
  }
  return false;
}
#endif

CIEModulo::CIEModulo(CIntElement* a, CIntElement* b) : mA(a), mB(b) {}

CIEModulo::~CIEModulo() {
  delete mA;
  delete mB;
}

bool CIEModulo::GetValue(int frame, int& valOut) const {
  int a, b;
  mA->GetValue(frame, a);
  mB->GetValue(frame, b);
  if (b != 0) {
    valOut = a % b;
  } else {
    valOut = a;
  }
  return false;
}

CIERandom::CIERandom(CIntElement* a, CIntElement* b) : mA(a), mB(b) {}

CIERandom::~CIERandom() {
  delete mA;
  delete mB;
}

bool CIERandom::GetValue(int frame, int& valOut) const {
  int a, b;
  mA->GetValue(frame, a);
  mB->GetValue(frame, b);
  if (a > 0) {
    valOut = CRandom16::GetRandomNumber()->Range(a, b);
  } else {
    valOut = CRandom16::GetRandomNumber()->Next();
  }
  return false;
}

CIESampleAndHold::CIESampleAndHold(CIntElement* a, CIntElement* b, CIntElement* c)
: mSampleSource(a), mNextSampleFrame(0), mWaitFramesMin(b), mWaitFramesMax(c) {}

CIESampleAndHold::~CIESampleAndHold() {
  delete mSampleSource;
  delete mWaitFramesMin;
  delete mWaitFramesMax;
}

bool CIESampleAndHold::GetValue(int frame, int& valOut) const {
  bool ret;
  if (mNextSampleFrame < frame) {
    int b, c;
    mWaitFramesMin->GetValue(frame, b);
    mWaitFramesMax->GetValue(frame, c);
    mNextSampleFrame = CRandom16::GetRandomNumber()->Range(b, c) + frame;
    ret = mSampleSource->GetValue(frame, valOut);
    mHoldVal = valOut;
  } else {
    valOut = mHoldVal;
    ret = false;
  }
  return ret;
}

CIEImpulse::CIEImpulse(CIntElement* a) : mA(a) {}

CIEImpulse::~CIEImpulse() { delete mA; }

bool CIEImpulse::GetValue(int frame, int& valOut) const {
  if (frame == 0) {
    mA->GetValue(frame, valOut);
  } else {
    valOut = 0;
  }
  return false;
}

CIETimescale::CIETimescale(CRealElement* a) : mA(a) {}

CIETimescale::~CIETimescale() { delete mA; }

bool CIETimescale::GetValue(int frame, int& valOut) const {
  float a;
  mA->GetValue(frame, a);
  valOut = static_cast< float >(frame) * a;
  return false;
}

CIEInitialRandom::CIEInitialRandom(CIntElement* a, CIntElement* b) : mA(a), mB(b) {}

CIEInitialRandom::~CIEInitialRandom() {
  delete mA;
  delete mB;
}

bool CIEInitialRandom::GetValue(int frame, int& valOut) const {
  if (frame == 0) {
    int a, b;
    mA->GetValue(frame, a);
    mB->GetValue(frame, b);
    valOut = CRandom16::GetRandomNumber()->Range(a, b);
  }
  return false;
}

CIETimeChain::CIETimeChain(CIntElement* a, CIntElement* b, CIntElement* c)
: mA(a), mB(b), mSwFrame(c) {}

CIETimeChain::~CIETimeChain() {
  delete mA;
  delete mB;
  delete mSwFrame;
}

bool CIETimeChain::GetValue(int frame, int& valOut) const {
  int v;
  mSwFrame->GetValue(frame, v);
  if (frame < v) {
    return mA->GetValue(frame, valOut);
  } else {
    return mB->GetValue(frame - v, valOut);
  }
}

CIEClamp::CIEClamp(CIntElement* a, CIntElement* b, CIntElement* c) : mMin(a), mMax(b), mVal(c) {}

CIEClamp::~CIEClamp() {
  delete mMin;
  delete mMax;
  delete mVal;
}

bool CIEClamp::GetValue(int frame, int& valOut) const {
  int a, b;
  mMin->GetValue(frame, a);
  mMax->GetValue(frame, b);
  mVal->GetValue(frame, valOut);
  if (valOut > b) {
    valOut = b;
  }
  if (valOut < a) {
    valOut = a;
  }
  return false;
}

CIEPulse::CIEPulse(CIntElement* a, CIntElement* b, CIntElement* c, CIntElement* d)
: mADuration(a), mBDuration(b), mAVal(c), mBVal(d) {}

CIEPulse::~CIEPulse() {
  delete mADuration;
  delete mBDuration;
  delete mAVal;
  delete mBVal;
}

bool CIEPulse::GetValue(int frame, int& valOut) const {
  int a, b;
  mADuration->GetValue(frame, a);
  mBDuration->GetValue(frame, b);
  int cv = a + b + 1;
  if (cv < 0) {
    cv = 1;
  }

  if (b >= 1) {
    if (frame % cv > a) {
      mBVal->GetValue(frame, valOut);
    } else {
      mAVal->GetValue(frame, valOut);
    }
  } else {
    mAVal->GetValue(frame, valOut);
  }
  return false;
}

CIELifetimePercent::CIELifetimePercent(CIntElement* a) : mPercentVal(a) {}

CIELifetimePercent::~CIELifetimePercent() { delete mPercentVal; }

bool CIELifetimePercent::GetValue(int frame, int& valOut) const {
  int a = 0;
  mPercentVal->GetValue(frame, a);
  if (a < 0) {
    a = 0;
  }
  valOut = (a / 100.0f) * CParticleGlobals::GetParticleLifetimeReal() + 0.5f;
  return false;
}

CIEKeyframeEmitter::CIEKeyframeEmitter(CInputStream& in)
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
: mPercent(in.ReadLong())
, mUnk1(in.ReadLong())
#else
: mPercent(in.ReadInt32())
, mUnk1(in.ReadInt32())
#endif

, mLoop(in.ReadBool())
, mUnk2(in.ReadBool())
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
, mLoopEnd(in.ReadLong())
, mLoopStart(in.ReadLong())
#else
, mLoopEnd(in.ReadInt32())
, mLoopStart(in.ReadInt32())
#endif

, mKeys(in) {}

CIEKeyframeEmitter::~CIEKeyframeEmitter() {}

bool CIEKeyframeEmitter::GetValue(int frame, int& valOut) const {
  if (mPercent == 0) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    int emitterTime = CParticleGlobals::GetEmitterTime();
    if (mLoop) {
      if (emitterTime >= mLoopEnd) {
        emitterTime -= mLoopStart;
        emitterTime = emitterTime % (mLoopEnd - mLoopStart);
        emitterTime += mLoopStart;
      }
      valOut = mKeys[emitterTime];
    } else {
      emitterTime = rstl::min_val(emitterTime, mLoopEnd - 1);
      valOut = mKeys[emitterTime];
    }
#else
    int emitterTime =
        GetKeyframeIndex(CParticleGlobals::GetEmitterTime(), mLoop, mLoopStart, mLoopEnd);
    valOut = mKeys[emitterTime];
#endif
    return false;
  } else {
    int ltPerc = CParticleGlobals::GetParticleLifetimePercentage();
    if (ltPerc == 100) {
      valOut = mKeys[ltPerc];
    } else {
      float ltPercRem = CParticleGlobals::GetParticleLifetimePercentageRemainder();
      float lerp = (1.0f - ltPercRem) * mKeys[ltPerc] + ltPercRem * mKeys[ltPerc + 1];
      valOut = CCast::ToInt32(lerp);
    }
    return false;
  }
}

#if !RSTL_PRIME1
CIEKeyframeInput::CIEKeyframeInput(CInputStream& in)
: mPercent(in.ReadInt32())
, mUnk1(in.ReadInt32())
, mLoop(in.ReadBool())
, mUnk2(in.ReadBool())
, mLoopEnd(in.ReadInt32())
, mLoopStart(in.ReadInt32())
, mInputStart(in.ReadFloat())
, mInputRate(in.ReadFloat())
, mKeys(in)
, mInput(CParticleDataFactory::GetRealElement(in)) {}

CIEKeyframeInput::~CIEKeyframeInput() { delete mInput; }

bool CIEKeyframeInput::GetValue(int frame, int& valOut) const {
  if (mPercent == 2) {
    float in = 0.0f;
    mInput->GetValue(frame, in);
    int idx = GetKeyframeIndex(GetKeyframeTime(in, mInputStart, mInputRate), mLoop, GetLoopStart(),
                               GetLoopEnd());
    bool lerp = idx > 0 && idx < mLoopEnd - 1;
    if (lerp) {
      float t =
          CMath::Clamp(0.0f, (in - mInputStart) - static_cast< float >(idx) / mInputRate, 1.0f);
      valOut = CCast::ToInt32((1.0f - t) * mKeys[idx] + t * mKeys[idx + 1]);
    } else {
      valOut = mKeys[idx];
    }
  }
  return false;
}
#endif

CIESubtract::CIESubtract(CIntElement* a, CIntElement* b) : mA(a), mB(b) {}

CIESubtract::~CIESubtract() {
  delete mA;
  delete mB;
}

bool CIESubtract::GetValue(int frame, int& valOut) const {
  int a, b;
  mA->GetValue(frame, a);
  mB->GetValue(frame, b);
  valOut = a - b;
  return false;
}

CIERealToInt::CIERealToInt(CRealElement* a, CRealElement* b) : mA(a), mB(b) {}

CIERealToInt::~CIERealToInt() {
  delete mA;
  delete mB;
}

bool CIERealToInt::GetValue(int frame, int& valOut) const {
  float a = 0.0f;
  float b = 1.0f;
  mB->GetValue(frame, b);
  mA->GetValue(frame, a);
  valOut = CCast::ToInt32(a * b);
  return false;
}

bool CIEGetCumulativeParticleCount::GetValue(int frame, int& valOut) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  valOut = CParticleGlobals::GetCurrentParticleSystem()->mSystem->GetCumulativeParticleCount();
#else
  CParticleGlobals::SParticleSystem* system = CParticleGlobals::GetCurrentParticleSystem();
  FourCC type = system == nullptr ? 'NULL' : system->mType;
  switch (type) {
  case 'PART':
    valOut = static_cast< CElementGen* >(system->mSystem)->GetCumulativeParticleCount();
    break;
  case 'ELSC':
    valOut = static_cast< CParticleElectric* >(system->mSystem)->GetCumulativeParticleCount();
    break;
  default:
    valOut = 0;
    break;
  }
#endif

  return false;
}

bool CIEGetActiveParticleCount::GetValue(int frame, int& valOut) const {
  valOut = CParticleGlobals::GetCurrentParticleSystem()->mSystem->GetParticleCount();
  return false;
}

bool CIEGetEmitterTime::GetValue(int frame, int& valOut) const {
  valOut = CParticleGlobals::GetCurrentParticleSystem()->mSystem->GetEmitterTime();
  return false;
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CIEGetEmitterTime::~CIEGetEmitterTime() {}
#else
CIEInitialSwitch::CIEInitialSwitch(CIntElement* a, CIntElement* b)
: mInitialValue(a), mSubsequentValue(b) {}

CIEInitialSwitch::~CIEInitialSwitch() {
  delete mInitialValue;
  delete mSubsequentValue;
}

bool CIEInitialSwitch::GetValue(int frame, int& valOut) const {
  if (frame == 0) {
    mInitialValue->GetValue(0, valOut);
  } else {
    mSubsequentValue->GetValue(frame - 1, valOut);
  }
  return false;
}

CIEKeepInitial::CIEKeepInitial(CIntElement* a) : mValue(a) {}

CIEKeepInitial::~CIEKeepInitial() { delete mValue; }

bool CIEKeepInitial::GetValue(int frame, int& valOut) const {
  if (frame == 0) {
    mValue->GetValue(0, valOut);
  }
  return false;
}

bool CIEParticleCreationTime::GetValue(int frame, int& valOut) const {
  valOut = CParticleGlobals::GetCurrentParticle()->mStartFrame;
  return false;
}
#endif

