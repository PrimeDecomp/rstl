#include "rstl/RstlVersions.h"
#include "Kyoto/CFactoryMgr.hpp"

#if !RSTL_PRIME1
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Streams/CLZOInputStream.hpp"
#endif

#include "Kyoto/Streams/CMemoryInStream.hpp"
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#include "Kyoto/Streams/CZipInputStream.hpp"
#else
#include "dolphin/os.h"
#endif

#include "ctype.h"
#if !RSTL_PRIME1
#include "string.h"
#endif

static const uint sTypeTable[] = {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    'CLSN', 'CMDL', 'CSKR', 'ANIM', 'CINF', 'TXTR', 'PLTT', 'FONT', 'ANCS', 'EVNT',
    'MADF', 'MLVL', 'MREA', 'MAPW', 'MAPA', 'SAVW', 'SAVA', 'PART', 'WPSC', 'SWHC',
    'DPSC', 'ELSC', 'CRSC', 'AFSM', 'DCLN', 'AGSC', 'ATBL', 'CSNG', 'STRG', 'SCAN',
    'PATH', 'DGRP', 'HMAP', 'CTWK', 'FRME', 'HINT', 'MAPU', 'DUMB',
#else
    'CLSN', 'CSPP', 'CMDL', 'CSKR', 'ANIM', 'CINF', 'TXTR', 'PLTT', 'FONT', 'ANCS', 'ANMS', 'MADF',
    'MLVL', 'MREA', 'MAPW', 'MAPA', 'SAVW', 'SAVA', 'PART', 'WPSC', 'SWHC', 'DPSC', 'ELSC', 'CRSC',
    'SPSC', 'SRSC', 'AFSM', 'DCLN', 'AGSC', 'ATBL', 'CSNG', 'STRG', 'SCAN', 'PATH', 'DGRP', 'HMAP',
    'PTLA', 'STLC', 'EGMC', 'RULE', 'FSM2', 'CTWK', 'FRME', 'HINT', 'MAPU', 'DUMB',
#endif

};

CFactoryMgr::CFactoryMgr() {}

CFactoryMgr::~CFactoryMgr() {}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CFactoryMgr::AddFactory(uint type, FFactoryFunc factory) {
  AUTO(it, mFactories.find(type));
#else
void CFactoryMgr::AddFactory(FourCC type, FFactoryFunc factory) {
  rstl::map< int, FFactoryFunc >::iterator it = mFactories.find(type);
#endif

  if (it != mFactories.end()) {
    return;
  }
  mFactories.insert(rstl::pair< int, FFactoryFunc >(type, factory));
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CFactoryMgr::AddFactory(uint type, FMemFactoryFunc factory) {
  AUTO(it, mMemFactories.find(type));
#else
void CFactoryMgr::AddFactory(FourCC type, FMemFactoryFunc factory) {
  rstl::map< int, FMemFactoryFunc >::iterator it = mMemFactories.find(type);
#endif

  if (it != mMemFactories.end()) {
    return;
  }
  mMemFactories.insert(rstl::pair< int, FMemFactoryFunc >(type, factory));
}

bool CFactoryMgr::CanMakeMemory(const SObjectTag& tag) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return mMemFactories.find(tag.GetType()) != mMemFactories.end();
#else
  return mMemFactories.find(tag.type) != mMemFactories.end();
#endif

}

rstl::auto_ptr< IObj > CFactoryMgr::MakeObject(const SObjectTag& tag, CInputStream& in,
                                               const CVParamTransfer& params) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  AUTO(it, mFactories.find(tag.GetType()));
#else
  rstl::map< int, FFactoryFunc >::iterator it = mFactories.find(tag.type);
#endif

  return it->second(tag, in, params).GetObjForTransfer();
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
rstl::auto_ptr< IObj > CFactoryMgr::MakeObjectFromMemory(const SObjectTag& tag, void* buffer,
#else
rstl::auto_ptr< IObj > CFactoryMgr::MakeObjectFromMemory(const SObjectTag& tag,
                                                         const rstl::auto_ptr< uchar >& buffer,
#endif

                                                         int size, bool compressed,
                                                         const CVParamTransfer& params) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  rstl::map< int, FMemFactoryFunc >::const_iterator memIt = mMemFactories.find(tag.GetType());
#else
  rstl::map< int, FMemFactoryFunc >::const_iterator memIt = mMemFactories.find(tag.type);
#endif

  if (memIt != mMemFactories.end()) {
    FMemFactoryFunc factory = memIt->second;
    if (compressed) {
      rstl::auto_ptr< CInputStream > in(
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
          rs_new CMemoryInStream(buffer, size, CMemoryInStream::kOS_Owned));
      const uint length = in->Get< uint >();
      rstl::auto_ptr< uchar > data(rs_new uchar[length]);
      {
        CZipInputStream zip(in);
        zip.Get(data.get(), length);
      }
#else
          rs_new CMemoryInStream(buffer.get(), size, CMemoryInStream::kOS_NotOwned));
      const uint length = in->ReadInt32();
      CLZOInputStream lzo(in, size - in->GetReadPosition(), length);
      rstl::auto_ptr< uchar > data(lzo.ReleaseBuffer());
#endif

      return factory(tag, data, length, params).GetObjForTransfer();
    }
// TODO: verify that this Prime/Echoes difference is real
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

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  FFactoryFunc factory = mFactories.find(tag.GetType())->second;
#else
  FFactoryFunc factory = mFactories.find(tag.type)->second;
#endif

  if (compressed) {
// TODO: verify that this Prime/Echoes difference is real
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
// TODO: verify that this Prime/Echoes difference is real
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

