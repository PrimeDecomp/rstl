#include "rstl/RstlVersions.h"
#include "Kyoto/Streams/CInputStream.hpp"

#include "Kyoto/Animation/CTransition.hpp"

#include "Kyoto/Animation/CMetaTransFactory.hpp"
#include "Kyoto/Animation/IMetaTrans.hpp"

CTransition::CTransition(CInputStream& in)
: mId(in.Get< uint >())
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
, mAnimA(in.Get< uint >())
, mAnimB(in.Get< uint >())
#else
, mAnimPair(in)
#endif
, mTrans(CMetaTransFactory::CreateMetaTrans(in)) {}

