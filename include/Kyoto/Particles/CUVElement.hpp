#ifndef _CUVELEMENT
#define _CUVELEMENT
#include "rstl/RstlVersions.h"

#if RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#endif
#include "rstl/types.h"

#include "Kyoto/Particles/IElement.hpp"
#include "Kyoto/TToken.hpp"

class CTexture;

class CUVEConstant : public CUVElement {
  TLockedToken< CTexture > mTex;

public:
  CUVEConstant(TToken< CTexture > tex);
  ~CUVEConstant() override;
  TLockedToken< CTexture > GetValueTexture(int frame) const override;
  void GetValueUV(int frame, SUVElementSet& valOut) const override;
  bool HasConstantTexture() const override { return true; }
  bool HasConstantUV() const override { return true; }
};

class CUVEAnimTexture : public CUVElement {
  TLockedToken< CTexture > mTex;
  int mTileW;
  int mTileH;
  int mStrideW;
  int mStrideH;
  int mTiles;
  bool mLoop;
  CIntElement* mCycleFrames;
  rstl::vector< SUVElementSet > mUvElems;

public:
  CUVEAnimTexture(TToken< CTexture > tex, CIntElement* tileW, CIntElement* tileH,
                  CIntElement* strideW, CIntElement* strideH, CIntElement* cycleFrames, bool loop);
  ~CUVEAnimTexture() override;
  TLockedToken< CTexture > GetValueTexture(int frame) const override;
  void GetValueUV(int frame, SUVElementSet& valOut) const override;
  bool HasConstantTexture() const override { return true; }
  bool HasConstantUV() const override { return false; }
};

#if RSTL_CORRUPTION
// Guessed name
class CUVETEXP : public CUVElement {
  TLockedToken< CTexture > mTex;
  int mTileW;
  int mTileH;
  int mTiles;
  CRealElement* mTileSelection;
  rstl::vector< SUVElementSet > mUvElems;

public:
  CUVETEXP(TToken< CTexture > tex, CIntElement* tileW, CIntElement* tileH,
           CRealElement* tileSelection);
  ~CUVETEXP() override;
  TLockedToken< CTexture > GetValueTexture(int frame) const override;
  void GetValueUV(int frame, SUVElementSet& valOut) const override;
  bool HasConstantTexture() const override;
  bool HasConstantUV() const override;
};
CHECK_SIZEOF(CUVETEXP, 0x30)


#endif
#endif // _CUVELEMENT
