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
    new (ptr) CAnimSource(in, store);
#else
  case kF_AnimSource:
    new (ptr) CAnimSource(in);
#endif

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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
: mFormatUnion(in, store), x68_(0.f, 0.f, 0.f), mTag(tag) {}
#else
: mFormatUnion(in, store), x88_(0.f, 0.f, 0.f), mTag(tag) {}
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
const CFactoryFnReturn AnimSourceFactory(const SObjectTag& tag, CInputStream& in,
#else
CFactoryFnReturn AnimSourceFactory(const SObjectTag& tag, CInputStream& in,
#endif

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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                                    const CCharAnimTime& time) {
#else
                                    const CCharAnimTime& time, const CAnimPOIData* poiData) {
#endif

  switch (tok->GetType()) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  case 0:
    return rs_new CAnimSourceReader(tok, time);
  case 2:
    return rs_new CFBStreamedAnimReader(tok, time);
#else
  case CAnimFormatUnion::kF_AnimSource:
    return rs_new CAnimSourceReader(TSubAnimTypeToken< CAnimSource >(tok), time, poiData);
  case CAnimFormatUnion::kF_FBStreamedCompression:
    return rs_new CFBStreamedAnimReader(TSubAnimTypeToken< CFBStreamedCompression >(tok), time,
                                        poiData);
#endif

  default:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    return rs_new CFBStreamedAnimReader(tok, time);
#else
    return rs_new CFBStreamedAnimReader(TSubAnimTypeToken< CFBStreamedCompression >(tok), time,
                                        poiData);
#endif

  }
}

CAllFormatsAnimSource::~CAllFormatsAnimSource() {}

