#ifndef _CPARTICLEGLOBALS
#define _CPARTICLEGLOBALS
#include "rstl/RstlVersions.h"

#include "types.h"

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Particles/CElementGen.hpp"

class CParticleGlobals {
public:
#if !RSTL_PRIME1
  // G2ME01 pushes this on the stack for the duration of a system update and restores the
  // previous system when it goes out of scope.
  struct SParticleSystem;
  friend struct SParticleSystem;
#endif

  struct SParticleSystem {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    FourCC mType;
    CElementGen* mSystem;
    SParticleSystem* mPrev;

    SParticleSystem(FourCC type, CElementGen* system)
#else
    SParticleSystem(FourCC type, CParticleGen* system)
#endif
    : mType(type)
    , mSystem(system)
    , mPrev(mCurrentParticleSystem) {
      mCurrentParticleSystem = this;
    }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION < RSTL_R3IJ
    ~SParticleSystem() { mCurrentParticleSystem = mPrev; }
#endif
#else
    ~SParticleSystem() { mCurrentParticleSystem = mPrev; }

    FourCC mType;
    CParticleGen* mSystem;
    SParticleSystem* mPrev;
#endif

  };

  static void SetEmitterTime(int time);
  static void SetParticleLifetime(int lifetime);
  static void UpdateParticleLifetimeTweenValues(int time);

  static int GetParticleLifetime() { return mParticleLifetime; }
  static float GetParticleLifetimeReal() { return mParticleLifetimeReal; }
  static int GetEmitterTime() { return mEmitterTime; }
  static float GetEmitterTimeReal() { return mEmitterTimeReal; }
  static int GetParticleLifetimePercentage() { return mParticleLifetimePercentage; }
  static float GetParticleLifetimePercentageReal() { return mParticleLifetimePercentageReal; }
  static float GetParticleLifetimePercentageRemainder() {
    return mParticleLifetimePercentageRemainder;
  }
  static CElementGen::CParticle* GetCurrentParticle() { return mCurrentParticle; }
  static float* GetParticleAccessParameters() { return mParticleAccessParameters; }
  static SParticleSystem* GetCurrentParticleSystem() { return mCurrentParticleSystem; }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static void SetCurrentParticleSystem(SParticleSystem* system) { mCurrentParticleSystem = system; }
#else
  static void SetCurrentParticle(CElementGen::CParticle* particle) { mCurrentParticle = particle; }
  static void SetParticleAccessParameters(float* parameters) {
    mParticleAccessParameters = parameters;
  }
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
public:
#else
private:
#endif

  static int mParticleLifetime;
  static float mParticleLifetimeReal;
  static int mEmitterTime;
  static float mEmitterTimeReal;
  static int mParticleLifetimePercentage;
  static float mParticleLifetimePercentageReal;
  static float mParticleLifetimePercentageRemainder;
  static CElementGen::CParticle* mCurrentParticle;
  static float* mParticleAccessParameters;
  static SParticleSystem* mCurrentParticleSystem;
};

#endif // _CPARTICLEGLOBALS
