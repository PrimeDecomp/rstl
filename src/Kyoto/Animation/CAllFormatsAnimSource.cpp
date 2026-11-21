#include "rstl/RstlVersions.h"
#if RSTL_CORRUPTION
#define RS_TTOKEN_FILE "TToken.H"

#else
#include "rstl/RstlVersions.h"
#endif
#include "Kyoto/Animation/CAllFormatsAnimSource.hpp"

#if RSTL_CORRUPTION
#include "Kyoto/Alloc/Assert.hpp"
#endif
#include "Kyoto/Animation/CAnimSourceReader.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/Animation/CCompressedAnimSourceReader.hpp"
#endif
#include "Kyoto/Animation/CFBStreamedAnimReader.hpp"

#include "Kyoto/CVParamTransfer.hpp"
#include "Kyoto/IObjectStore.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/Streams/CMemoryInStream.hpp"


#endif

CAnimFormatUnion::CAnimFormatUnion(CInputStream& in, IObjectStore& store) {
  mFormatType = in.Get< uint >();
  SubConstruct(mFormatData, mFormatType, in, store);
}

CAnimFormatUnion::~CAnimFormatUnion() {
  switch (mFormatType) {
#if RSTL_CORRUPTION
  case kF_AnimSource:
    AsCAnimSource().~CAnimSource();
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  case 0:
#else
  case kF_AnimSource:
#endif
    reinterpret_cast< CAnimSource* >(mFormatData)->~CAnimSource();
#endif
    break;
#if RSTL_CORRUPTION
  case kF_CompressedAnimSource:
    AsCCompressedAnimSource().~CCompressedAnimSource();
    break;
  case kF_FBStreamedCompression:
    AsCFBStreamedCompression().~CFBStreamedCompression();
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  case 2:
#else
  case kF_FBStreamedCompression:
#endif
    reinterpret_cast< CFBStreamedCompression* >(mFormatData)->~CFBStreamedCompression();
#endif
    break;
  default:
#if RSTL_CORRUPTION
    RS_VERIFY_THROW(31, false, kException_UnknownType, "Unknown type");
#endif
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
#if RSTL_CORRUPTION
  case kF_CompressedAnimSource:
    new (ptr) CCompressedAnimSource(in);
    break;
  case kF_FBStreamedCompression:
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  case 2:
#else
  case kF_FBStreamedCompression:
#endif
#endif
    new (ptr) CFBStreamedCompression(in, store);
    break;
  default:
#if RSTL_CORRUPTION
    RS_VERIFY_THROW(54, false, kException_UnknownType, "Unknown type");
#endif
    break;
  }
}

CAllFormatsAnimSource::CAllFormatsAnimSource(CInputStream& in, IObjectStore& store,
                                             const SObjectTag& tag)
#if RSTL_CORRUPTION
: mFormatUnion(in, store), x9c_(CVector3f::Zero()), mTag(tag) {}
#else
: mFormatUnion(in, store)
#endif
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_CORRUPTION
CFactoryFnReturn AnimSourceFactory(const SObjectTag& tag, const rstl::auto_ptr< uchar >& buffer,
                                   int size, const CVParamTransfer& param) {
  CMemoryInStream in(buffer.get(), size);
  const rstl::rc_ptr< IVParamObj > obj = param.GetObj();
#else
#if RSTL_PRIME1
, x68_(0.f, 0.f, 0.f)
#else
, x88_(0.f, 0.f, 0.f)
#endif
, mTag(tag) {}

RSTL_PRIME1_CONST CFactoryFnReturn AnimSourceFactory(const SObjectTag& tag, CInputStream& in,
                                   const CVParamTransfer& param) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const rstl::rc_ptr< IVParamObj > obj = param.mObj;
#else
  const rstl::rc_ptr< IVParamObj > obj = param.GetObj();
#endif
#endif
  IObjectStore* pool = static_cast< TObjOwnerParam< IObjectStore* >* >(obj.GetPtr())->GetData();
  return rs_new_line(86) CAllFormatsAnimSource(in, *pool, tag);
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
#if RSTL_CORRUPTION
  case CAnimFormatUnion::kF_AnimSource:
    return rs_new_line(101) CAnimSourceReader(TSubAnimTypeToken< CAnimSource >(tok), time, poiData);
  case CAnimFormatUnion::kF_CompressedAnimSource:
    return rs_new_line(104)
        CCompressedAnimSourceReader(TSubAnimTypeToken< CCompressedAnimSource >(tok), time, poiData);
  case CAnimFormatUnion::kF_FBStreamedCompression:
    return rs_new_line(107)
        CFBStreamedAnimReader(TSubAnimTypeToken< CFBStreamedCompression >(tok), time, poiData);
#else
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

#endif
  default:
#if RSTL_CORRUPTION
    return rs_new_line(113)
        CFBStreamedAnimReader(TSubAnimTypeToken< CFBStreamedCompression >(tok), time, poiData);
#else
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
#endif
  }
}

CAllFormatsAnimSource::~CAllFormatsAnimSource() {}
