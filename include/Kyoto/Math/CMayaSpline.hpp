#ifndef _CMAYASPLINE
#define _CMAYASPLINE
#include "rstl/RstlVersions.h"
#if RSTL_PRIME1
#include "types.h"
#endif

#include "Kyoto/Math/CAbsAngle.hpp"
#include "Kyoto/Math/CVector2f.hpp"

#if !RSTL_PRIME1
#include "rstl/pair.hpp"
#endif

#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_R3IJ

class CMayaSplineKnot {
public:
  // Enum spellings inferred from the tangent calculations.
  enum ETangentType {
    kTT_Linear,
    kTT_Flat,
    kTT_Smooth,
    kTT_Step,
    kTT_Clamped,
    kTT_Fixed,
    kTT_Invalid = 255
  };

  CMayaSplineKnot(float time, float amplitude, ETangentType inTangentType,
                  ETangentType outTangentType, const CAbsAngle& inAngle = CAbsAngle::FromRadians(0.f),
                  const CAbsAngle& outAngle = CAbsAngle::FromRadians(0.f));

  float GetTime() const { return mTime; }
  float GetAmplitude() const { return mAmplitude; }
  ETangentType GetInTangentType() const { return static_cast< ETangentType >(mInTangentType); }
  ETangentType GetOutTangentType() const { return static_cast< ETangentType >(mOutTangentType); }
  void GetTangents(const CMayaSplineKnot* previous, const CMayaSplineKnot* next,
                   CVector2f& tangentA, CVector2f& tangentB) const;
  bool operator<(const CMayaSplineKnot& other) const { return GetTime() < other.GetTime(); }

private:
  void CalculateTangents(const CMayaSplineKnot* previous, const CMayaSplineKnot* next) const;

  float mTime;
  float mAmplitude;
  mutable uint mInTangentType : 8;
  mutable uint mOutTangentType : 8;
  mutable uint mDirty : 1;
  mutable CVector2f mCachedTangentA;
  mutable CVector2f mCachedTangentB;
};
CHECK_SIZEOF(CMayaSplineKnot, 0x1c)

namespace rstl {
RSTL_DECLARE_TRIVIALLY_DESTRUCTIBLE(CMayaSplineKnot)
RSTL_DECLARE_DIRECT_CONSTRUCTION(CMayaSplineKnot)
} // namespace rstl

class CMayaSpline {
public:
  // Enum spellings inferred; values follow the original Maya infinity modes.
  enum EInfinityType { kIT_Constant, kIT_Linear, kIT_Cycle, kIT_CycleRelative, kIT_Oscillate };
  enum EClampMode { kCM_None, kCM_Clamp, kCM_Wrap };

  CMayaSpline();
  CMayaSpline(const CMayaSpline& other);
  CMayaSpline(const rstl::vector< CMayaSplineKnot >& knots, float minAmplitude,
              float maxAmplitude, EClampMode clampMode, EInfinityType preInfinity,
              EInfinityType postInfinity);
  void operator=(const CMayaSpline& other);

  float EvaluateAt(float time) const;
  uint GetKnotCount() const { return mKnots.size(); }
  const rstl::vector< CMayaSplineKnot >& GetKnots() const { return mKnots; }
  float GetMinTime() const { return mKnots.front().GetTime(); }
  float GetMaxTime() const { return mKnots.back().GetTime(); }
  float GetDuration() const { return GetMaxTime() - GetMinTime(); }

  static CMayaSpline BuildLinearSpline(float timeA, float amplitudeA, float timeB,
                                       float amplitudeB);
  // Factory spelling inferred from its callers in CTweakPlayerControl.
  static CMayaSpline BuildSpline(const CMayaSplineKnot* knots, uint count, EClampMode clampMode,
                                 EInfinityType preInfinity, EInfinityType postInfinity,
                                 float minAmplitude, float maxAmplitude);

private:
  float EvaluateHermite(float time) const;
  float EvaluateInfinities(float time, bool preInfinity) const;
  bool FindKnot(float time, int& knotIndex) const;
  float EvaluateAtUnclamped(float time) const;
  void FindControlPoints(int knotIndex, rstl::reserved_vector< CVector2f, 4 >& points) const;
  void CalculateHermiteCoefficients(const rstl::reserved_vector< CVector2f, 4 >& points,
                                     float* coefficients) const;

  EInfinityType mPreInfinity;
  EInfinityType mPostInfinity;
  rstl::vector< CMayaSplineKnot > mKnots;
  EClampMode mClampMode;
  float mMinAmplitude;
  float mMaxAmplitude;
  struct SCache {
    SCache()
    : mKnotIndex(-1), mSegmentIndex(-1), mStep(false), mMinTime(0.f) {}

