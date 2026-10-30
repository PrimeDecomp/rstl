#ifndef _CINTELEMENT
#define _CINTELEMENT
#include "rstl/RstlVersions.h"

#include "types.h"

#include "Kyoto/Particles/IElement.hpp"

#include "rstl/vector.hpp"

class CInputStream;

class CIEConstant : public CIntElement {
  int mVal;

public:
  CIEConstant(int val);
  ~CIEConstant() override;
  bool GetValue(int frame, int& valOut) const override;
};

class CIEDeath : public CIntElement {
  CIntElement* mA;
  CIntElement* mB;

public:
  CIEDeath(CIntElement* a, CIntElement* b);
  ~CIEDeath() override;
  bool GetValue(int frame, int& valOut) const override;
};

class CIEAdd : public CIntElement {
  CIntElement* mA;
  CIntElement* mB;

public:
  CIEAdd(CIntElement* a, CIntElement* b);
  ~CIEAdd() override;
  bool GetValue(int frame, int& valOut) const override;
};

class CIEMultiply : public CIntElement {
  CIntElement* mA;
  CIntElement* mB;

public:
  CIEMultiply(CIntElement* a, CIntElement* b);
  ~CIEMultiply() override;
  bool GetValue(int frame, int& valOut) const override;
};

class CIEModulo : public CIntElement {
  CIntElement* mA;
  CIntElement* mB;

public:
  CIEModulo(CIntElement* a, CIntElement* b);
  ~CIEModulo() override;
  bool GetValue(int frame, int& valOut) const override;
};

class CIERandom : public CIntElement {
  CIntElement* mA;
  CIntElement* mB;

public:
  CIERandom(CIntElement* a, CIntElement* b);
  ~CIERandom() override;
  bool GetValue(int frame, int& valOut) const override;
};

class CIESampleAndHold : public CIntElement {
  CIntElement* mSampleSource;
  mutable int mNextSampleFrame;
  CIntElement* mWaitFramesMin;
  CIntElement* mWaitFramesMax;
  mutable int mHoldVal;

public:
  CIESampleAndHold(CIntElement* a, CIntElement* b, CIntElement* c);
  ~CIESampleAndHold() override;
  bool GetValue(int frame, int& valOut) const override;
};

class CIEImpulse : public CIntElement {
  CIntElement* mA;

public:
  CIEImpulse(CIntElement* a);
  ~CIEImpulse() override;
  bool GetValue(int frame, int& valOut) const override;
};

class CIETimescale : public CIntElement {
  CRealElement* mA;

public:
  CIETimescale(CRealElement* a);
  ~CIETimescale();
  bool GetValue(int frame, int& valOut) const override;
};

class CIEInitialRandom : public CIntElement {
  CIntElement* mA;
  CIntElement* mB;

public:
  CIEInitialRandom(CIntElement* a, CIntElement* b);
  ~CIEInitialRandom() override;
  bool GetValue(int frame, int& valOut) const override;
};

class CIETimeChain : public CIntElement {
  CIntElement* mA;
  CIntElement* mB;
  CIntElement* mSwFrame;

public:
  CIETimeChain(CIntElement* a, CIntElement* b, CIntElement* c);
  ~CIETimeChain() override;
  bool GetValue(int frame, int& valOut) const override;
};

class CIEClamp : public CIntElement {
  CIntElement* mMin;
  CIntElement* mMax;
  CIntElement* mVal;

public:
  CIEClamp(CIntElement* a, CIntElement* b, CIntElement* c);
  ~CIEClamp() override;
  bool GetValue(int frame, int& valOut) const override;
};

class CIEPulse : public CIntElement {
  CIntElement* mADuration;
  CIntElement* mBDuration;
  CIntElement* mAVal;
  CIntElement* mBVal;

public:
  CIEPulse(CIntElement* a, CIntElement* b, CIntElement* c, CIntElement* d);
  ~CIEPulse() override;
  bool GetValue(int frame, int& valOut) const override;
};

