#ifndef _CMIDIMANAGER
#define _CMIDIMANAGER

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "rstl/RstlVersions.h"

#include <rstl/auto_ptr.hpp>
#include <rstl/reserved_vector.hpp>

class CInputStream;

class CMidiManager {
public:
  class CMidiWrapper {
  public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    typedef u32 TSysHandle;
#else
    typedef uint TSysHandle;
#endif
    CMidiWrapper();
    const CSfxHandle& GetManagerHandle() const;
    const TSysHandle GetAudioSysHandle() const;
    const bool IsAvailable() const;

    void SetAvailable(const bool v);
    void SetAudioSysHandle(const TSysHandle handle);
    const short GetSongId() const;
    void SetMidiHandle(const CSfxHandle& handle);
    void SetSongId(const short id);

  private:
    TSysHandle mSysHandle;
    CSfxHandle mMidiHandle;
    short mSongId;
    bool mAvailable;
  };

  class CMidiData {
  public:
    CMidiData(CInputStream& in);

    const short GetSongId() const { return mSongId; }
    const short GetGroupId() const { return mGroupId; }
    CAssetId GetAGSCAssetId() const { return mAgscId; }
    uchar* GetData() const { return mData.get(); }

  private:
    short mSongId;
    short mGroupId;
    CAssetId mAgscId;
    rstl::auto_ptr< uchar > mData;
  };

  static CSfxHandle Play(const CMidiData&, unsigned short fadeTime, bool stopExisting,
                         short volume);
  static void Stop(const CSfxHandle&, unsigned short);
  static void StopAll();

  static CSfxHandle LocateHandle();

  static rstl::reserved_vector< CMidiWrapper, 3 > mMidiWrappers;
};

#endif // _CMIDIMANAGER
