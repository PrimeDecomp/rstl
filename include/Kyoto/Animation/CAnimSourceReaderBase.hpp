#ifndef _CANIMSOURCEREADERBASE
#define _CANIMSOURCEREADERBASE
#include "rstl/RstlVersions.h"

#include "Kyoto/Animation/IAnimReader.hpp"
#include "Kyoto/Animation/IAnimSourceInfo.hpp"
#include "rstl/object_owner.hpp"
#include "rstl/pair.hpp"
#include "rstl/set.hpp"
#include "rstl/vector.hpp"

class CAnimPOIData;

class CAnimSourceReaderBase : public IAnimReader {
public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  ~CAnimSourceReaderBase() override {}
  uint VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, uint capacity,
                       uint iterator, int unk) const override;
  uint VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, uint capacity,
                        uint iterator, int unk) const override;
  uint VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, uint capacity,
                           uint iterator, int unk) const override;
  uint VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, uint capacity,
                        uint iterator, int unk) const override;
  bool VGetBoolPOIState(const char* name) const override;
  s32 VGetInt32POIState(const char* name) const override;
  CParticleData::EParentedMode VGetParticlePOIState(const char* name) const override;
#endif

  CAnimSourceReaderBase(const rstl::ownership_transfer< IAnimSourceInfo >& sourceInfo
#if !RSTL_PRIME1
                        
                        , const CAnimPOIData* poiData
#endif
                        )
  : mSourceInfo(sourceInfo)
#if !RSTL_PRIME1
  , mPOIData(poiData)
#endif
  , mCurTime(0.f) {}

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
  : mSourceInfo(sourceInfo)
#if !RSTL_PRIME1
  , mPOIData(poiData)
#endif
  , mCurTime(time)
  , mPassedBoolCount(passedBoolCount)
  , mPassedIntCount(passedIntCount)
  , mPassedParticleCount(passedParticleCount)
  , mPassedSoundCount(passedSoundCount)
  , mBoolStates(boolStates)
  , mInt32States(intStates)
  , mParticleStates(particleStates) {}

#if !RSTL_PRIME1
  // IAnimReader
  ~CAnimSourceReaderBase() override;
  uint VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, uint capacity,
                       uint iterator, int additive) const override;
  uint VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, uint capacity,
                        uint iterator, int additive) const override;
  uint VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, uint capacity,
                           uint iterator, int additive) const override;
  uint VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, uint capacity,
                        uint iterator, int additive) const override;
  bool VGetBoolPOIState(uint nameHash) const override;
  s32 VGetInt32POIState(uint nameHash) const override;
  CParticleData::EParentedMode VGetParticlePOIState(uint nameHash) const override;
#endif

  void PostConstruct(const CCharAnimTime& time);
  void UpdatePOIStates();
#if RSTL_PRIME1
  const IAnimSourceInfo& AnimSource() const { return *mSourceInfo; }
  const CCharAnimTime& GetCurTime() const { return mCurTime; }
#endif

protected:
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

  rstl::object_owner< IAnimSourceInfo > mSourceInfo;
#if !RSTL_PRIME1
  const CAnimPOIData* mPOIData;
#endif
  CCharAnimTime mCurTime;
  int mPassedBoolCount;
  int mPassedIntCount;
  int mPassedParticleCount;
  int mPassedSoundCount;
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
};
CHECK_CHILD_SIZEOF(CAnimSourceReaderBase, IAnimReader, RSTL_PRIME1 ? 0x50 : (RSTL_VERSION >= RSTL_R32J ? 0x48 : 0x54))

#if !RSTL_PRIME1
inline CAnimSourceReaderBase::~CAnimSourceReaderBase() {}
#endif

#endif // _CANIMSOURCEREADERBASE
