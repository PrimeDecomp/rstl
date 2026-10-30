#include "rstl/RstlVersions.h"
#if !RSTL_PRIME1
// Make the pair stream constructor visible before instantiating the member type.
#include "Kyoto/Streams/CInputStream.hpp"
#endif

#include "Kyoto/Animation/CTransition.hpp"

#include "Kyoto/Animation/CMetaTransFactory.hpp"
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#include "Kyoto/Streams/CInputStream.hpp"
#else
#include "Kyoto/Animation/IMetaTrans.hpp"
#endif

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

