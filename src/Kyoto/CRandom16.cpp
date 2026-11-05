#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Alloc/Assert.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/CCast.hpp"

CRandom16* CRandom16::gRandomNumber = nullptr;
CGlobalRandom* CGlobalRandom::gCurrentGlobalRandom = nullptr;

CGlobalRandom::CGlobalRandom(CRandom16& rnd) : mRandom(rnd) {
  mIsFirst = true;
  mPrev = gCurrentGlobalRandom;
  if (mPrev != nullptr) {
    mPrev->mIsFirst = false;
  }

  gCurrentGlobalRandom = this;
  CRandom16::_SetRandomNumber(&mRandom);
}

CGlobalRandom::~CGlobalRandom() {
  gCurrentGlobalRandom = mPrev;
  if (gCurrentGlobalRandom != nullptr) {
    gCurrentGlobalRandom->mIsFirst = true;
    CRandom16::_SetRandomNumber(&gCurrentGlobalRandom->mRandom);
  } else {
    CRandom16::_SetRandomNumber(nullptr);
  }
}

CRandom16* CRandom16::GetRandomNumber() {
#if RSTL_KYOTO_RANDOM16_PROTOTYPE
  RS_VERIFY_THROW(60, gRandomNumber != NULL, false, "CRandom16::gRandomNumber was set to NULL");
#endif
  return gRandomNumber;
}

void CRandom16::_SetRandomNumber(CRandom16* rnd) { gRandomNumber = rnd; }

CRandom16::CRandom16(const uint seed) : mSeed(seed) {}

void CRandom16::SetSeed(const uint seed) { mSeed = seed; }

int CRandom16::Range(const int min, const int max) { return min + (Next() % ((max - min) + 1)); }

float CRandom16::Range(const float min, const float max) { return ((max - min) * Float()) + min; }

int CRandom16::Next() {
  mSeed = (mSeed * 0x41c64e6d) + 0x00003039;
  return (mSeed >> 16) & 0xffff;
}

#if RSTL_KYOTO_CONST_SCALAR_RETURNS
const float CRandom16::Float() {
  const float next = CCast::ToReal32(Next());
  return 1.52590222E-5f * next;
}
#else
float CRandom16::Float() {
#if RSTL_KYOTO_RANDOM16_PROTOTYPE
  mSeed = (mSeed * 0x41c64e6d) + 0x00003039;
  return 3.7252903e-9f * (mSeed & 0x0fffffff);
#else
  const int next = Next();
  return 1.52590222E-5f * next;
#endif
}
#endif
