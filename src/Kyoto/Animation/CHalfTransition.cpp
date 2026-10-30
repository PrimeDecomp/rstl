#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CHalfTransition.hpp"

#include "Kyoto/Animation/CMetaTransFactory.hpp"
#if !RSTL_PRIME1
#include "Kyoto/Animation/IMetaTrans.hpp"
#endif

#include "Kyoto/Streams/CInputStream.hpp"

CHalfTransition::CHalfTransition(CInputStream& in)
: mId(in.Get< uint >()), mTrans(CMetaTransFactory::CreateMetaTrans(in)) {}

