#ifndef _CMATH
#define _CMATH
#include "rstl/RstlVersions.h"

#include "types.h"

#include "float.h"
#include "math.h"
#include "stdlib.h"

#include "Kyoto/Math/CVector3f.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_PIF
#define M_PIF 3.14159265358979323846f
#endif
#ifndef M_2PIF
#define M_2PIF 6.28318530718f
#endif

class CMath {
public:
  // The type name is exported in the MP2 Wii SEL; the value names are guesses.
  enum EEaseTypes { kET_Sinusoidal, kET_Quadratic };

  static float EaseInOut(float t, EEaseTypes type, float easeInEnd, float easeOutStart, float start,
                         float end, float slope);
#if !RSTL_PRIME1
  static float PhongBlob(float t, float exponent);
  static bool SolveQuadratic(float a, float b, float c, float& plus, float& minus);
  // Reconstructed solver names; coefficients are ordered from constant to leading term.
  static uint SolveCubic(const float* coefficients, float* roots);
  static uint SolveQuartic(const float* coefficients, float* roots);
  static int SolveCubicDouble(double c0, double c1, double c2, double c3, double epsilon,
                              double* roots);
#endif

  static float FastFmod(float x, float y) {
    int v = static_cast< int >(x * (1.f / y));
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1 && RSTL_VERSION >= RSTL_R3IJ
    return x - static_cast< float >(v * y);
#else
    return x - v * y;
#endif
  }
  template < typename T >
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static const T& Clamp(const T& min, const T& val, const T& max);
#else
  static T Clamp(const T min, const T val, const T max) {
    return min > val ? min : max < val ? max : val;
  }
#if RSTL_VERSION >= RSTL_R32J
  static float Clamp(const float min, const float val, const float max) {
    return FastClamp(min, val, max);
  }
#endif
  static inline int ClampI(int min, int val, int max) {
    return val < min ? min : (val <= max ? val : max);
  }
#endif

  static float SqrtF(float v);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static float Limit(float v, float h) { return fabs(v) > h ? h * Sign(v) : v; }
#else
  static float Limit(const float v, const float h) {
#if RSTL_VERSION >= RSTL_R32J
    return FastMin(FastMax(-h, v), h);
#else
    return AbsF(v) > h ? h * Sign(v) : v;
#endif
  }
#endif

