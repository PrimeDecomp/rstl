#ifndef _CTIMEREMAINDERANDFRACTION
#define _CTIMEREMAINDERANDFRACTION

#include "Kyoto/Animation/CCharAnimTime.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "rstl/RstlVersions.h"
#include "rstl/math.hpp"

class CIntegerTimeAndRemainder {
public:
#if RSTL_VERSION >= RSTL_GM8P_00
  CIntegerTimeAndRemainder(const CCharAnimTime& time, const CCharAnimTime& interval);
#else
  CIntegerTimeAndRemainder(const CCharAnimTime& time, const CCharAnimTime& interval)
  : mRealTime(time.GetSeconds())
  , mIntegerTime(CCast::ToUint32(time / interval))
  , mRemainder(rstl::max_val(mRealTime - mIntegerTime * interval.GetSeconds(), 0.f)) {}
#endif

  const float& RealTime() const { return mRealTime; }

  const uint& IntegerTime() const { return mIntegerTime; }

  const float& Remainder() const { return mRemainder; }

private:
  float mRealTime;
  uint mIntegerTime;
  float mRemainder;
};
DECLARE_FULL_SIZE_FOR(CIntegerTimeAndRemainder, 0xc)
CHECK_SIZEOF(CIntegerTimeAndRemainder, CIntegerTimeAndRemainder_FULL_SIZE)

class CTimeRemainderAndFraction : public CIntegerTimeAndRemainder {
public:
#if RSTL_VERSION >= RSTL_GM8P_00
  CTimeRemainderAndFraction(const CCharAnimTime& time, const CCharAnimTime& interval);
#else
  CTimeRemainderAndFraction(const CCharAnimTime& time, const CCharAnimTime& interval)
  : CIntegerTimeAndRemainder(time, interval)
  , mFraction(Remainder() / interval.GetSeconds())
  , mFinestSample(interval.GetSeconds()) {}
#endif

  const float& FinestSample() const { return mFinestSample; }

private:
  float mFraction;
  float mFinestSample;
};
CHECK_CHILD_SIZEOF(CTimeRemainderAndFraction, CIntegerTimeAndRemainder, 0x8)

#endif
