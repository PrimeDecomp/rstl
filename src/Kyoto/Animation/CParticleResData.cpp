#include "Kyoto/Animation/CParticleResData.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

CParticleResData::CParticleResData(CInputStream& in, ushort tableCount)
: mPart(in), mSwhc(in), mElscA(in), mSpsc(in), mSrsc(in), mElscB(in) {}