  static float Sign(float v) { return FastFSel(v, 1.f, -1.f); }
#ifdef __MWERKS__
  static float FastFSel(register float v, register float h, register float l) {
    register float out;
    asm {
      fsel out, v, h, l
    }
    return out;
  }
#else
  static float FastFSel(float v, float h, float l) { return v >= 0.f ? h : l; }
#endif
  static float AbsF(float v) { return fabs(v); }
  static double AbsD(double v) { return fabs(v); }
  static int AbsI(int v) { return abs(v); }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static float WrapPi(float rad) {
    rad = FastFmod(rad, M_2PIF);
#else
  // Guessed name
  static bool IsFinite(float v) {
    const int c = fpclassify(v);
    return c == FP_ZERO || c == FP_NORMAL || c == FP_SUBNORMAL;
  }
  static float WrapPi(float x) {
    float rad = FastFmod(x, M_2PIF);
#endif

    if (rad > M_PIF) {
      rad = rad - M_2PIF;
    } else if (rad < -M_PIF) {
      rad = M_2PIF + rad;
    }
    return rad;
  }
#if !RSTL_PRIME1
  static float WrapTwoPi(float rad) {
    if (rad > M_2PIF) {
      rad = FastFmod(rad, M_2PIF);
    } else if (rad < 0.f) {
      rad = M_2PIF + FastFmod(rad, M_2PIF);
    }
    return rad;
  }
#endif

  template < typename T >
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static const T& Min(const T& a, const T& b);
#else
  static T Min(const T a, const T b) {
    return a < b ? a : b;
  }
#endif

  template < typename T >
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static const T& Max(const T& a, const T& b);
#else
  static T Max(const T a, const T b) {
    return b < a ? a : b;
  }
#if RSTL_VERSION >= RSTL_R32J
  static float Max(const float a, const float b) { return FastMax(a, b); }
#endif
#endif

  static float InvSqrtF(float x);
  static float FastArcCosR(float x);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static float SlowCosineR(float x);
  static float SlowSineR(float x);
  static float SlowTangentR(float x);
#else
  static float SlowCosineR(float x) { return cos(x); }
  static float SlowSineR(float x) { return sin(x); }
  static float SlowTangentR(float x) { return tan(x); }
#endif

  static float FastSinR(float x);
  static float FastCosR(float x);
  static float ClampRadians(float rad) {
    float value = FastFmod(rad, M_2PIF);
    if (value < 0.f) {
      value += M_2PIF;
    }
    return value;
  }
  static float ModF(float x, float y) { return static_cast< float >(fmod(x, y)); }
  static float Deg2Rad(float deg) { return Deg2Rev(deg) * M_2PIF; }
  static float Deg2Rev(float deg) { return deg * (1.f / 360.f); }
  static float ArcSineR(float v);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static float ArcCosineR(float v);
#else
  static float ArcCosineR(float v) { return acosf(v); }
#endif

  static float ArcTangentR(float v);
  static float PowF(float x, float y);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static const float FloorF(float x);
#else
  static const float FloorF(float x) { return floor(x); }
#endif

  static float CeilingF(float x);
  static CVector3f GetCatmullRomSplinePoint(const CVector3f& a, const CVector3f& b,
                                            const CVector3f& c, const CVector3f& d, float t);
  static float GetCatmullRomSplinePoint(float a, float b, float c, float d, float t);
#if !RSTL_PRIME1
  // Guessed names, based on the native spline basis and normalized-bisector formulas.
  static CVector3f GetHermiteSplinePoint(const CVector3f& a, const CVector3f& b,
                                         const CVector3f& tangentA, const CVector3f& tangentB,
                                         float t);
  static CVector3f GetHermiteSplineTangent(const CVector3f& a, const CVector3f& b,
                                           const CVector3f& tangentA, const CVector3f& tangentB,
                                           float t);
  static CVector3f GetCatmullRomSplineTangent(const CVector3f& a, const CVector3f& b,
                                              const CVector3f& c, const CVector3f& d, float t);
  static CVector3f GetBSplinePoint(const CVector3f& a, const CVector3f& b, const CVector3f& c,
                                   const CVector3f& d, float t);
  static CVector3f GetBSplineTangent(const CVector3f& a, const CVector3f& b, const CVector3f& c,
                                     const CVector3f& d, float t);
  static CVector3f GetBezierTangent(const CVector3f& a, const CVector3f& b, const CVector3f& c,
                                    const CVector3f& d, float t);
  static CVector3f GetRoundedCatmullRomSplinePoint(const CVector3f& a, const CVector3f& b,
                                                   const CVector3f& c, const CVector3f& d, float t);
  static CVector3f GetRoundedCatmullRomSplineTangent(const CVector3f& a, const CVector3f& b,
                                                     const CVector3f& c, const CVector3f& d,
                                                     float t);
#endif

  static CVector3f GetBezierPoint(const CVector3f&, const CVector3f&, const CVector3f&,
                                  const CVector3f&, float);
  static CVector3f BaryToWorld(const CVector3f& p0, const CVector3f& p1, const CVector3f& p2,
                               const CVector3f& bary);
  // GetCatmullRomSplinePoint__5CMathFRC9CVector3fRC9CVector3fRC9CVector3fRC9CVector3ff global
#ifdef __MWERKS__
#if !RSTL_PRIME1
  static inline float FastInvSqrtF(float x) { return __frsqrte(x); } // Guessed name
#endif
  static inline float FastSqrtF(register float x) {
    if (x == 0.f) {
      return 0.f;
    }
    return x * __frsqrte(x);
  }
#else
#if !RSTL_PRIME1
  static inline float FastInvSqrtF(float x) { return 1.f / sqrtf(x); } // Guessed name
#endif
  static inline float FastSqrtF(float x) { return sqrtf(x); }
#endif
#if !RSTL_PRIME1
  // Noise3d is a Wii SEL export; the other arities are guessed from its call pattern.
  static float Noise1d(float x);
  static float Noise2d(float x, float y);
  static float Noise3d(float x, float y, float z);
  static float Noise4d(float x, float y, float z, float w);
#endif

  static double SqrtD(double x);
  static bool IsEpsilon(float x, float y, float epsilon) { return AbsF(x - y) < epsilon; }

  static float FastMin(float a, float b) { return FastFSel(a - b, b, a); }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static float FastMax(float a, float b) {
    a = FastFSel(a - b, a, b);
    return a;
  }
#else
  static float FastMax(float a, float b) { return FastFSel(a - b, a, b); }
#endif

#if RSTL_PRIME1
  static float FastClamp(float min, float val, float max) {
    min = FastMax(min, val);
    float diff = min - max;
    return FastFSel(diff, max, min);
  }
#else
  static float FastClamp(const float& min, const float& val, const float& max) {
    float v = FastMax(min, val);
    float diff = v - max;
    return FastFSel(diff, max, v);
  }
#endif
#if RSTL_PRIME1
  static float FastLimit(float v, float h) {
    float low = -h;
    float diff = low - v;
    float limited = FastFSel(diff, low, v);
    return FastFSel(limited - h, h, limited);
  }
#endif

  // PowF__5CMathFff global
  // Rev2Deg__5CMathFf weak
  // SlowTangentR__5CMathFf global
  static float Rad2Deg(float rad) { return rad * (180.f / M_PIF); }
  static float Rad2Rev(float rad) { return rad * (1.f / M_2PIF); }
  // CeilingF__5CMathFf global
  // ArcTangentR__5CMathFf global
  template < typename T >
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static void Swap(T& a, T& b) {
    T tmp = a;
    a = b;
    b = tmp;
  }
#else
  static void Swap(T& a, T& b);
#endif

  static int FloorPowerOfTwo(int v);
#if !RSTL_PRIME1
  // Target-derived name: returns the base-two exponent, with zero mapped to zero.
  static int FloorLog2(uint v);
#endif

};

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
template < typename T >
const T& CMath::Clamp(const T& min, const T& val, const T& max) {
  return min > val ? min : max < val ? max : val;
}

template < typename T >
const T& CMath::Min(const T& a, const T& b) {
  return a < b ? a : b;
}

template < typename T >
const T& CMath::Max(const T& a, const T& b) {
  return a > b ? a : b;
}
#elif RSTL_VERSION >= RSTL_R32J
template <>
inline float CMath::Min< float >(const float a, const float b) {
  return FastMin(a, b);
}
#endif

#endif // _CMATH
