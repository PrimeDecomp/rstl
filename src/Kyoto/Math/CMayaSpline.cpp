#include "rstl/RstlVersions.h"
#include "Kyoto/Math/CMayaSpline.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

#include "rstl/algorithm.hpp"
#include "rstl/functional.hpp"

#include "float.h"
#include "math.h"

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_R3IJ

CMayaSplineKnot::CMayaSplineKnot(float time, float amplitude, ETangentType inTangentType,
                               ETangentType outTangentType, const CAbsAngle& inAngle,
                               const CAbsAngle& outAngle)
: mTime(time)
, mAmplitude(amplitude)
, mInTangentType(inTangentType)
, mOutTangentType(outTangentType)
, mDirty(true)
, mCachedTangentA(CVector2f(0.f, 0.f))
, mCachedTangentB(CVector2f(0.f, 0.f)) {
  if (inTangentType == kTT_Fixed) {
    mCachedTangentA = CVector2f(3.f * cosf(inAngle.AsRadians()), 3.f * sinf(inAngle.AsRadians()));
  }
  if (outTangentType == kTT_Fixed) {
    mCachedTangentB = CVector2f(3.f * cosf(outAngle.AsRadians()), 3.f * sinf(outAngle.AsRadians()));
  }
}

void CMayaSplineKnot::GetTangents(const CMayaSplineKnot* previous, const CMayaSplineKnot* next,
                                CVector2f& tangentA, CVector2f& tangentB) const {
  if (mDirty) {
    CalculateTangents(previous, next);
  }
  tangentA = mCachedTangentA;
  tangentB = mCachedTangentB;
}

static void ValidateTangent(CVector2f& tangent) {
  if (tangent[0] < 0.f) {
    tangent[0] = 0.f;
  }
  const float magnitude = tangent.Magnitude();
  if (magnitude != 0.f) {
    tangent /= magnitude;
  }
  if (tangent[0] == 0.f && tangent[1] != 0.f) {
    tangent[0] = 0.0001f;
    tangent[1] = 5729578.f * tangent[0] * (tangent[1] < 0.f ? -1.f : 1.f);
  }
}

void CMayaSplineKnot::CalculateTangents(const CMayaSplineKnot* previous,
                                      const CMayaSplineKnot* next) const {
  mDirty = false;
  bool calculateSmooth = false;
  if (mInTangentType == kTT_Clamped && previous != nullptr) {
    const float previousDifference = fabsf(previous->GetAmplitude() - GetAmplitude());
    const float nextDifference = next != nullptr
                                     ? fabsf(next->GetAmplitude() - GetAmplitude())
                                     : previousDifference;
    if (nextDifference <= 0.05f || previousDifference <= 0.05f) {
      mInTangentType = kTT_Flat;
    }
  }

  switch (mInTangentType) {
  case kTT_Linear:
    if (previous == nullptr) {
      mCachedTangentA = CVector2f(1.f, 0.f);
    } else {
      mCachedTangentA = CVector2f(GetTime() - previous->GetTime(), GetAmplitude() - previous->GetAmplitude());
    }
    break;
  case kTT_Flat: {
    const float difference = previous != nullptr ? GetTime() - previous->GetTime()
                            : next != nullptr ? next->GetTime() - GetTime()
                                              : 0.f;
    mCachedTangentA = CVector2f(difference, 0.f);
    break;
  }
  case kTT_Step:
    mCachedTangentA = CVector2f(1.f, 0.f);
    break;
  case kTT_Clamped:
    mInTangentType = kTT_Smooth;
  case kTT_Smooth:
    calculateSmooth = true;
    break;
  }

  if (mOutTangentType == kTT_Clamped && next != nullptr) {
    const float nextDifference = fabsf(next->GetAmplitude() - GetAmplitude());
    const float previousDifference = previous != nullptr
                                     ? fabsf(previous->GetAmplitude() - GetAmplitude())
                                     : nextDifference;
    if (nextDifference <= 0.05f || previousDifference <= 0.05f) {
      mOutTangentType = kTT_Flat;
    }
  }

  switch (mOutTangentType) {
  case kTT_Linear:
    if (next == nullptr) {
      mCachedTangentB = CVector2f(1.f, 0.f);
    } else {
      mCachedTangentB = CVector2f(next->GetTime() - GetTime(), next->GetAmplitude() - GetAmplitude());
    }
    break;
  case kTT_Flat: {
    const float difference = next != nullptr ? next->GetTime() - GetTime()
                            : previous != nullptr ? GetTime() - previous->GetTime()
                                              : 0.f;
    mCachedTangentB = CVector2f(difference, 0.f);
    break;
  }
  case kTT_Step:
    mCachedTangentB = CVector2f(1.f, 0.f);
    break;
  case kTT_Clamped:
    mOutTangentType = kTT_Smooth;
  case kTT_Smooth:
    calculateSmooth = true;
    break;
  }

  if (calculateSmooth) {
    CVector2f tangentA(0.f, 0.f);
    CVector2f tangentB(0.f, 0.f);
    if (previous == nullptr && next != nullptr) {
      tangentA = tangentB = CVector2f(next->GetTime() - GetTime(),
                                     next->GetAmplitude() - GetAmplitude());
    } else if (previous != nullptr && next == nullptr) {
      tangentA = tangentB = CVector2f(GetTime() - previous->GetTime(),
                                     GetAmplitude() - previous->GetAmplitude());
    } else if (previous != nullptr && next != nullptr) {
      const float timeDifference = next->GetTime() - previous->GetTime();
      const float amplitudeDifference = next->GetAmplitude() - previous->GetAmplitude();
      float slope;
      if (timeDifference < 0.0001f) {
        slope = amplitudeDifference > 0.f ? 5729578.f : -5729578.f;
      } else {
        slope = amplitudeDifference / timeDifference;
      }
      float nextTime = next->GetTime() - GetTime();
      float previousTime = GetTime() - previous->GetTime();
      float nextAmplitude;
      float previousAmplitude;
      if (nextTime < 0.0001f) {
        nextAmplitude = slope;
        nextTime = 0.f;
      } else {
        nextAmplitude = nextTime * slope;
      }
      if (previousTime < 0.0001f) {
        previousAmplitude = slope;
        previousTime = 0.f;
      } else {
        previousAmplitude = previousTime * slope;
      }
      tangentB = CVector2f(previousTime, previousAmplitude);
      tangentA = CVector2f(nextTime, nextAmplitude);
    } else {
      tangentA = CVector2f(1.f, 0.f);
      tangentB = CVector2f(1.f, 0.f);
    }
    if (mInTangentType == kTT_Smooth) {
      mCachedTangentA = tangentA;
    }
    if (mOutTangentType == kTT_Smooth) {
      mCachedTangentB = tangentB;
    }
  }
  ValidateTangent(mCachedTangentA);
  ValidateTangent(mCachedTangentB);
}

