#include "rstl/RstlVersions.h"
#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CHalfTransition.hpp"

#include "Kyoto/Animation/CMetaTransFactory.hpp"
#include "Kyoto/Animation/IMetaTrans.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

CHalfTransition::CHalfTransition(CInputStream& in)
#if RSTL_CORRUPTION
: mVersion(in.Get< uchar >())
, mToAnim(in.Get< CAssetId >())
, mMetaTrans(CMetaTransFactory::CreateMetaTrans(in)) {}

const CAssetId& CHalfTransition::GetToAnimId() const { return mToAnim; }

const rstl::rc_ptr< IMetaTrans >& CHalfTransition::GetMetaTrans() const { return mMetaTrans; }
#else
: mId(in.Get< uint >()), mTrans(CMetaTransFactory::CreateMetaTrans(in)) {}
#endif