    int mKnotIndex;
    int mSegmentIndex;
    bool mStep : 1;
    float mMinTime;
    float mHermiteCoefficients[4];
  };
  mutable SCache mCache;
};
CHECK_SIZEOF(CMayaSpline, 0x40)

#endif
#else
class CMayaSplineKnot {
  float mTime;
  float mAmplitude;
  mutable uint mFlagA : 8;
  mutable uint mFlagB : 8;
  mutable uint mDirty : 1;
  mutable CVector2f mCachedTangentA;
  mutable CVector2f mCachedTangentB;

public:
  CMayaSplineKnot(CInputStream& in);
  CMayaSplineKnot(float time, float amplitude, int flagA, int flagB,
                  const CAbsAngle& inAngle = CAbsAngle::FromRadians(0.f),
                  const CAbsAngle& outAngle = CAbsAngle::FromRadians(0.f));
  bool operator<(const CMayaSplineKnot& other) const { return mTime < other.mTime; }

  float GetTime() const { return mTime; }
  float GetAmplitude() const { return mAmplitude; }
  int GetTangentModeA() const { return mFlagA; }
  int GetTangentModeB() const { return mFlagB; }
  void GetTangents(const CMayaSplineKnot* prev, const CMayaSplineKnot* next, CVector2f& tangentA,
                   CVector2f& tangentB) const;
  void CalculateTangents(const CMayaSplineKnot* prev, const CMayaSplineKnot* next) const;
};

namespace rstl {
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(CMayaSplineKnot)
} // namespace rstl
namespace rstl {
RSTL_DECLARE_DIRECT_CONSTRUCTION(CMayaSplineKnot)
} // namespace rstl

class CMayaSpline {
  struct SCache {
    SCache() : mKnotIndex(-1), mSegmentIndex(-1), mStepSegment(false), mMinTime(0.f) {}

    int mKnotIndex;
    int mSegmentIndex;
    bool mStepSegment : 1;
    float mMinTime;
    float mHermiteCoefs[4];
  };

  int mPreInfinity;
  int mPostInfinity;
  rstl::vector< CMayaSplineKnot > mKnots;
  int mClampMode;
  float mMinAmplitude;
  float mMaxAmplitude;
  mutable SCache mCache;

public:
  CMayaSpline();
  CMayaSpline(const rstl::vector< CMayaSplineKnot >& knots, int clampMode, int preInfinity,
              int postInfinity, float minAmplitude, float maxAmplitude);
  CMayaSpline(CInputStream& in);
#if RSTL_VERSION >= RSTL_R32J
  CMayaSpline(const CMayaSpline& other);
  void operator=(const CMayaSpline& other);
#endif
  ~CMayaSpline() {}

  static CMayaSpline CreateFor(float timeA, float amplitudeA, float timeB, float amplitudeB);

  size_t GetKnotCount() const;
  const rstl::vector< CMayaSplineKnot >& GetKnots() const;
  float GetMaxTime() const;
  float GetDuration() const;

  rstl::pair< float, float > FindMaximumAmplitude() const;
  float FindFirstIntersection(float amplitude) const;
  float FindLastIntersection(float amplitude) const;
  void FindIntersections(float amplitude, rstl::reserved_vector< float, 8 >& intersections) const;
  rstl::reserved_vector< float, 8 >
  FilterLeftIntersections(const rstl::reserved_vector< float, 8 >& intersections) const;
  rstl::reserved_vector< float, 8 >
  FilterRightIntersections(const rstl::reserved_vector< float, 8 >& intersections) const;
  bool IsSegmentConstant(int knotIndex) const;
  void FindSegmentExtrema(int knotIndex,
                          rstl::reserved_vector< rstl::pair< float, float >, 2 >& extrema) const;
  void FindSegmentIntersections(float amplitude, int knotIndex,
                                rstl::reserved_vector< float, 3 >& intersections) const;

  float EvaluateAt(float time) const;
  float EvaluateAtUnclamped(float time) const;
  float EvaluateInfinities(float time, bool Pre) const;
  float EvaluateHermite(float time) const;
  bool FindKnot(float time, int& knotIndex) const;
  void FindControlPoints(int knotIndex, rstl::reserved_vector< CVector2f, 4 >& controlPoints) const;
  void CalculateHermiteCoefficients(const rstl::reserved_vector< CVector2f, 4 >& controlPoits,
                                    float* coefs) const;
};

// Compatibility name used by generated script-loader declarations.
typedef CMayaSpline SLdrSpline;

CHECK_SIZEOF(CMayaSplineKnot, 0x1c)
CHECK_SIZEOF(CMayaSpline, 0x44)
CHECK_SIZEOF(SLdrSpline, 0x44)
#endif

#endif // _CMAYASPLINE