CMayaSpline::CMayaSpline(const CMayaSpline& other)
: mPreInfinity(other.mPreInfinity)
, mPostInfinity(other.mPostInfinity)
, mKnots(other.mKnots)
, mClampMode(other.mClampMode)
, mMinAmplitude(other.mMinAmplitude)
, mMaxAmplitude(other.mMaxAmplitude)
, mCache(other.mCache) {}

void CMayaSpline::operator=(const CMayaSpline& other) {
#if NONMATCHING
  if (this == &other) {
    return;
  }
#endif
  this->~CMayaSpline();
  new (this) CMayaSpline(other);
}

CMayaSpline::CMayaSpline(const rstl::vector< CMayaSplineKnot >& knots, float minAmplitude,
                         float maxAmplitude, EClampMode clampMode, EInfinityType preInfinity,
                         EInfinityType postInfinity)
: mPreInfinity(preInfinity)
, mPostInfinity(postInfinity)
, mKnots(knots)
, mClampMode(clampMode)
, mMinAmplitude(minAmplitude)
, mMaxAmplitude(maxAmplitude) {
  rstl::sort(mKnots.begin(), mKnots.end(), rstl::less< CMayaSplineKnot >());
}

CMayaSpline::CMayaSpline()
: mPreInfinity(kIT_Constant)
, mPostInfinity(kIT_Constant)
, mClampMode(kCM_None)
, mMinAmplitude(-FLT_MAX)
, mMaxAmplitude(FLT_MAX) {}

float CMayaSpline::EvaluateHermite(float time) const {
  const float t = time - mCache.mMinTime;
  return mCache.mHermiteCoefficients[3] +
         t * (mCache.mHermiteCoefficients[2] +
              t * (mCache.mHermiteCoefficients[1] + t * mCache.mHermiteCoefficients[0]));
}

float CMayaSpline::EvaluateInfinities(float time, bool preInfinity) const {
  if (mKnots.empty()) {
    return 0.f;
  }
  const int last = mKnots.size() - 1;
  const float startTime = mKnots[0].GetTime();
  const float endTime = mKnots[last].GetTime();
  float duration = endTime - startTime;
  if (CMath::IsEpsilon(duration, 0.f, 1.e-5f)) {
    return mKnots[0].GetAmplitude();
  }

  double cycles;
  float fraction;
  if (time > endTime) {
    fraction = fabsf(static_cast< float >(modf((time - endTime) / duration, &cycles)));
  } else {
    fraction = fabsf(static_cast< float >(modf((time - startTime) / duration, &cycles)));
  }
  duration *= fraction;
  cycles = 1.f + fabsf(static_cast< float >(cycles));

  if (preInfinity) {
    if (mPreInfinity == kIT_Oscillate) {
      fraction = fmod(cycles, 2.0);
      if (!CMath::IsEpsilon(fraction, 0.f, 1.e-5f)) {
        duration = startTime + duration;
      } else {
        duration = endTime - duration;
      }
    } else if (mPreInfinity == kIT_Cycle || mPreInfinity == kIT_CycleRelative) {
      duration = endTime - duration;
    } else if (mPreInfinity == kIT_Linear) {
      time = startTime - time;
      CVector2f tangentA(0.f, 0.f);
      CVector2f tangentB(0.f, 0.f);
      mKnots[0].GetTangents(nullptr, &mKnots[1], tangentA, tangentB);
      float amplitude = mKnots[0].GetAmplitude();
      if (!CMath::IsEpsilon(tangentA.GetX(), 0.f, 1.e-5f)) {
        amplitude -= time * tangentA.GetY() / tangentA.GetX();
      }
      return amplitude;
    }
  } else {
    if (mPostInfinity == kIT_Oscillate) {
      fraction = fmod(cycles, 2.0);
      if (!CMath::IsEpsilon(fraction, 0.f, 1.e-5f)) {
        duration = endTime - duration;
      } else {
        duration = startTime + duration;
      }
    } else if (mPostInfinity == kIT_Cycle || mPostInfinity == kIT_CycleRelative) {
      duration = startTime + duration;
    } else if (mPostInfinity == kIT_Linear) {
      time = time - endTime;
      CVector2f tangentA(0.f, 0.f);
      CVector2f tangentB(0.f, 0.f);
      mKnots[last].GetTangents(last > 0 ? &mKnots[last - 1] : nullptr, nullptr,
                                tangentA, tangentB);
      float amplitude = mKnots[last].GetAmplitude();
      if (!CMath::IsEpsilon(tangentB.GetX(), 0.f, 1.e-5f)) {
        amplitude += time * tangentB.GetY() / tangentB.GetX();
      }
      return amplitude;
    }
  }

  float amplitude = EvaluateAt(duration);
  if (preInfinity && mPreInfinity == kIT_CycleRelative) {
    const float delta = mKnots[last].GetAmplitude() - mKnots[0].GetAmplitude();
    amplitude -= static_cast< float >(cycles) * delta;
  } else if (!preInfinity && mPostInfinity == kIT_CycleRelative) {
    const float delta = mKnots[last].GetAmplitude() - mKnots[0].GetAmplitude();
    amplitude += static_cast< float >(cycles) * delta;
  }
  return amplitude;
}

bool CMayaSpline::FindKnot(float time, int& knotIndex) const {
  knotIndex = 0;
  if (!mKnots.empty()) {
    int low = 0;
    int high = mKnots.size() - 1;
    do {
      const int middle = (low + high) >> 1;
      const float knotTime = mKnots[middle].GetTime();
      if (time < knotTime) {
        high = middle - 1;
      } else if (time > knotTime) {
        low = middle + 1;
      } else {
        knotIndex = middle;
        return true;
      }
    } while (low <= high);
    knotIndex = low;
  }
  return false;
}

float CMayaSpline::EvaluateAt(float time) const {
  const float amplitude = EvaluateAtUnclamped(time);
  switch (mClampMode) {
  case kCM_Clamp:
    return CMath::FastClamp(mMinAmplitude, amplitude, mMaxAmplitude);
  case kCM_Wrap: {
    const float range = mMaxAmplitude - mMinAmplitude;
    if (range > 0.f) {
      if (amplitude > FLT_EPSILON + mMaxAmplitude) {
        return amplitude - range * (static_cast< int >((amplitude - mMaxAmplitude) / range) + 1);
      }
      if (amplitude < mMinAmplitude - FLT_EPSILON) {
        return amplitude + range *
                               (abs(static_cast< int >((amplitude - mMinAmplitude) / range)) + 1);
      }
      return amplitude;
    }
    return mMinAmplitude;
  }
  case kCM_None:
    return amplitude;
  default:
    return 0.f;
  }
}

