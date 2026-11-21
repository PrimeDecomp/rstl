#ifndef _CSFXPITCHBEND
#define _CSFXPITCHBEND
#include "rstl/RstlVersions.h"

#include "rstl/static_assert.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/Audio/CAudioHandle.hpp"


#else
#include "Kyoto/Audio/CSfxHandle.hpp"
#endif

// Guessed name: a timed pitch transition owned by CSfxManager.
class CSfxPitchBend {
public:
#if RSTL_CORRUPTION
  CSfxPitchBend(const CAudioHandle& handle, ushort start, ushort target, float duration);
#else
  CSfxPitchBend(const CSfxHandle& handle, ushort start, ushort target, float duration);
#endif
  // Guessed method names, supported by the manager's update/apply/retire sequence.
  void Update(float dt);
  bool IsFinished() const;
#if RSTL_CORRUPTION
  const CAudioHandle& GetHandle() const { return mHandle; }
#else
  const CSfxHandle& GetHandle() const { return mHandle; }
#endif
  ushort GetPitch() const { return mPitch; }

private:
#if RSTL_CORRUPTION
  CAudioHandle mHandle;
#else
  CSfxHandle mHandle;
#endif
  ushort mPitch;
  ushort mTargetPitch;
  float mTimeRemaining;
};
CHECK_SIZEOF(CSfxPitchBend, 0xc)

#endif // _CSFXPITCHBEND
