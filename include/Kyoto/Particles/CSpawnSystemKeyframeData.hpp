#ifndef _CSPAWNSYSTEMKEYFRAMEDATA
#define _CSPAWNSYSTEMKEYFRAMEDATA
#include "rstl/RstlVersions.h"
#if !RSTL_PRIME1
#include "types.h"
#endif

#include "Kyoto/CToken.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/pair.hpp"
#include "rstl/vector.hpp"

class CInputStream;

class CSimplePool;
class CSpawnSystemKeyframeData {
public:
  class CSpawnSystemKeyframeInfo {
  public:
    CSpawnSystemKeyframeInfo(CInputStream& in);
    ~CSpawnSystemKeyframeInfo() {
      if (mToken) {
        CToken& token = *mToken;
        token.Unlock();
      }
    }
    void LoadToken(CSimplePool* pool);
    rstl::optional_object< CToken > GetToken() const { return mToken; }
#if !RSTL_PRIME1
    uint GetType() const { return mType; }
#endif

  private:
    uint mId;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    uint x4;
    uint x8;
    uint xc;
#else
    uint mType;
    uint x8_;
    uint xc_;
#endif

    rstl::optional_object< CToken > mToken;
  };

  CSpawnSystemKeyframeData(CInputStream& in);
  ~CSpawnSystemKeyframeData() {}

  void LoadAllSpawnedSystemTokens(CSimplePool* pool);
  rstl::vector< CSpawnSystemKeyframeInfo >& GetSpawnedSystemsAtFrame(uint frame);
#if !RSTL_PRIME1
  uint GetEndFrame() const { return mEndFrame; }
#endif

private:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  int mUnknown1;
  int mUnknown2;
#else
  int x0_;
  int x4_;
#endif

  int mEndFrame;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  int mUnknown3;
#else
  int xc_;
#endif

  rstl::vector< rstl::pair< uint, rstl::vector< CSpawnSystemKeyframeInfo > > > mFrames;
};
#if !RSTL_PRIME1
CHECK_SIZEOF(CSpawnSystemKeyframeData, 0x20)
#endif

#endif // _CSPAWNSYSTEMKEYFRAMEDATA