float CMayaSpline::EvaluateAtUnclamped(float time) const {
  if (GetKnots().empty()) {
    return 0.f;
  }
  const int last = GetKnots().size() - 1;
  if (time < GetKnots()[0].GetTime()) {
    if (mPreInfinity == kIT_Constant) {
      return GetKnots()[0].GetAmplitude();
    }
    return EvaluateInfinities(time, true);
  }
  if (time > GetKnots()[last].GetTime()) {
    if (mPostInfinity == kIT_Constant) {
      return GetKnots()[last].GetAmplitude();
    }
    return EvaluateInfinities(time, false);
  }

  int knotIndex = -1;
  bool found = false;
  const int& cached = mCache.mKnotIndex;
  if (cached != -1) {
#if NONMATCHING
    // Use the cached knot to enable the forward shortcut.
    if (cached < last && time > GetKnots()[cached].GetTime()) {
#else
    // The original checks the final knot, making this shortcut unreachable for finite times.
    if (cached < last && time > GetKnots()[last].GetTime()) {
#endif
      const int next = cached + 1;
      if (time == GetKnots()[next].GetTime()) {
#if NONMATCHING
        mCache.mKnotIndex = next;
        return GetKnots()[next].GetAmplitude();
#else
        mCache.mKnotIndex = last;
        return GetKnots()[last].GetAmplitude();
#endif
      }
      if (time < GetKnots()[next].GetTime()) {
        knotIndex = next;
        found = true;
      }
    } else if (cached > 0 && time < GetKnots()[mCache.mKnotIndex].GetTime()) {
      const int previous = cached - 1;
      if (time > GetKnots()[previous].GetTime()) {
        knotIndex = cached;
        found = true;
      }
      if (time == GetKnots()[previous].GetTime()) {
        mCache.mKnotIndex = previous;
        return GetKnots()[mCache.mKnotIndex].GetAmplitude();
      }
    }
  }
  if (!found && FindKnot(time, knotIndex)) {
#if NONMATCHING
    // An exact knot hit has the same amplitude regardless of the preceding segment's mode.
    mCache.mKnotIndex = knotIndex;
    return GetKnots()[knotIndex].GetAmplitude();
#else
    if (knotIndex == 0) {
      mCache.mKnotIndex = knotIndex;
      return GetKnots()[knotIndex].GetAmplitude();
    }
    if (knotIndex == GetKnots().size()) {
      mCache.mKnotIndex = 0;
      return GetKnots()[last].GetAmplitude();
    }
#endif
  }

  const int segment = knotIndex - 1;
  if (mCache.mSegmentIndex != segment) {
    mCache.mKnotIndex = segment;
    mCache.mSegmentIndex = segment;
    if (GetKnots()[segment].GetOutTangentType() == CMayaSplineKnot::kTT_Step) {
      mCache.mStep = true;
    } else {
      mCache.mStep = false;
      rstl::reserved_vector< CVector2f, 4 > points;
      FindControlPoints(segment, points);
      CalculateHermiteCoefficients(points, mCache.mHermiteCoefficients);
      mCache.mMinTime = points[0].GetX();
    }
  }
  if (mCache.mStep) {
#if NONMATCHING
    // Exact knot hits can move the lookup index without changing the cached segment.
    return GetKnots()[mCache.mSegmentIndex].GetAmplitude();
#else
    return GetKnots()[mCache.mKnotIndex].GetAmplitude();
#endif
  }
  return EvaluateHermite(time);
}

CMayaSpline CMayaSpline::BuildLinearSpline(float timeA, float amplitudeA, float timeB,
                                          float amplitudeB) {
  rstl::vector< CMayaSplineKnot > knots;
  knots.reserve(2);
  knots.push_back_unsafe(CMayaSplineKnot(timeA, amplitudeA, CMayaSplineKnot::kTT_Linear,
                                 CMayaSplineKnot::kTT_Linear));
  knots.push_back_unsafe(CMayaSplineKnot(timeB, amplitudeB, CMayaSplineKnot::kTT_Linear,
                                 CMayaSplineKnot::kTT_Linear));
  return CMayaSpline(knots, amplitudeA, amplitudeB, kCM_None, kIT_Constant, kIT_Constant);
}

CMayaSpline CMayaSpline::BuildSpline(const CMayaSplineKnot* knots, uint count, EClampMode clampMode,
                                    EInfinityType preInfinity, EInfinityType postInfinity,
                                    float minAmplitude, float maxAmplitude) {
  rstl::vector< CMayaSplineKnot > knotVector(count);
  for (uint i = 0; i < count; ++i) {
    knotVector.push_back_unsafe(knots[i]);
  }
  return CMayaSpline(knotVector, minAmplitude, maxAmplitude, clampMode, preInfinity, postInfinity);
}

void CMayaSpline::FindControlPoints(int knotIndex,
                                    rstl::reserved_vector< CVector2f, 4 >& points) const {
  const CMayaSplineKnot* knot = &mKnots[knotIndex];
  points.push_back(CVector2f(knot->GetTime(), knot->GetAmplitude()));
  CVector2f tangentA(0.f, 0.f);
  CVector2f tangentB(0.f, 0.f);
  knot->GetTangents(knotIndex - 1 >= 0 ? &mKnots[knotIndex - 1] : nullptr,
                    knotIndex + 1 < mKnots.size() ? &mKnots[knotIndex + 1] : nullptr,
                    tangentA, tangentB);
  points.push_back(points[0] + tangentB * (1.f / 3.f));

  ++knotIndex;
  knot = &mKnots[knotIndex];
  CVector2f tangentC(0.f, 0.f);
  CVector2f tangentD(0.f, 0.f);
  knot->GetTangents(knotIndex - 1 >= 0 ? &mKnots[knotIndex - 1] : nullptr,
                     knotIndex + 1 < mKnots.size() ? &mKnots[knotIndex + 1] : nullptr,
                     tangentC, tangentD);
  const CVector2f end(knot->GetTime(), knot->GetAmplitude());
  points.push_back(end - tangentC * (1.f / 3.f));
  points.push_back(end);
}

void CMayaSpline::CalculateHermiteCoefficients(
    const rstl::reserved_vector< CVector2f, 4 >& points, float* coefficients) const {
  const CVector2f span = points[3] - points[0];
  float slopeA = 5729578.f;
  const CVector2f tangentA = points[1] - points[0];
  if (tangentA.GetX() != 0.f) {
    slopeA = tangentA.GetY() / tangentA.GetX();
  }
  float slopeB = 5729578.f;
  const CVector2f tangentB = points[3] - points[2];
  if (tangentB.GetX() != 0.f) {
    slopeB = tangentB.GetY() / tangentB.GetX();
  }
  const float& dy = span[1];
  const float dx = span.GetX();
  const float invSquare = 1.f / (dx * dx);
  const float scaledA = slopeA * dx;
  const float scaledB = slopeB * dx;
  coefficients[0] = invSquare * (scaledA + scaledB - dy - dy) / dx;
  coefficients[1] = invSquare * (dy + (dy + dy) - scaledA - scaledA - scaledB);
  coefficients[2] = slopeA;
  coefficients[3] = points[0].GetY();
}

#endif
#else
CMayaSplineKnot::CMayaSplineKnot(CInputStream& in)
: mTime(in.ReadFloat())
, mAmplitude(in.ReadFloat())
, mFlagA(in.ReadInt8())
, mFlagB(in.ReadInt8())
, mDirty(true)
, mCachedTangentA(CVector2f(0.f, 0.f))
, mCachedTangentB(CVector2f(0.f, 0.f)) {
  if (mFlagA == 5) {
    float x = in.ReadFloat();
    float y = in.ReadFloat();
    mCachedTangentA.SetX(x);
    mCachedTangentA.SetY(y);
  }
  if (mFlagB == 5) {
    float x = in.ReadFloat();
    float y = in.ReadFloat();
    mCachedTangentB.SetX(x);
    mCachedTangentB.SetY(y);
  }
}

CMayaSplineKnot::CMayaSplineKnot(float time, float amplitude, int flagA, int flagB,
                                 const CAbsAngle& inAngle, const CAbsAngle& outAngle)
: mTime(time)
, mAmplitude(amplitude)
, mFlagA(flagA)
, mFlagB(flagB)
, mDirty(true)
, mCachedTangentA(CVector2f(0.0f, 0.0f))
, mCachedTangentB(CVector2f(0.0f, 0.0f)) {
  if (flagA == 5) {
    float s = sin(inAngle.AsRadians());
    float c = cos(inAngle.AsRadians());
    mCachedTangentA = CVector2f(3.0f * c, 3.0f * s);
  }
  if (flagB == 5) {
    float s = sin(outAngle.AsRadians());
    float c = cos(outAngle.AsRadians());
    mCachedTangentB = CVector2f(3.0f * c, 3.0f * s);
  }
}

void CMayaSplineKnot::GetTangents(const CMayaSplineKnot* prev, const CMayaSplineKnot* next,
                                  CVector2f& tangentA, CVector2f& tangentB) const {
  if (mDirty) {
    CalculateTangents(prev, next);
  }

  tangentA = mCachedTangentA;
  tangentB = mCachedTangentB;
}

void ValidateTangent(CVector2f& tangent) {
  if (tangent.GetX() < 0.f) {
    tangent.SetX(0.f);
  }

  const float mag = tangent.Magnitude();
  if (mag != 0.f) {
    tangent /= mag;
  }

  if (tangent.GetX() == 0.f && tangent.GetY() != 0.f) {
    tangent.SetX(0.0001f);
    tangent.SetY(5729578.0f * tangent.GetX() * (tangent.GetY() < 0.f ? -1.f : 1.f));
  }
}

void CMayaSplineKnot::CalculateTangents(const CMayaSplineKnot* prev,
                                        const CMayaSplineKnot* next) const {
  mDirty = false;
  bool calculateTangents = false;
  if (mFlagA == 4 && prev != nullptr) {
    float prevAmplitude = CMath::AbsF(prev->GetAmplitude() - GetAmplitude());
    float nextAmplitude;
    if (next != nullptr) {
      nextAmplitude = CMath::AbsF(next->GetAmplitude() - GetAmplitude());
    } else {
      nextAmplitude = prevAmplitude;
    }
    if (nextAmplitude <= 0.05f || prevAmplitude <= 0.05f) {
      mFlagA = 1;
    }
  }

  switch (mFlagA) {
  case 0:
    if (prev == nullptr) {
      mCachedTangentA = CVector2f(1.f, 0.f);
    } else {
      mCachedTangentA =
          CVector2f(GetTime() - prev->GetTime(), GetAmplitude() - prev->GetAmplitude());
    }
    break;
  case 1:
    mCachedTangentA = CVector2f(prev != nullptr   ? GetTime() - prev->GetTime()
                                : next != nullptr ? next->GetTime() - GetTime()
                                                  : 0.f,
                                0.f);
    break;
  case 3:
    mCachedTangentA = CVector2f(1.f, 0.f);
    break;
  case 4:
    mFlagA = 2;
  case 2:
    calculateTangents = true;
    break;
  case 5:
    break;
  }

  if (mFlagB == 4 && next != nullptr) {
    float nextAmplitude = CMath::AbsF(next->GetAmplitude() - GetAmplitude());
    float prevAmplitude;
    if (prev != nullptr) {
      prevAmplitude = CMath::AbsF(prev->GetAmplitude() - GetAmplitude());
    } else {
      prevAmplitude = nextAmplitude;
    }
    if (nextAmplitude <= 0.05f || prevAmplitude <= 0.05f) {
      mFlagB = 1;
    }
  }

  switch (mFlagB) {
  case 0:
    if (next == nullptr) {
      mCachedTangentB = CVector2f(1.f, 0.f);
    } else {
      mCachedTangentB =
          CVector2f(next->GetTime() - GetTime(), next->GetAmplitude() - GetAmplitude());
    }
    break;
  case 1:
    mCachedTangentB = CVector2f(next != nullptr   ? next->GetTime() - GetTime()
                                : prev != nullptr ? GetTime() - prev->GetTime()
                                                  : 0.f,
                                0.f);
    break;
  case 3:
    mCachedTangentB = CVector2f(1.f, 0.f);
    break;
  case 4:
    mFlagB = 2;
  case 2:
    calculateTangents = true;
    break;
  case 5:
    break;
  }

  if (calculateTangents) {
    CVector2f tangentA(0.f, 0.f);
    CVector2f tangentB(0.f, 0.f);
    if (prev == nullptr && next != nullptr) {
      tangentA = tangentB =
          CVector2f(next->GetTime() - GetTime(), next->GetAmplitude() - GetAmplitude());
    } else if (prev != nullptr && next == nullptr) {
      tangentA = tangentB =
          CVector2f(GetTime() - prev->GetTime(), GetAmplitude() - prev->GetAmplitude());
    } else if (prev != nullptr && next != nullptr) {
      float timeDiff = next->GetTime() - prev->GetTime();
      float amplitudeDiff = next->GetAmplitude() - prev->GetAmplitude();
      float slope;
      if (timeDiff < 0.0001f) {
        slope = amplitudeDiff > 0.f ? 5729578.f : -5729578.f;
      } else {
        slope = amplitudeDiff / timeDiff;
      }
      float nextTime = next->GetTime() - GetTime();
      float prevTime = GetTime() - prev->GetTime();
      float nextAmplitude;
      float prevAmplitude;
      if (nextTime < 0.0001f) {
        nextAmplitude = slope;
        nextTime = 0.f;
      } else {
        nextAmplitude = nextTime * slope;
      }
      if (prevTime < 0.0001f) {
        prevAmplitude = slope;
        prevTime = 0.f;
      } else {
        prevAmplitude = prevTime * slope;
      }
      tangentB = CVector2f(prevTime, prevAmplitude);
      tangentA = CVector2f(nextTime, nextAmplitude);
    } else {
      tangentA = CVector2f(1.f, 0.f);
      tangentB = CVector2f(1.f, 0.f);
    }
    if (mFlagA == 2) {
      mCachedTangentA = tangentA;
    }
    if (mFlagB == 2) {
      mCachedTangentB = tangentB;
    }
  }
  ValidateTangent(mCachedTangentA);
  ValidateTangent(mCachedTangentB);
}

CMayaSpline::CMayaSpline(CInputStream& in)
: mPreInfinity(in.ReadInt8())
, mPostInfinity(in.ReadInt8())
, mKnots(in)
, mClampMode(in.ReadInt8())
, mMinAmplitude(in.ReadFloat())
, mMaxAmplitude(in.ReadFloat())
, mCache() {}

#if RSTL_VERSION >= RSTL_R32J
CMayaSpline::CMayaSpline(const CMayaSpline& other)
: mPreInfinity(other.mPreInfinity)
, mPostInfinity(other.mPostInfinity)
, mKnots(other.mKnots)
, mClampMode(other.mClampMode)
, mMinAmplitude(other.mMinAmplitude)
, mMaxAmplitude(other.mMaxAmplitude)
, mCache(other.mCache) {}

void CMayaSpline::operator=(const CMayaSpline& other) {
  this->~CMayaSpline();
  new (this) CMayaSpline(other);
}
#endif

CMayaSpline::CMayaSpline() : mPreInfinity(0), mPostInfinity(0), mKnots(), mClampMode(0), mCache() {}

static rstl::less< CMayaSplineKnot > sKnotLess;

CMayaSpline::CMayaSpline(const rstl::vector< CMayaSplineKnot >& knots, int clampMode,
                         int preInfinity, int postInfinity, float minAmplitudeTime,
                         float maxAmplitudeTime)
: mPreInfinity(preInfinity)
, mPostInfinity(postInfinity)
, mKnots(knots)
, mClampMode(clampMode)
, mMinAmplitude(minAmplitudeTime)
, mMaxAmplitude(maxAmplitudeTime)
, mCache() {
  rstl::sort(mKnots.begin(), mKnots.end(), sKnotLess);
}

float CMayaSpline::EvaluateHermite(float time) const {
  const float timeDiff = time - mCache.mMinTime;
  return ((timeDiff * mCache.mHermiteCoefs[0] + mCache.mHermiteCoefs[1]) * timeDiff +
          mCache.mHermiteCoefs[2]) *
             timeDiff +
         mCache.mHermiteCoefs[3];
}

float CMayaSpline::EvaluateInfinities(float time, bool pre) const {
  if (mKnots.empty()) {
    return 0.f;
  }

  int lastIdx = mKnots.size() - 1;
  const CMayaSplineKnot* curKnot = &mKnots[0];
  const float startTime = mKnots[0].GetTime();
  const float endTime = mKnots[lastIdx].GetTime();
  float center = endTime - startTime;

  if (CMath::IsEpsilon(center, 0, 1.e-5f)) {
    return curKnot->GetAmplitude();
  }

  double tmp;
  float divTime;
  if (time > endTime) {
    divTime = CMath::AbsF(modf((time - endTime) / center, &tmp));
  } else {
    divTime = CMath::AbsF(modf((time - startTime) / center, &tmp));
  }
  center *= divTime;
  tmp = 1.f + CMath::AbsF(tmp);

  if (pre) {
    if (mPreInfinity == 4) {
      divTime = CMath::ModF(tmp, 2.f);
      if (!CMath::IsEpsilon(divTime, 0.f, 1.e-5f)) {
        center = startTime + center;
      } else {
        center = endTime - center;
      }
    } else if (mPreInfinity == 2 || mPreInfinity == 3) {
      center = endTime - center;
    } else if (mPreInfinity == 1) {
      center = (startTime - time);
      CVector2f tangentA(0.0f, 0.0f);
      CVector2f tangentB(0.0f, 0.0f);
      mKnots[0].GetTangents(nullptr, &mKnots[1], tangentA, tangentB);
      const float amplitude = mKnots[0].GetAmplitude();
      return !CMath::IsEpsilon(tangentA.GetX(), 0.f, 1.e-5f)
                 ? amplitude - center * tangentA.GetY() / tangentA.GetX()
                 : amplitude;
    }
  } else {
    if (mPostInfinity == 4) {
      divTime = CMath::ModF(tmp, 2.f);
      if (!CMath::IsEpsilon(divTime, 0.f, 1.e-5f)) {
        center = endTime - center;
      } else {
        center = startTime + center;
      }
    } else if (mPostInfinity == 2 || mPostInfinity == 3) {
      center = startTime + center;
    } else if (mPostInfinity == 1) {
      center = time - endTime;
      CVector2f tangentA(0.0f, 0.0f);
      CVector2f tangentB(0.0f, 0.0f);
      mKnots[lastIdx].GetTangents(lastIdx > 0 ? &mKnots[lastIdx - 1] : nullptr, nullptr, tangentA,
                                  tangentB);
      const float amplitude = mKnots[lastIdx].GetAmplitude();
      return !CMath::IsEpsilon(tangentB.GetX(), 0.f, 1.e-5f)
                 ? amplitude + center * tangentB.GetY() / tangentB.GetX()
                 : amplitude;
    }
  }

  float eval = EvaluateAt(center);
  if (pre && mPreInfinity == 3) {
    return eval - (float(tmp) * (mKnots[lastIdx].GetAmplitude() - mKnots[0].GetAmplitude()));
  }

  if (!pre && mPostInfinity == 3) {
    return eval + (float(tmp) * (mKnots[lastIdx].GetAmplitude() - mKnots[0].GetAmplitude()));
  }
  return eval;
}

bool CMayaSpline::FindKnot(float time, int& knotIndex) const {
  knotIndex = 0;
  int count = mKnots.size();
  if (count != 0) {
    int lower = 0;
    int upper = count - 1;
    do {
      int index = (lower + upper) >> 1;
      const CMayaSplineKnot& knot = mKnots[index];
      if (time < knot.GetTime()) {
        upper = index - 1;
      } else if (time > knot.GetTime()) {
        lower = index + 1;
      } else {
        knotIndex = index;
        return true;
      }
    } while (lower <= upper);
    knotIndex = lower;
  }

  return false;
}

float CMayaSpline::EvaluateAt(float time) const {
  const float amplitude = EvaluateAtUnclamped(time);
  switch (mClampMode) {
  case 0:
    break;
  case 1:
    return CMath::Clamp(mMinAmplitude, amplitude, mMaxAmplitude);
  case 2: {
    const float range = mMaxAmplitude - mMinAmplitude;

    if (range > 0.f) {
      if (amplitude > FLT_EPSILON + mMaxAmplitude) {
        return amplitude - range * (static_cast< int >((amplitude - mMaxAmplitude) / range) + 1);
      }
      if (amplitude < mMinAmplitude - FLT_EPSILON) {
        return amplitude +
               range * (abs(static_cast< int >((amplitude - mMinAmplitude) / range)) + 1);
      }
      return amplitude;
    }
    return mMinAmplitude;
  }
  }

  return amplitude;
}

float CMayaSpline::EvaluateAtUnclamped(float time) const {
  if (mKnots.empty()) {
    return 0.f;
  }

  int lastIdx = mKnots.size() - 1;
  if (time < mKnots[0].GetTime()) {
    if (mPreInfinity == 0) {
      return mKnots[0].GetAmplitude();
    }
    return EvaluateInfinities(time, true);
  }

  if (time > mKnots[lastIdx].GetTime()) {
    if (mPostInfinity == 0) {
      return mKnots[lastIdx].GetAmplitude();
    }
    return EvaluateInfinities(time, false);
  }

  int nextKnotIndex = -1;
  bool segmentKnown = false;
  const int& cachedKnotIndex = mCache.mKnotIndex;
  if (cachedKnotIndex != -1) {
    // The target compares against the last knot here, not the cached one.
    if (cachedKnotIndex < lastIdx && time > mKnots[lastIdx].GetTime()) {
      int next = cachedKnotIndex + 1;
      float nextTime = mKnots[next].GetTime();
      if (time == nextTime) {
        mCache.mKnotIndex = lastIdx;
        return mKnots[mCache.mKnotIndex].GetAmplitude();
      }
      if (time < nextTime) {
        nextKnotIndex = next;
        segmentKnown = true;
      }
    } else if (cachedKnotIndex > 0 && time < mKnots[cachedKnotIndex].GetTime()) {
      int previousKnotIndex = cachedKnotIndex - 1;
      if (time > mKnots[previousKnotIndex].GetTime()) {
        nextKnotIndex = cachedKnotIndex;
        segmentKnown = true;
      }
      if (time == mKnots[previousKnotIndex].GetTime()) {
        mCache.mKnotIndex = previousKnotIndex;
        return mKnots[mCache.mKnotIndex].GetAmplitude();
      }
    }
  }

  if (!segmentKnown && FindKnot(time, nextKnotIndex)) {
    if (nextKnotIndex == 0) {
      mCache.mKnotIndex = nextKnotIndex;
      return mKnots[nextKnotIndex].GetAmplitude();
    }
    if (nextKnotIndex == mKnots.size()) {
      mCache.mKnotIndex = 0;
      return mKnots[lastIdx].GetAmplitude();
    }
  }

  const int segment = nextKnotIndex - 1;
  if (mCache.mSegmentIndex != segment) {
    mCache.mKnotIndex = segment;
    mCache.mSegmentIndex = segment;
    if (mKnots[mCache.mKnotIndex].GetTangentModeB() == 3) {
      mCache.mStepSegment = true;
    } else {
      mCache.mStepSegment = false;
      rstl::reserved_vector< CVector2f, 4 > points;
      FindControlPoints(mCache.mKnotIndex, points);
      CalculateHermiteCoefficients(points, mCache.mHermiteCoefs);
      mCache.mMinTime = points[0].GetX();
    }
  }

  if (mCache.mStepSegment) {
    return mKnots[mCache.mKnotIndex].GetAmplitude();
  }
  return EvaluateHermite(time);
}

CMayaSpline CMayaSpline::CreateFor(float timeA, float amplitudeA, float timeB, float amplitudeB) {
  rstl::vector< CMayaSplineKnot > knots;
  knots.reserve(2);
  knots.push_back_unsafe(CMayaSplineKnot(timeA, amplitudeA, 2, 2));
  knots.push_back_unsafe(CMayaSplineKnot(timeB, amplitudeB, 2, 2));
  return CMayaSpline(knots, 0, 0, 0, -FLT_MAX, FLT_MAX);
}

const rstl::vector< CMayaSplineKnot >& CMayaSpline::GetKnots() const { return mKnots; }

float CMayaSpline::GetMaxTime() const {
  int count = mKnots.size();
  if (count == 0) {
    return 0.0f;
  }
  return mKnots[count - 1].GetTime();
}

float CMayaSpline::GetDuration() const {
  if (mKnots.empty()) {
    return 0.0f;
  }
  return GetMaxTime() - mKnots[0].GetTime();
}

size_t CMayaSpline::GetKnotCount() const { return mKnots.size(); }

void CMayaSpline::FindControlPoints(int knotIndex,
                                    rstl::reserved_vector< CVector2f, 4 >& controlPoints) const {
  const CMayaSplineKnot* knot = &mKnots[knotIndex];
  controlPoints.push_back(CVector2f(knot->GetTime(), knot->GetAmplitude()));

  CVector2f tangentA(0.f, 0.f);
  CVector2f tangentB(0.f, 0.f);
  knot->GetTangents(knotIndex - 1 >= 0 ? &mKnots[knotIndex - 1] : nullptr,
                    knotIndex + 1 < mKnots.size() ? &mKnots[knotIndex + 1] : nullptr, tangentA,
                    tangentB);
  controlPoints.push_back(controlPoints[0] + tangentB * (1.f / 3.f));

  const int nextKnotIndex = knotIndex + 1;
  knot = &mKnots[nextKnotIndex];
  CVector2f nextTangentA(0.f, 0.f);
  CVector2f nextTangentB(0.f, 0.f);
  knot->GetTangents(nextKnotIndex - 1 >= 0 ? &mKnots[nextKnotIndex - 1] : nullptr,
                    nextKnotIndex + 1 < mKnots.size() ? &mKnots[nextKnotIndex + 1] : nullptr,
                    nextTangentA, nextTangentB);
  CVector2f knotPoint(knot->GetTime(), knot->GetAmplitude());
  controlPoints.push_back(knotPoint - nextTangentA * (1.f / 3.f));
  controlPoints.push_back(knotPoint);
}

void CMayaSpline::CalculateHermiteCoefficients(
    const rstl::reserved_vector< CVector2f, 4 >& controlPoints, float* coefs) const {
  const CVector2f span = controlPoints[3] - controlPoints[0];
  float slopeA = 5729578.f;
  const CVector2f tangentA = controlPoints[1] - controlPoints[0];
  if (tangentA[0] != 0.f) {
    slopeA = tangentA[1] / tangentA[0];
  }
  float slopeB = 5729578.f;
  const CVector2f tangentB = controlPoints[3] - controlPoints[2];
  if (tangentB[0] != 0.f) {
    slopeB = tangentB[1] / tangentB[0];
  }
  const float time = span[0];
  const float amplitude = span[1];
  const float invTimeSq = 1.f / (time * time);
  const float amplitudeA = slopeA * time;
  const float amplitudeB = slopeB * time;
  coefs[0] = invTimeSq * ((amplitudeA + amplitudeB - amplitude) - amplitude) / time;
  coefs[1] =
      invTimeSq * (((amplitude + amplitude + amplitude - amplitudeA) - amplitudeA) - amplitudeB);
  coefs[2] = slopeA;
  coefs[3] = controlPoints[0].GetY();
}

void CMayaSpline::FindSegmentIntersections(float amplitude, int knotIndex,
                                           rstl::reserved_vector< float, 3 >& intersections) const {
  if (mKnots[knotIndex].GetTangentModeB() == 3) {
    if (CMath::IsEpsilon(mKnots[knotIndex].GetAmplitude(), amplitude, 0.002f)) {
      intersections.push_back(mKnots[knotIndex].GetTime());
      intersections.push_back(mKnots[knotIndex + 1].GetTime());
    }
    return;
  }

  rstl::reserved_vector< CVector2f, 4 > points;
  FindControlPoints(knotIndex, points);
  float coefs[4];
  CalculateHermiteCoefficients(points, coefs);
  coefs[3] -= amplitude;
  double roots[4];
  int count = CMath::SolveCubicDouble(coefs[3], coefs[2], coefs[1], coefs[0], FLT_EPSILON, roots);
  const float start = mKnots[knotIndex].GetTime();
  const float end = mKnots[knotIndex + 1].GetTime();
  if (count < 0 && CMath::IsEpsilon(mKnots[knotIndex].GetAmplitude(), amplitude, 0.002f)) {
    count = 2;
    roots[0] = 0.0;
    roots[1] = end - start;
  }
  if (count > 0 && intersections.size() != 3) {
    for (int i = 0; i < count; ++i) {
      float time = start + float(roots[i]);
      if (CMath::IsEpsilon(time, start, 0.002f)) {
        time = start;
      }
      if (CMath::IsEpsilon(time, end, 0.002f)) {
        time = end;
      }
      if (time >= start && time <= end) {
        intersections.push_back(time);
      }
    }
  }
  rstl::sort(intersections.begin(), intersections.end());
}

void CMayaSpline::FindIntersections(float amplitude,
                                    rstl::reserved_vector< float, 8 >& intersections) const {
  if (mKnots.size() < 2) {
    return;
  }
  rstl::reserved_vector< float, 3 > segmentIntersections;
  for (int i = 0; i < mKnots.size() - 1; ++i) {
    segmentIntersections.clear();
    FindSegmentIntersections(amplitude, i, segmentIntersections);
    if (!segmentIntersections.empty()) {
      for (int j = 0; j < segmentIntersections.size(); ++j) {
        if (intersections.size() != 8) {
          intersections.push_back(segmentIntersections[j]);
        }
      }
    }
  }

  float* last = intersections.begin();
  for (float* next = intersections.begin() + 1; next <= intersections.end() - 1; ++next) {
    if (!CMath::IsEpsilon(*next, *last, 0.002f)) {
      *++last = *next;
    }
  }
  const int count = last - intersections.begin() + 1;
  intersections.resize(count, 0.f);
}

rstl::reserved_vector< float, 8 >
CMayaSpline::FilterLeftIntersections(const rstl::reserved_vector< float, 8 >& intersections) const {
  rstl::reserved_vector< float, 8 > result;
  for (int i = 0; i <= intersections.size() - 1; ++i) {
    const float time = intersections[i];
    for (int j = 0; j < mKnots.size() - 1; ++j) {
      bool atEndpoint = false;
      if (CMath::IsEpsilon(time, mKnots[j].GetTime(), 0.002f) ||
          CMath::IsEpsilon(time, mKnots[j + 1].GetTime(), 0.002f)) {
        atEndpoint = true;
      }
      if (atEndpoint || (time >= mKnots[j].GetTime() && time <= mKnots[j + 1].GetTime())) {
        CVector2f tangentA = CVector2f::Zero();
        CVector2f tangentB = CVector2f::Zero();
        const CMayaSplineKnot* next = &mKnots[j + 1];
        mKnots[j].GetTangents(nullptr, next, tangentA, tangentB);
        if (CMath::IsEpsilon(mKnots[j].GetAmplitude(), 0.f, 1.e-5f) && tangentB.GetY() < 0.f) {
          tangentB.SetY(0.f);
        }
        if (CMath::IsEpsilon(mKnots[j].GetAmplitude(), 1.f, 1.e-5f) && tangentB.GetY() > 0.f) {
          tangentB.SetY(0.f);
        }
        CVector2f nextTangentA = CVector2f::Zero();
        CVector2f nextTangentB = CVector2f::Zero();
        const CMayaSplineKnot* prev = &mKnots[j];
        mKnots[j + 1].GetTangents(prev, nullptr, nextTangentA, nextTangentB);
        if (CMath::IsEpsilon(mKnots[j + 1].GetAmplitude(), 0.f, 1.e-5f) &&
            nextTangentA.GetY() > 0.f) {
          nextTangentA.SetY(0.f);
        }
        if (CMath::IsEpsilon(mKnots[j + 1].GetAmplitude(), 1.f, 1.e-5f) &&
            nextTangentA.GetY() < 0.f) {
          nextTangentA.SetY(0.f);
        }
        if (mKnots[j].GetTangentModeB() == 3 ||
            (CMath::IsEpsilon(mKnots[j].GetAmplitude(), mKnots[j + 1].GetAmplitude(), 1.e-5f) &&
             CMath::IsEpsilon(tangentB.GetY(), 0.f, 1.e-5f) &&
             CMath::IsEpsilon(nextTangentA.GetY(), 0.f, 1.e-5f))) {
          if (!CMath::IsEpsilon(time, mKnots[j].GetTime(), 0.002f)) {
            break;
          }
          result.push_back(time);
          if (result.size() == 8) {
            break;
          }
        } else {
          result.push_back(time);
          break;
        }
      }
    }
  }
  return result;
}

rstl::reserved_vector< float, 8 > CMayaSpline::FilterRightIntersections(
    const rstl::reserved_vector< float, 8 >& intersections) const {
  rstl::reserved_vector< float, 8 > result;
  for (int i = 0; i <= intersections.size() - 1; ++i) {
    const float time = intersections[i];
    for (int j = mKnots.size() - 2; j >= 0; --j) {
      bool atEndpoint = false;
      if (CMath::IsEpsilon(time, mKnots[j].GetTime(), 0.002f) ||
          CMath::IsEpsilon(time, mKnots[j + 1].GetTime(), 0.002f)) {
        atEndpoint = true;
      }
      if (atEndpoint || (time >= mKnots[j].GetTime() && time <= mKnots[j + 1].GetTime())) {
        if (IsSegmentConstant(j)) {
          if (!CMath::IsEpsilon(time, mKnots[j + 1].GetTime(), 0.002f)) {
            break;
          }
          result.push_back(time);
        } else {
          result.push_back(time);
          break;
        }
      }
    }
  }
  return result;
}

bool CMayaSpline::IsSegmentConstant(int knotIndex) const {
  if (knotIndex > mKnots.size() - 2) {
    return false;
  }
  CVector2f tangentA = CVector2f::Zero();
  CVector2f tangentB = CVector2f::Zero();
  const CMayaSplineKnot* next = &mKnots[knotIndex + 1];
  mKnots[knotIndex].GetTangents(nullptr, next, tangentA, tangentB);
  if (CMath::IsEpsilon(mKnots[knotIndex].GetAmplitude(), 0.f, 1.e-5f) && tangentB.GetY() < 0.f) {
    tangentB.SetY(0.f);
  }
  if (CMath::IsEpsilon(mKnots[knotIndex].GetAmplitude(), 1.f, 1.e-5f) && tangentB.GetY() > 0.f) {
    tangentB.SetY(0.f);
  }
  CVector2f nextTangentA = CVector2f::Zero();
  CVector2f nextTangentB = CVector2f::Zero();
  const CMayaSplineKnot* prev = &mKnots[knotIndex];
  mKnots[knotIndex + 1].GetTangents(prev, nullptr, nextTangentA, nextTangentB);
  if (CMath::IsEpsilon(mKnots[knotIndex + 1].GetAmplitude(), 0.f, 1.e-5f) &&
      nextTangentA.GetY() > 0.f) {
    nextTangentA.SetY(0.f);
  }
  if (CMath::IsEpsilon(mKnots[knotIndex + 1].GetAmplitude(), 1.f, 1.e-5f) &&
      nextTangentA.GetY() < 0.f) {
    nextTangentA.SetY(0.f);
  }
  if (mKnots[knotIndex].GetTangentModeB() == 3 ||
      (CMath::IsEpsilon(mKnots[knotIndex].GetAmplitude(), mKnots[knotIndex + 1].GetAmplitude(),
                        1.e-5f) &&
       CMath::IsEpsilon(tangentB.GetY(), 0.f, 1.e-5f) &&
       CMath::IsEpsilon(nextTangentA.GetY(), 0.f, 1.e-5f))) {
    return true;
  }
  return false;
}

void CMayaSpline::FindSegmentExtrema(
    int knotIndex, rstl::reserved_vector< rstl::pair< float, float >, 2 >& extrema) const {
  typedef rstl::pair< float, float > Point;
  if (mKnots[knotIndex].GetTangentModeB() == 3) {
    extrema.push_back(Point(mKnots[knotIndex].GetTime(), mKnots[knotIndex].GetAmplitude()));
    extrema.push_back(Point(mKnots[knotIndex + 1].GetTime(), mKnots[knotIndex].GetAmplitude()));
  } else if (mKnots[knotIndex].GetTangentModeB() == 0) {
    extrema.push_back(Point(mKnots[knotIndex].GetTime(), mKnots[knotIndex].GetAmplitude()));
    const CMayaSplineKnot& next = mKnots[knotIndex + 1];
    extrema.push_back(Point(next.GetTime(), next.GetAmplitude()));
  } else {
    rstl::reserved_vector< CVector2f, 4 > points;
    FindControlPoints(knotIndex, points);
    float coefs[4];
    CalculateHermiteCoefficients(points, coefs);
    float rootA = 0.f;
    float rootB = 0.f;
    bool found = CMath::SolveQuadratic(3.f * coefs[0], 2.f * coefs[1], coefs[2], rootA, rootB);
    const float start = mKnots[knotIndex].GetTime();
    const float end = mKnots[knotIndex + 1].GetTime();
    if (found) {
      rootA += start;
      rootB += start;
      if (CMath::IsEpsilon(rootA, start, 0.002f)) {
        rootA = start;
      }
      if (CMath::IsEpsilon(rootA, end, 0.002f)) {
        rootA = end;
      }
      if (CMath::IsEpsilon(rootB, start, 0.002f)) {
        rootB = start;
      }
      if (CMath::IsEpsilon(rootB, end, 0.002f)) {
        rootB = end;
      }
      if (rootA >= start && rootA <= end) {
        extrema.push_back(Point(rootA, EvaluateAt(rootA)));
      }
      if (rootB >= start && rootB <= end) {
        extrema.push_back(Point(rootB, EvaluateAt(rootB)));
      }
    }
    rstl::sort(extrema.begin(), extrema.end());
  }
}

float CMayaSpline::FindFirstIntersection(float amplitude) const {
  if (mKnots.size() < 2) {
    return -1.f;
  }
  rstl::reserved_vector< float, 3 > intersections;
  for (int i = 0; i < mKnots.size() - 1; ++i) {
    intersections.clear();
    FindSegmentIntersections(amplitude, i, intersections);
    if (!intersections.empty()) {
      return intersections.front();
    }
  }
  return -1.f;
}

float CMayaSpline::FindLastIntersection(float amplitude) const {
  if (mKnots.size() < 2) {
    return -1.f;
  }
  rstl::reserved_vector< float, 3 > intersections;
  for (int i = mKnots.size() - 2; i >= 0; --i) {
    intersections.clear();
    FindSegmentIntersections(amplitude, i, intersections);
    if (!intersections.empty()) {
      return intersections.back();
    }
  }
  return -1.f;
}

rstl::pair< float, float > CMayaSpline::FindMaximumAmplitude() const {
  rstl::pair< float, float > result(0.f, 0.f);
  if (mKnots.size() < 2) {
    return result;
  }
  rstl::reserved_vector< rstl::pair< float, float >, 2 > extrema;
  for (int i = 0; i < mKnots.size() - 1; ++i) {
    extrema.clear();
    FindSegmentExtrema(i, extrema);
    if (!extrema.empty()) {
      for (int j = 0; j < extrema.size(); ++j) {
        float amplitude = CMath::AbsF(extrema[j].second);
        if (result.second < amplitude) {
          result.first = extrema[j].first;
          result.second = amplitude;
        }
      }
    }
  }
  return result;
}
#endif

