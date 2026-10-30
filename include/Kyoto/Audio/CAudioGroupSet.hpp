#ifndef _CAUDIOGROUPSET
#define _CAUDIOGROUPSET
#include "rstl/RstlVersions.h"

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#include "Kyoto/TToken.hpp"
#else
#include "types.h"
#endif

#include "rstl/auto_ptr.hpp"
#if RSTL_PRIME1
#include "rstl/optional_object.hpp"
#endif

#include "rstl/single_ptr.hpp"
#if RSTL_PRIME1
#include "rstl/string.hpp"
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"

class CInputStream;
#else
#include "Kyoto/CFactoryMgr.hpp"
#endif

#if !RSTL_PRIME1
class CVParamTransfer;

// Echoes AGSC: the group is pushed straight into ARAM through a MusyX sound stack.
#endif

class CAudioGrpSetLoc {
public:
  CAudioGrpSetLoc(const rstl::auto_ptr< uchar >& data, int length);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const rstl::string& GetBaseDirName() const { return mBaseDirName; }
  const rstl::string& GetGroupSetName() const { return mGroupSetName; }
  void FreeSampleBuffer() const;
  int AramUsage() const { return mAramSize; }
  void* GetPool() const { return mPool; }
  void* GetProject() const { return mProject; }
  void* GetSample() const { return mSamples; }
  void* GetSampleDir() const { return mSampleDir; }
#else
  ~CAudioGrpSetLoc();

  static bool sInSinglePlayer;
#endif

private:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  uint ReadHeader(void* ptr, uint len, uint& readPosition) {
    CMemoryInStream in(ptr, len);
    in.CInputStream::Read(mBaseDirName);
    in.CInputStream::Read(mGroupSetName);
    const uint ret = in.ReadLong();
    readPosition = in.GetReadPosition();
    return ret;
  }
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
#if !RSTL_PRIME1
CHECK_SIZEOF(CAudioGrpSetLoc, 0x18)
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
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

CHECK_SIZEOF(CAudioGrpSetLoc, 0x44)
CHECK_SIZEOF(CAudioGroupSet, 0x30)
#else
CFactoryFnReturn FAudioGroupSetLocDataFactory(const SObjectTag& tag,
                                                    const rstl::auto_ptr< uchar >& data, int length,
                                                    const CVParamTransfer& xfer);
#endif

#endif // _CAUDIOGROUPSET

