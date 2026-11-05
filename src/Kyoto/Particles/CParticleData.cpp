#include "rstl/RstlVersions.h"
#include "Kyoto/Particles/CParticleData.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

#if !RSTL_KYOTO_PARTICLE_DATA_PROTOTYPE
CParticleData::CParticleData(CInputStream& in)
: mDuration(in.Get(TGetType(mDuration)))
, mParticle(in)
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
, mBoneName(in.Get(TGetType(mBoneName)))
#else
, mBone(in)
#endif

, mScale(in.Get(TGetType(mScale)))
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
, mParentMode(in.Get(TGetType(mParentMode)))
#else
, mParentMode(EParentedMode(in.Get< uint >()))
#endif
{}
#endif