class CIELifetimePercent : public CIntElement {
  CIntElement* mPercentVal;

public:
  CIELifetimePercent(CIntElement* a);
  ~CIELifetimePercent() override;
  bool GetValue(int frame, int& valOut) const override;
};

class CIEKeyframeEmitter : public CIntElement {
  int mPercent;
  int mUnk1;
  bool mLoop;
  bool mUnk2;
  int mLoopEnd;
  int mLoopStart;
  rstl::vector< int > mKeys;

public:
  CIEKeyframeEmitter(CInputStream& in);
  ~CIEKeyframeEmitter() override;
  bool GetValue(int frame, int& valOut) const override;
};

class CIESubtract : public CIntElement {
  CIntElement* mA;
  CIntElement* mB;

public:
  CIESubtract(CIntElement* a, CIntElement* b);
  ~CIESubtract() override;
  bool GetValue(int frame, int& valOut) const override;
};

class CIERealToInt : public CIntElement {
  CRealElement* mA;
  CRealElement* mB;

public:
  CIERealToInt(CRealElement* a, CRealElement* b);
  ~CIERealToInt() override;
  bool GetValue(int frame, int& valOut) const override;
};

class CIEGetCumulativeParticleCount : public CIntElement {
public:
  ~CIEGetCumulativeParticleCount() override {}
  bool GetValue(int frame, int& valOut) const override;
};

class CIEGetActiveParticleCount : public CIntElement {
public:
  ~CIEGetActiveParticleCount() override {}
  bool GetValue(int frame, int& valOut) const override;
};

class CIEGetEmitterTime : public CIntElement {
public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  ~CIEGetEmitterTime() override;
#else
  ~CIEGetEmitterTime() override {}
#endif

  bool GetValue(int frame, int& valOut) const override;
};

// Echoes additions. Class and member names below are target-derived hypotheses.

#if !RSTL_PRIME1
// Guessed name
class CIEDivide : public CIntElement {
  CIntElement* mDividend;
  CIntElement* mDivisor;

public:
  CIEDivide(CIntElement* a, CIntElement* b);
  // IElement
  ~CIEDivide() override;
  // CIntElement
  bool GetValue(int frame, int& valOut) const override;
};
CHECK_SIZEOF(CIEDivide, 0xC)

// Guessed name
class CIEInitialSwitch : public CIntElement {
  CIntElement* mInitialValue;
  CIntElement* mSubsequentValue;

public:
  CIEInitialSwitch(CIntElement* a, CIntElement* b);
  // IElement
  ~CIEInitialSwitch() override;
  // CIntElement
  bool GetValue(int frame, int& valOut) const override;
};
CHECK_SIZEOF(CIEInitialSwitch, 0xC)

// Guessed name
class CIEKeyframeInput : public CIntElement {
  int mPercent;
  int mUnk1;
  bool mLoop;
  bool mUnk2;
  int mLoopEnd;
  int mLoopStart;
  float mInputStart;
  float mInputRate;
  rstl::vector< int > mKeys;
  CRealElement* mInput;

public:
  CIEKeyframeInput(CInputStream& in);
  // IElement
  ~CIEKeyframeInput() override;
  // CIntElement
  bool GetValue(int frame, int& valOut) const override;
  int GetLoopStart() const { return mLoopStart; }
  int GetLoopEnd() const { return mLoopEnd; }
};
CHECK_SIZEOF(CIEKeyframeInput, 0x34)

// Constructed for both PCRT and PDET.
// Guessed name
class CIEParticleCreationTime : public CIntElement {
public:
  // CIntElement
  bool GetValue(int frame, int& valOut) const override;
  // IElement
  ~CIEParticleCreationTime() override;
};
CHECK_SIZEOF(CIEParticleCreationTime, 0x4)

// Guessed name
class CIEKeepInitial : public CIntElement {
  CIntElement* mValue;

public:
  CIEKeepInitial(CIntElement* a);
  // IElement
  ~CIEKeepInitial() override;
  // CIntElement
  bool GetValue(int frame, int& valOut) const override;
};
CHECK_SIZEOF(CIEKeepInitial, 0x8)
#endif

#endif // _CINTELEMENT
