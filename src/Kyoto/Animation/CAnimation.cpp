#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/RstlVersions.h"
#endif
#include "Kyoto/Animation/CAnimation.hpp"

#include "Kyoto/Animation/CMetaAnimFactory.hpp"
#include "Kyoto/Animation/IMetaAnim.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

#if RSTL_CORRUPTION


CAnimation::CAnimation(CInputStream& in) : mName(in), mAnim(CMetaAnimFactory::CreateMetaAnim(in)) {}
#else
CAnimation::CAnimation(CInputStream& in)
: mName(in)
, mAnim(CMetaAnimFactory::CreateMetaAnim(in)) {}
#endif
