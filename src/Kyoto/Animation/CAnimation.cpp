#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CAnimation.hpp"

#include "Kyoto/Animation/CMetaAnimFactory.hpp"
#include "Kyoto/Animation/IMetaAnim.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CAnimation::CAnimation(CInputStream& in)
: mName(in)
, mAnim(CMetaAnimFactory::CreateMetaAnim(in)) {

}
#else
CAnimation::CAnimation(CInputStream& in) : mName(in), mAnim(CMetaAnimFactory::CreateMetaAnim(in)) {}
#endif

