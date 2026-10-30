#include "rstl/RstlVersions.h"
#include "Kyoto/Particles/CParticleGen.hpp"

#if !RSTL_PRIME1
#include "Kyoto/Particles/CElementGen.hpp"

// Wii exports corroborate the member names; GameCube readers establish their roles.
bool CElementGen::sMoveRedToAlphaBuffer;
uint CParticleGen::sDrawFlags;
uint CParticleGen::sDrawMask;
#endif

void CParticleGen::AddModifier(CWarp* warp) {
  mModifiersList.push_back(warp);
}

