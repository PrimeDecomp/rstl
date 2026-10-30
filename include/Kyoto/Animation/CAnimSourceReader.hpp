#ifndef _CANIMSOURCEREADER
#define _CANIMSOURCEREADER
#include "rstl/RstlVersions.h"

#include "Kyoto/Animation/CAllFormatsAnimSource.hpp"
#include "Kyoto/Animation/CAnimSource.hpp"

#include "Kyoto/Animation/CAnimSourceReaderBase.hpp"
#if !RSTL_PRIME1
#include "Kyoto/Animation/TSubAnimTypeToken.hpp"
#endif

class CAnimSourceInfo : public IAnimSourceInfo {
public:
#if !RSTL_PRIME1
  explicit CAnimSourceInfo(const TSubAnimTypeToken< CAnimSource >& source) : mSource(source) {}

  // IAnimSourceInfo
#endif

  ~CAnimSourceInfo() override {}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  bool HasPOIData() const override { return mToken->HasPOIData(); }
  const rstl::vector< CBoolPOINode >& GetBoolPOIStream() const override {
    return mToken->GetBoolPOIStream();
  }
  const rstl::vector< CInt32POINode >& GetInt32POIStream() const override {
    return mToken->GetInt32POIStream();
  }
  const rstl::vector< CParticlePOINode >& GetParticlePOIStream() const override {
    return mToken->GetParticlePOIStream();
  }
  const rstl::vector< CSoundPOINode >& GetSoundPOIStream() const override {
    return mToken->GetSoundPOIStream();
  }
  CCharAnimTime GetAnimationDuration() const override { return mToken->GetAnimationDuration(); }
#else
  CCharAnimTime GetAnimationDuration() const override { return mSource->GetAnimationDuration(); }
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CAnimSourceInfo(const TSubAnimTypeToken< CAnimSource >& source) : mToken(source) {}
#else
  bool HasScaleData() const override { return mSource->HasScaleData(); }
#endif

private:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  TSubAnimTypeToken< CAnimSource > mToken;
#else
  TSubAnimTypeToken< CAnimSource > mSource;
#endif
};
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CHECK_SIZEOF(CAnimSourceInfo, 0x14)
#else
CHECK_CHILD_SIZEOF(CAnimSourceInfo, IAnimSourceInfo, 0x10)
#endif

class CAnimSourceReader : public CAnimSourceReaderBase {
public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  ~CAnimSourceReader() override {}
#else
  CAnimSourceReader(const TSubAnimTypeToken< CAnimSource >& source, const CCharAnimTime& time,
                    const CAnimPOIData* poiData);

  // IAnimReader
  ~CAnimSourceReader() override;
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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CAnimSourceReader(const TSubAnimTypeToken< CAnimSource >& source, const CCharAnimTime& time);
#endif

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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CHECK_SIZEOF(CAnimSourceReader, 0x7c)
#else
CHECK_CHILD_SIZEOF(CAnimSourceReader, CAnimSourceReaderBase, 0x28)
#endif

#endif // _CANIMSOURCEREADER
