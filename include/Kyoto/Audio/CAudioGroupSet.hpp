#ifndef _CAUDIOGROUPSET
#define _CAUDIOGROUPSET
#include "rstl/RstlVersions.h"

#include "Kyoto/TToken.hpp"
#include "types.h"

#include "rstl/auto_ptr.hpp"
#include "rstl/optional_object.hpp"

#include "rstl/single_ptr.hpp"
#include "rstl/string.hpp"

#include "Kyoto/CFactoryMgr.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"

class CInputStream;
class CVParamTransfer;

class CAudioGrpSetLoc {
public:
  CAudioGrpSetLoc(const rstl::auto_ptr< uchar >& data, int length);
  ~CAudioGrpSetLoc();
  const rstl::string& GetBaseDirName() const;
  const rstl::string& GetGroupSetName() const;
  void FreeSampleBuffer() const;
  int AramUsage() const;
  void* GetPool() const;
  void* GetProject() const;
  void* GetSample() const;
  void* GetSampleDir() const;

  static bool sInSinglePlayer;

private:
  uint ReadHeader(void* ptr, uint len, uint& readPosition);

#if RSTL_PRIME1
  mutable rstl::auto_ptr< uchar > mData;
  rstl::auto_ptr< uchar > mGroupData;
  rstl::string mBaseDirName;
  rstl::string mGroupSetName;
  int mAramSize;
  uchar* mPool;
  uchar* mProject;
  uchar* mSampleDir;
  mutable uchar* mSamples;
#else
  uint mStackId;
  const void* mAramAlloc;
  rstl::single_ptr< uchar > mStackBuffer;
  rstl::single_ptr< uchar > mPool;
  rstl::single_ptr< uchar > mProject;
  rstl::single_ptr< uchar > mSampleDir;
#endif
};
CHECK_SIZEOF(CAudioGrpSetLoc, RSTL_PRIME1 ? 0x44 : 0x18)

#if RSTL_PRIME1
inline CAudioGrpSetLoc::~CAudioGrpSetLoc() {}
inline const rstl::string& CAudioGrpSetLoc::GetBaseDirName() const { return mBaseDirName; }
inline const rstl::string& CAudioGrpSetLoc::GetGroupSetName() const { return mGroupSetName; }
inline int CAudioGrpSetLoc::AramUsage() const { return mAramSize; }
inline void* CAudioGrpSetLoc::GetPool() const { return mPool; }
inline void* CAudioGrpSetLoc::GetProject() const { return mProject; }
inline void* CAudioGrpSetLoc::GetSample() const { return mSamples; }
inline void* CAudioGrpSetLoc::GetSampleDir() const { return mSampleDir; }
inline uint CAudioGrpSetLoc::ReadHeader(void* ptr, uint len, uint& readPosition) {
  CMemoryInStream in(ptr, len);
  in.CInputStream::Read(mBaseDirName);
  in.CInputStream::Read(mGroupSetName);
  const uint ret = in.ReadLong();
  readPosition = in.GetReadPosition();
  return ret;
}
#endif

class CAudioGroupSet {
public:
  CAudioGroupSet(const TLockedToken< CAudioGrpSetLoc >& group);
  ~CAudioGroupSet();
  void Reload();
  void FreeSampleBuffer();
  void FreeAllBuffers();
  const rstl::string& GetName() const { return mGroupSetName; }
  int AramUsage() { return mGroupSetTok.data()->AramUsage(); }
  void* GetProjBuffer() const { return mGroupSetTok.data()->GetProject(); }
  void* GetSampleBuffer() const { return mGroupSetTok.data()->GetSample(); }
  void* GetSDirBuffer() const { return mGroupSetTok.data()->GetSampleDir(); }
  void* GetPoolBuffer() const { return mGroupSetTok.data()->GetPool(); }

private:
  rstl::string mBaseDir;
  rstl::string mGroupSetName;
  rstl::optional_object< TLockedToken< CAudioGrpSetLoc > > mGroupSetTok;
};
CHECK_SIZEOF(CAudioGroupSet, 0x30)

RSTL_PRIME1_CONST CFactoryFnReturn FAudioGroupSetLocDataFactory(const SObjectTag& tag,
                                                                const rstl::auto_ptr< uchar >& data,
                                                                int length,
                                                                const CVParamTransfer& xfer);

#endif // _CAUDIOGROUPSET
