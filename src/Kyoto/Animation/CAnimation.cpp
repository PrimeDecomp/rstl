#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CAnimation.hpp"

#if !RSTL_PRIME1
#include "Kyoto/Animation/CMetaAnimFactory.hpp"
#include "Kyoto/Animation/IMetaAnim.hpp"
#endif

#include "Kyoto/Streams/CInputStream.hpp"
#if RSTL_PRIME1
#include "Kyoto/Animation/CMetaAnimFactory.hpp"
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CAnimation::CAnimation(CInputStream& in)
: mName(in)
, mAnim(CMetaAnimFactory::CreateMetaAnim(in)) {

}
#else
CAnimation::CAnimation(CInputStream& in) : mName(in), mAnim(CMetaAnimFactory::CreateMetaAnim(in)) {}
#endif

