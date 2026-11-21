#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/RstlVersions.h"
#include "Kyoto/Streams/CInputStream.hpp"

#endif
#include "Kyoto/Animation/CTransition.hpp"

#include "Kyoto/Animation/CMetaTransFactory.hpp"
#include "Kyoto/Animation/IMetaTrans.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/Streams/CInputStream.hpp"


#endif

CTransition::CTransition(CInputStream& in)
#if RSTL_CORRUPTION
: mVersion(in.Get< uchar >())
, mFromAnim(in.Get< CAssetId >())
, mToAnim(in.Get< CAssetId >())
, mMetaTrans(CMetaTransFactory::CreateMetaTrans(in)) {}
#else
: mId(in.Get< uint >())
#endif
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_CORRUPTION
const rstl::rc_ptr< IMetaTrans >& CTransition::GetMetaTrans() const { return mMetaTrans; }

const CAssetId& CTransition::GetFromAnimId() const { return mFromAnim; }

const CAssetId& CTransition::GetToAnimId() const { return mToAnim; }
#else
#if RSTL_PRIME1
, mAnimA(in.Get< uint >())
, mAnimB(in.Get< uint >())
#else
, mAnimPair(in)
#endif
, mTrans(CMetaTransFactory::CreateMetaTrans(in)) {}
#endif
