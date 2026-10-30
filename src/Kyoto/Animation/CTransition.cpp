#include "rstl/RstlVersions.h"
#include "Kyoto/Streams/CInputStream.hpp"

#include "Kyoto/Animation/CTransition.hpp"

#include "Kyoto/Animation/CMetaTransFactory.hpp"
#include "Kyoto/Animation/IMetaTrans.hpp"

CTransition::CTransition(CInputStream& in)
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
: mId(in.Get<uint>())
, mAnimA(in.Get<uint>())
, mAnimB(in.Get<uint>())
, mTrans(CMetaTransFactory::CreateMetaTrans(in)) {}
#else
: mId(in.Get< uint >()), mAnimPair(in), mTrans(CMetaTransFactory::CreateMetaTrans(in)) {}
#endif

