#ifndef _CANIMSOURCEREADERBASE
#define _CANIMSOURCEREADERBASE
#include "rstl/RstlVersions.h"
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"

#if RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#endif
#include "Kyoto/Animation/IAnimReader.hpp"
#include "Kyoto/Animation/IAnimSourceInfo.hpp"
#include "rstl/object_owner.hpp"
#include "rstl/pair.hpp"
#include "rstl/set.hpp"
#include "rstl/vector.hpp"

class CAnimPOIData;

class CAnimSourceReaderBase : public IAnimReader {
public:
#if RSTL_CORRUPTION
  CAnimSourceReaderBase(const rstl::ownership_transfer< IAnimSourceInfo >& sourceInfo,
                        const CAnimPOIData* poiData)
#else
#if RSTL_PRIME1
  ~CAnimSourceReaderBase() override {}
#else
  ~CAnimSourceReaderBase() override;
#endif
  uint VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, uint capacity,
                       uint iterator, int additive) const override;
  uint VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, uint capacity,
                        uint iterator, int additive) const override;
  uint VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, uint capacity,
                           uint iterator, int additive) const override;
  uint VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, uint capacity,
                        uint iterator, int additive) const override;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  bool VGetBoolPOIState(const char* name) const override;
  s32 VGetInt32POIState(const char* name) const override;
  CParticleData::EParentedMode VGetParticlePOIState(const char* name) const override;
#else
  bool VGetBoolPOIState(uint nameHash) const override;
  s32 VGetInt32POIState(uint nameHash) const override;
  CParticleData::EParentedMode VGetParticlePOIState(uint nameHash) const override;
#endif

  CAnimSourceReaderBase(const rstl::ownership_transfer< IAnimSourceInfo >& sourceInfo
#if !RSTL_PRIME1
                        , const CAnimPOIData* poiData
#endif
                        )
#endif
  : mSourceInfo(sourceInfo)
#if RSTL_CORRUPTION
  , mPOIData(poiData)
  , mCurTime(0.f)
  , mPassedBoolCount(0)
  , mPassedParticleCount(0)
  , mPassedSoundCount(0)
  , mPassedAssetSoundCount(0)
  , mPassedAssetCount(0) {}
#else
#if !RSTL_PRIME1
  , mPOIData(poiData)
#endif
  , mCurTime(0.f) {}
#endif

#if RSTL_CORRUPTION
  CAnimSourceReaderBase(
      const rstl::ownership_transfer< IAnimSourceInfo >& sourceInfo, const CAnimPOIData* poiData,
      const CCharAnimTime& time, int passedBoolCount, int passedParticleCount, int passedSoundCount,
      int passedAssetSoundCount, int passedAssetCount,
                        const rstl::vector< rstl::pair< uint, bool > >& boolStates,
      const rstl::vector< rstl::pair< uint, CParticleData::EParentedMode > >& particleStates)
#else
  CAnimSourceReaderBase(const rstl::ownership_transfer< IAnimSourceInfo >& sourceInfo
#if !RSTL_PRIME1
                        , const CAnimPOIData* poiData
#endif
                        , const CCharAnimTime& time, int passedBoolCount, int passedIntCount,
                        int passedParticleCount, int passedSoundCount,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                        const rstl::vector< rstl::pair< rstl::string, bool > >& boolStates,
                        const rstl::vector< rstl::pair< rstl::string, int > >& intStates,
                        const rstl::vector< rstl::pair< rstl::string, CParticleData::EParentedMode > >&
                            particleStates)
#else
                        const rstl::vector< rstl::pair< uint, bool > >& boolStates,
                        const rstl::vector< rstl::pair< uint, int > >& intStates,
                        const rstl::vector< rstl::pair< uint, CParticleData::EParentedMode > >&
                            particleStates)
#endif
#endif
  : mSourceInfo(sourceInfo)
#if !RSTL_PRIME1
  , mPOIData(poiData)
#endif
  , mCurTime(time)
  , mPassedBoolCount(passedBoolCount)
#if !RSTL_CORRUPTION
  , mPassedIntCount(passedIntCount)
#endif
  , mPassedParticleCount(passedParticleCount)
  , mPassedSoundCount(passedSoundCount)
