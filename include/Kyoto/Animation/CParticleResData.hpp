#ifndef _CPARTICLERESDATA
#define _CPARTICLERESDATA

#include "rstl/static_assert.hpp"
#include "rstl/types.h"

#include "Kyoto/CAssetId.hpp"

#include "rstl/vector.hpp"

class CInputStream;

class CParticleResData {
public:
  CParticleResData(CInputStream& in, ushort tableCount);
  // Guessed parameter names
  CParticleResData(const rstl::vector< CAssetId >& part, const rstl::vector< CAssetId >& swhc,
                   const rstl::vector< CAssetId >& elscA, const rstl::vector< CAssetId >& spsc,
                   const rstl::vector< CAssetId >& srsc, const rstl::vector< CAssetId >& elscB)
  : mPart(part), mSwhc(swhc), mElscA(elscA), mSpsc(spsc), mSrsc(srsc), mElscB(elscB) {}
  const rstl::vector< CAssetId >& GetParts() const { return mPart; }
  const rstl::vector< CAssetId >& GetSwooshes() const { return mSwhc; }
  const rstl::vector< CAssetId >& GetElectrics() const { return mElscA; }
  const rstl::vector< CAssetId >& GetSpawnSystems() const { return mSpsc; }
  const rstl::vector< CAssetId >& GetSortedSystems() const { return mSrsc; }

private:
  rstl::vector< CAssetId > mPart;
  rstl::vector< CAssetId > mSwhc;
  rstl::vector< CAssetId > mElscA;
  rstl::vector< CAssetId > mSpsc;
  rstl::vector< CAssetId > mSrsc;
  rstl::vector< CAssetId > mElscB;
};
CHECK_SIZEOF(CParticleResData, 0x60)

#endif // _CPARTICLERESDATA
