#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/RstlVersions.h"
#endif
#include "Kyoto/Particles/CParticleGen.hpp"

#if RSTL_CORRUPTION

// Wii exports corroborate the member names; GameCube readers establish their roles.
// Wii exports corroborate the member names; GameCube readers establish their roles.
uint CParticleGen::sDrawFlags;
uint CParticleGen::sDrawMask;
#else
#if !RSTL_PRIME1
#include "Kyoto/Particles/CElementGen.hpp"

// Wii exports corroborate the member names; GameCube readers establish their roles.
bool CElementGen::sMoveRedToAlphaBuffer;
uint CParticleGen::sDrawFlags;
uint CParticleGen::sDrawMask;
#endif
#endif

#if RSTL_CORRUPTION
void CParticleGen::AddModifier(CWarp* warp) { mModifiersList.push_back(warp); }
#else
void CParticleGen::AddModifier(CWarp* warp) {
  mModifiersList.push_back(warp);
}
#endif
