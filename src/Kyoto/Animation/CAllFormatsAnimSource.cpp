#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CAllFormatsAnimSource.hpp"

#include "Kyoto/Animation/CAnimSourceReader.hpp"
#include "Kyoto/Animation/CFBStreamedAnimReader.hpp"

#include "Kyoto/CVParamTransfer.hpp"
#include "Kyoto/IObjectStore.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

CAnimFormatUnion::CAnimFormatUnion(CInputStream& in, IObjectStore& store) {
  mFormatType = in.Get< uint >();
  SubConstruct(mFormatData, mFormatType, in, store);
}

CAnimFormatUnion::~CAnimFormatUnion() {
  switch (mFormatType) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  case 0:
#else
  case kF_AnimSource:
#endif
    reinterpret_cast< CAnimSource* >(mFormatData)->~CAnimSource();
    break;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  case 2:
#else
  case kF_FBStreamedCompression:
#endif
    reinterpret_cast< CFBStreamedCompression* >(mFormatData)->~CFBStreamedCompression();
    break;
  default:
    break;
  }
}

void CAnimFormatUnion::SubConstruct(uchar* ptr, const uint format, CInputStream& in,
                                    IObjectStore& store) {
  switch (format) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  case 0:
#else
  case kF_AnimSource:
#endif
    new (ptr) CAnimSource(in
#if RSTL_PRIME1
                          , store
#endif
    );
    break;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  case 2:
#else
  case kF_FBStreamedCompression:
#endif
    new (ptr) CFBStreamedCompression(in, store);
    break;
  default:
    break;
  }
}

CAllFormatsAnimSource::CAllFormatsAnimSource(CInputStream& in, IObjectStore& store,
                                             const SObjectTag& tag)
: mFormatUnion(in, store)
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
, x68_(0.f, 0.f, 0.f)
#else
, x88_(0.f, 0.f, 0.f)
#endif
, mTag(tag) {}

// TODO: verify that this Prime/Echoes difference is real
RSTL_PRIME1_CONST CFactoryFnReturn AnimSourceFactory(const SObjectTag& tag, CInputStream& in,
                                   const CVParamTransfer& param) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const rstl::rc_ptr< IVParamObj > obj = param.mObj;
#else
  const rstl::rc_ptr< IVParamObj > obj = param.GetObj();
#endif
  IObjectStore* pool = static_cast< TObjOwnerParam< IObjectStore* >* >(obj.GetPtr())->GetData();

  return rs_new CAllFormatsAnimSource(in, *pool, tag);
}

rstl::ownership_transfer< IAnimReader >
CAllFormatsAnimSource::GetNewReader(const TLockedToken< CAllFormatsAnimSource >& tok,
                                    const CCharAnimTime& time
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
                                    , const CAnimPOIData* poiData
#endif
                                    ) {
  switch (tok->GetType()) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  case 0:
#else
  case CAnimFormatUnion::kF_AnimSource:
#endif
    return rs_new CAnimSourceReader(
#if RSTL_PRIME1
        tok
#else
        TSubAnimTypeToken< CAnimSource >(tok)
#endif
        , time
#if !RSTL_PRIME1
        , poiData
#endif
    );
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  case 2:
#else
  case CAnimFormatUnion::kF_FBStreamedCompression:
#endif
    return rs_new CFBStreamedAnimReader(
#if RSTL_PRIME1
        tok
#else
        TSubAnimTypeToken< CFBStreamedCompression >(tok)
#endif
        , time
#if !RSTL_PRIME1
        , poiData
#endif
    );

  default:
    return rs_new CFBStreamedAnimReader(
#if RSTL_PRIME1
        tok
#else
        TSubAnimTypeToken< CFBStreamedCompression >(tok)
#endif
        , time
#if !RSTL_PRIME1
        , poiData
#endif
    );
  }
}

CAllFormatsAnimSource::~CAllFormatsAnimSource() {}

