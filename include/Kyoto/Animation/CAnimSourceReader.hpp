#ifndef _CANIMSOURCEREADER
#define _CANIMSOURCEREADER
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"

#include "Kyoto/Animation/CAllFormatsAnimSource.hpp"
#include "Kyoto/Animation/CAnimSource.hpp"

#include "Kyoto/Animation/CAnimSourceReaderBase.hpp"
#if !RSTL_PRIME1
#include "Kyoto/Animation/TSubAnimTypeToken.hpp"
#endif

class CAnimSourceInfo : public IAnimSourceInfo {
public:
  explicit CAnimSourceInfo(const TSubAnimTypeToken< CAnimSource >& source) : mSource(source) {}

  // IAnimSourceInfo
  ~CAnimSourceInfo() override {}

#if RSTL_PRIME1
  bool HasPOIData() const override { return mSource->HasPOIData(); }
  const rstl::vector< CBoolPOINode >& GetBoolPOIStream() const override {
    return mSource->GetBoolPOIStream();
  }
  const rstl::vector< CInt32POINode >& GetInt32POIStream() const override {
    return mSource->GetInt32POIStream();
  }
  const rstl::vector< CParticlePOINode >& GetParticlePOIStream() const override {
    return mSource->GetParticlePOIStream();
  }
  const rstl::vector< CSoundPOINode >& GetSoundPOIStream() const override {
    return mSource->GetSoundPOIStream();
  }
#endif
  CCharAnimTime GetAnimationDuration() const override { return mSource->GetAnimationDuration(); }

#if !RSTL_PRIME1
  bool HasScaleData() const override { return mSource->HasScaleData(); }
#endif

private:
  TSubAnimTypeToken< CAnimSource > mSource;
};
CHECK_CHILD_SIZEOF(CAnimSourceInfo, IAnimSourceInfo, 0x10)

class CAnimSourceReader : public CAnimSourceReaderBase {
public:
  CAnimSourceReader(const TSubAnimTypeToken< CAnimSource >& source, const CCharAnimTime& time,
                    const CAnimPOIData* poiData);
#if !RSTL_PRIME1

  // IAnimReader
  ~CAnimSourceReader() override;
#else
  ~CAnimSourceReader() override {}
#endif
  SAdvancementResults VAdvanceView(const CCharAnimTime& time) override;

  CCharAnimTime VGetTimeRemaining() const override;
  CSteadyStateAnimInfo VGetSteadyStateAnimInfo() const override;
  bool VHasOffset(const CSegId& seg) const override;
  CVector3f VGetOffset(const CSegId& seg) const override;
  CQuaternion VGetRotation(const CSegId& seg) const override;
  void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut) const override;
  void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut,
                           const CCharAnimTime& time) const override;
#if !RSTL_PRIME1
  void VGetJointData_Linear(const CCharLayoutInfo& layout, CJointData_LinearStorage& data,
                            const CCharAnimTime& time) const override;
  void VGetJointData_Linear(const CCharLayoutInfo& layout,
                            CJointData_LinearStorage& data) const override;
#endif

  rstl::ownership_transfer< IAnimReader > VClone() const override;
  void VSetPhase(float phase) override;
  SAdvancementResults VGetAdvancementResults(const CCharAnimTime& time,
                                             const CCharAnimTime& startOffset) const override;

  virtual CVector3f VGetOffset(const CSegId& seg, const CCharAnimTime& time) const;
  virtual bool VSupportsReverseView() const;
  virtual SAdvancementResults VReverseView(const CCharAnimTime& time);
  CAnimSourceReader(const TSubAnimTypeToken< CAnimSource >& source, const CCharAnimTime& time);

private:
  CAnimSourceReader(const TSubAnimTypeToken< CAnimSource >& source
#if !RSTL_PRIME1
                    , const CAnimPOIData* poiData
#endif
                    , const CCharAnimTime& time, const CSteadyStateAnimInfo& steadyStateInfo,
                    int passedBoolCount, int passedIntCount, int passedParticleCount,
                    int passedSoundCount,
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
  : CAnimSourceReaderBase(rs_new CAnimSourceInfo(source)
#if !RSTL_PRIME1
                          , poiData
#endif
                          , time, passedBoolCount, passedIntCount, passedParticleCount,
                          passedSoundCount, boolStates, intStates, particleStates)
  , mSource(source)
  , mSteadyStateInfo(steadyStateInfo) {}

  TSubAnimTypeToken< CAnimSource > mSource;
  CSteadyStateAnimInfo mSteadyStateInfo;
};
CHECK_CHILD_SIZEOF(CAnimSourceReader, CAnimSourceReaderBase, 0x28)

#endif // _CANIMSOURCEREADER