#if RSTL_CORRUPTION
  , mPassedAssetSoundCount(passedAssetSoundCount)
  , mPassedAssetCount(passedAssetCount)
#endif
  , mBoolStates(boolStates)
#if !RSTL_CORRUPTION
  , mInt32States(intStates)
#endif
  , mParticleStates(particleStates) {}
#if RSTL_CORRUPTION

  // IAnimReader
  ~CAnimSourceReaderBase() override {}
  uint VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, uint capacity,
                       uint iterator, const CSegIdMaskMap& mask) const override;
  uint VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, uint capacity,
                           uint iterator, const CSegIdMaskMap& mask) const override;
  uint VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, uint capacity,
                        uint iterator, const CSegIdMaskMap& mask) const override;
  uint VGetAssetSoundPOIList(const CCharAnimTime& time, CAssetSoundPOINode* listOut, uint capacity,
                             uint iterator, const CSegIdMaskMap& mask) const override;
  uint VGetAssetPOIList(const CCharAnimTime& time, CAssetPOINode* listOut, uint capacity,
                        uint iterator, const CSegIdMaskMap& mask) const override;
  bool VGetBoolPOIState(uint nameHash) const override;
  CParticleData::EParentedMode VGetParticlePOIState(uint nameHash) const override;
#endif

  void PostConstruct(const CCharAnimTime& time);
  void UpdatePOIStates();
#if RSTL_PRIME1
  const IAnimSourceInfo& AnimSource() const { return *mSourceInfo; }
  const CCharAnimTime& GetCurTime() const { return mCurTime; }
#endif

protected:
#if RSTL_CORRUPTION
  rstl::set< rstl::pair< uint, int > > GetUniqueBoolPOIs() const;
  rstl::set< rstl::pair< uint, int > > GetUniqueParticlePOIs() const;
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  rstl::set< rstl::pair< rstl::string, int > > GetUniqueBoolPOIs() const;
  rstl::set< rstl::pair< rstl::string, int > > GetUniqueInt32POIs() const;
  rstl::set< rstl::pair< rstl::string, int > > GetUniqueParticlePOIs() const;
#else
  rstl::set< rstl::pair< uint, int > > GetUniqueBoolPOIs() const;
  rstl::set< rstl::pair< uint, int > > GetUniqueInt32POIs() const;
  rstl::set< rstl::pair< uint, int > > GetUniqueParticlePOIs() const;
#endif
#endif

  rstl::object_owner< IAnimSourceInfo > mSourceInfo;
#if !RSTL_PRIME1
  const CAnimPOIData* mPOIData;
#endif
  CCharAnimTime mCurTime;
  int mPassedBoolCount;
#if !RSTL_CORRUPTION
  int mPassedIntCount;
#endif
  int mPassedParticleCount;
  int mPassedSoundCount;
#if RSTL_CORRUPTION
  int mPassedAssetSoundCount; // Guessed name
  int mPassedAssetCount;      // Guessed name
  rstl::vector< rstl::pair< uint, bool > > mBoolStates;
  rstl::vector< rstl::pair< uint, CParticleData::EParentedMode > > mParticleStates;
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  rstl::vector< rstl::pair< rstl::string, bool > > mBoolStates;
  rstl::vector< rstl::pair< rstl::string, int > > mInt32States;
  rstl::vector< rstl::pair< rstl::string, CParticleData::EParentedMode > > mParticleStates;
#else
  rstl::vector< rstl::pair< uint, bool > > mBoolStates;
  rstl::vector< rstl::pair< uint, int > > mInt32States;
  rstl::vector< rstl::pair< uint, CParticleData::EParentedMode > > mParticleStates;
#endif
#endif
};
#if RSTL_CORRUPTION
CHECK_SIZEOF(CAnimSourceReaderBase, 0x4c)
#else
CHECK_CHILD_SIZEOF(CAnimSourceReaderBase, IAnimReader, RSTL_PRIME1 ? 0x50 : (RSTL_VERSION >= RSTL_R32J ? 0x48 : 0x54))
#endif

#if !RSTL_CORRUPTION
#if !RSTL_PRIME1
inline CAnimSourceReaderBase::~CAnimSourceReaderBase() {}
#endif
#endif

#endif // _CANIMSOURCEREADERBASE
