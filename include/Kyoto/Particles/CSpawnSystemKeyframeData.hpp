#ifndef _CSPAWNSYSTEMKEYFRAMEDATA
#define _CSPAWNSYSTEMKEYFRAMEDATA
#include "rstl/RstlVersions.h"
#include "rstl/static_assert.hpp"
#if !RSTL_CORRUPTION
#include "rstl/RstlVersions.h"
#endif
#include "rstl/types.h"

#if RSTL_CORRUPTION
#include "Kyoto/CAssetId.hpp"
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
#if RSTL_CORRUPTION
    CAssetId mId;
#else
    uint mId;
#endif
    uint mType;
#if !RSTL_CORRUPTION
    uint x8_;
#endif
    uint xc_;
#if RSTL_CORRUPTION
    uint x10_;
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
  int x0_;
  int x4_;
  int mEndFrame;
  int xc_;
  rstl::vector< rstl::pair< uint, rstl::vector< CSpawnSystemKeyframeInfo > > > mFrames;
};
#if RSTL_CORRUPTION
CHECK_SIZEOF(CSpawnSystemKeyframeData, 0x20)
NESTED_CHECK_SIZEOF(CSpawnSystemKeyframeData, CSpawnSystemKeyframeInfo, 0x20)
#else
#if !RSTL_PRIME1
CHECK_SIZEOF(CSpawnSystemKeyframeData, 0x20)
#endif
#endif

#endif // _CSPAWNSYSTEMKEYFRAMEDATA
