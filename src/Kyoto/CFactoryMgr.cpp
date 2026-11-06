#include "Kyoto/CFactoryMgr.hpp"
#include "rstl/macros.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Streams/CLZOInputStream.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"
#include "Kyoto/Streams/CZipInputStream.hpp"
#include "rstl/RstlVersions.h"

#include "ctype.h"
#include "dolphin/os.h"
#include "string.h"

static const uint sTypeTable[] = {
    'CLSN',
    IF_NOT_RSTL_PRIME1('CSPP', ) //
    'CMDL',
    'CSKR',
    'ANIM',
    'CINF',
    'TXTR',
    'PLTT',
    'FONT',
    'ANCS',
    IF_RSTL_PRIME1('EVNT', )     //
    IF_NOT_RSTL_PRIME1('ANMS', ) //
    'MADF',
    'MLVL',
    'MREA',
    'MAPW',
    'MAPA',
    'SAVW',
    'SAVA',
    'PART',
    'WPSC',
    'SWHC',
    'DPSC',
    'ELSC',
    'CRSC',
    IF_NOT_RSTL_PRIME1('SPSC', ) //
    IF_NOT_RSTL_PRIME1('SRSC', ) //
    'AFSM',
    'DCLN',
    'AGSC',
    'ATBL',
    'CSNG',
    'STRG',
    'SCAN',
    'PATH',
    'DGRP',
    'HMAP',
    IF_NOT_RSTL_PRIME1('PTLA', ) //
    IF_NOT_RSTL_PRIME1('STLC', ) //
    IF_NOT_RSTL_PRIME1('EGMC', ) //
    IF_NOT_RSTL_PRIME1('RULE', ) //
    IF_NOT_RSTL_PRIME1('FSM2', ) //
    'CTWK',
    'FRME',
    'HINT',
    'MAPU',
    'DUMB',
};

CFactoryMgr::CFactoryMgr() {}

CFactoryMgr::~CFactoryMgr() {}

void CFactoryMgr::AddFactory(FourCC type, FFactoryFunc factory) {
  AUTO(it, mFactories.find(type));
  if (it != mFactories.end()) {
    return;
  }
  mFactories.insert(rstl::pair< int, FFactoryFunc >(type, factory));
}

void CFactoryMgr::AddFactory(FourCC type, FMemFactoryFunc factory) {
  AUTO(it, mMemFactories.find(type));
  if (it != mMemFactories.end()) {
    return;
  }
  mMemFactories.insert(rstl::pair< int, FMemFactoryFunc >(type, factory));
}

bool CFactoryMgr::CanMakeMemory(const SObjectTag& tag) const {
  return mMemFactories.find(tag.GetType()) != mMemFactories.end();
}

rstl::auto_ptr< IObj > CFactoryMgr::MakeObject(const SObjectTag& tag, CInputStream& in,
                                               const CVParamTransfer& params) {
  AUTO(it, mFactories.find(tag.GetType()));
  return it->second(tag, in, params).GetObjForTransfer();
}

rstl::auto_ptr< IObj > CFactoryMgr::MakeObjectFromMemory(const SObjectTag& tag,
                                                         MemoryBufferPtr buffer, int size,
                                                         bool compressed,
                                                         const CVParamTransfer& params) {
  rstl::map< int, FMemFactoryFunc >::const_iterator memIt = mMemFactories.find(tag.GetType());
  if (memIt != mMemFactories.end()) {
    FMemFactoryFunc factory = memIt->second;
    if (compressed) {
#if RSTL_PRIME1
      rstl::auto_ptr< CInputStream > in(
          rs_new CMemoryInStream(buffer, size, CMemoryInStream::kOS_Owned));
      const uint length = in->Get< uint >();
      rstl::auto_ptr< uchar > data(rs_new uchar[length]);
      {
        CZipInputStream zip(in);
        zip.Get(data.get(), length);
      }
#else
      rstl::auto_ptr< CInputStream > in(
          rs_new CMemoryInStream(buffer.get(), size, CMemoryInStream::kOS_NotOwned));
      const uint length = in->ReadInt32();
      CLZOInputStream lzo(in, size - in->GetReadPosition(), length);
      rstl::auto_ptr< uchar > data(lzo.ReleaseBuffer());
#endif
      return factory(tag, data, length, params).GetObjForTransfer();
    }
#if RSTL_PRIME1
    rstl::auto_ptr< uchar > data(static_cast< uchar* >(buffer));
#else
    rstl::auto_ptr< uchar > data(buffer);
    if (!data.owner()) {
      data = rstl::auto_ptr< uchar >(
          static_cast< uchar* >(CMemory::Alloc(size, IAllocator::kHI_RoundUpLen)));
      memcpy(data.get(), buffer.get(), size);
      DCStoreRange(data.get(), size);
    }
#endif
    return factory(tag, data, size, params).GetObjForTransfer();
  }

  FFactoryFunc factory = mFactories.find(tag.GetType())->second;
  if (compressed) {
#if RSTL_PRIME1
    CInputStream* in = rs_new CMemoryInStream(buffer, size, CMemoryInStream::kOS_Owned);
    in->ReadLong();
    CZipInputStream zip(in);
    return factory(tag, zip, params).GetObjForTransfer();
#else
    rstl::auto_ptr< CInputStream > in(
        rs_new CMemoryInStream(buffer.get(), size, CMemoryInStream::kOS_NotOwned));
    const uint length = in->ReadInt32();
    CLZOInputStream lzo(in, size - in->GetReadPosition(), length);
    return factory(tag, lzo, params).GetObjForTransfer();
#endif
  }
#if RSTL_PRIME1
  CMemoryInStream in(buffer, size, CMemoryInStream::kOS_Owned);
#else
  CMemoryInStream in(buffer.get(), size, CMemoryInStream::kOS_NotOwned);
#endif
  return factory(tag, in, params).GetObjForTransfer();
}

uint CFactoryMgr::TypeIdxToFourCC(uint idx) { return sTypeTable[idx]; }

uint CFactoryMgr::FourCCToTypeIdx(uint fcc) {
  char* type = reinterpret_cast< char* >(&fcc);
  type[0] = toupper(type[0]);
  type[1] = toupper(type[1]);
  type[2] = toupper(type[2]);
  type[3] = toupper(type[3]);
  for (uint i = 0; i < sizeof(sTypeTable) / sizeof(sTypeTable[0]); ++i) {
    if (fcc == sTypeTable[i]) {
      return i;
    }
  }
  return -1;
}
