#ifndef _CANIMTREETIMESCALE
#define _CANIMTREETIMESCALE
#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"
#endif

#if RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#endif
#include "Kyoto/Animation/CAnimTreeSingleChild.hpp"
#include "Kyoto/Animation/CTimeScaleFunctions.hpp"
#include "rstl/object_owner.hpp"

class CAnimTreeTimeScale : public CAnimTreeSingleChild {
public:
  CAnimTreeTimeScale(const rstl::ncrc_ptr< CAnimTreeNode >& node, float scale,
                     const rstl::string& name)
  : CAnimTreeSingleChild(node, name)
#if RSTL_CORRUPTION
  , mTimeScale(rs_new_line_in("CAnimTreeTimeScale.h", 61) CConstantAnimationTimeScale(scale))
#else
  , mTimeScale(rs_new CConstantAnimationTimeScale(scale))
#endif
  , mCurAccelTime(0.f)
  , mTargetAccelTime(CCharAnimTime::Infinity())
  , mInitialTime(CCharAnimTime::ZeroFlat()) {}
  CAnimTreeTimeScale(const rstl::ncrc_ptr< CAnimTreeNode >& node,
                     const rstl::ownership_transfer< IVaryingAnimationTimeScale >& timeScale,
#if RSTL_CORRUPTION
                     const CCharAnimTime& time, const rstl::string& name)
  : CAnimTreeSingleChild(node, name)
  , mTimeScale(timeScale)
  , mCurAccelTime(0.f)
  , mTargetAccelTime(time)
  , mInitialTime(node->GetSteadyStateAnimInfo().GetDuration() - node->GetTimeRemaining()) {}
#else
                     const CCharAnimTime& time, const rstl::string& name);

#endif
  // IAnimReader
  // VAdvanceView is declared first so it, not the destructor, is the key function.
  SAdvancementResults VAdvanceView(const CCharAnimTime& time) override;

  ~CAnimTreeTimeScale() override;
  CCharAnimTime VGetTimeRemaining() const override;
  CSteadyStateAnimInfo VGetSteadyStateAnimInfo() const override;
  uint VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, uint capacity,
#if RSTL_CORRUPTION
                       uint iterator, const CSegIdMaskMap& mask) const override;
#else
                       uint iterator, int additive) const override;

  uint VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, uint capacity,
                        uint iterator, int additive) const override;

#endif
  uint VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, uint capacity,
#if RSTL_CORRUPTION
                           uint iterator, const CSegIdMaskMap& mask) const override;
#else
                           uint iterator, int additive) const override;

#endif
  uint VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, uint capacity,
#if RSTL_CORRUPTION
                        uint iterator, const CSegIdMaskMap& mask) const override;
  uint VGetAssetSoundPOIList(const CCharAnimTime& time, CAssetSoundPOINode* listOut, uint capacity,
                             uint iterator, const CSegIdMaskMap& mask) const override;
  uint VGetAssetPOIList(const CCharAnimTime& time, CAssetPOINode* listOut, uint capacity,
                        uint iterator, const CSegIdMaskMap& mask) const override;
  bool VGetBoolPOIState(uint nameHash) const override;
  CParticleData::EParentedMode VGetParticlePOIState(uint nameHash) const override;
#else
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

#endif
  rstl::ownership_transfer< IAnimReader > VClone() const override;
  rstl::optional_object< rstl::ownership_transfer< IAnimReader > > VSimplified() override;
  void VSetPhase(float phase) override;
  // CAnimTreeNode
  CAnimTreeEffectiveContribution VGetContributionOfHighestInfluence() const override;
  rstl::rc_ptr< CAnimTreeNode > VGetBestUnblendedChild() const override;

  CCharAnimTime GetRealLifeTime(const CCharAnimTime& time) const;

  static rstl::string CreatePrimitiveName(const rstl::ncrc_ptr< CAnimTreeNode >& node, float scaleA,
                                          const CCharAnimTime& time, float scaleB);

private:
  CAnimTreeTimeScale(const rstl::ncrc_ptr< CAnimTreeNode >& node,
                     const rstl::ownership_transfer< IVaryingAnimationTimeScale >& timeScale,
                     const CCharAnimTime& curTime, const CCharAnimTime& targetTime,
                     const CCharAnimTime& initialTime, const rstl::string& name)
  : CAnimTreeSingleChild(node, name)
  , mTimeScale(timeScale)
  , mCurAccelTime(curTime)
  , mTargetAccelTime(targetTime)
  , mInitialTime(initialTime) {}

  rstl::object_owner< IVaryingAnimationTimeScale > mTimeScale;
  CCharAnimTime mCurAccelTime;
  CCharAnimTime mTargetAccelTime;
  CCharAnimTime mInitialTime;
};
#if RSTL_CORRUPTION
CHECK_SIZEOF(CAnimTreeTimeScale, 0x3c)
#else
CHECK_CHILD_SIZEOF(CAnimTreeTimeScale, CAnimTreeSingleChild, 0x20)
#endif

#if !RSTL_CORRUPTION
#if !RSTL_PRIME1
inline CAnimTreeTimeScale::CAnimTreeTimeScale(
    const rstl::ncrc_ptr< CAnimTreeNode >& node,
    const rstl::ownership_transfer< IVaryingAnimationTimeScale >& timeScale,
    const CCharAnimTime& time, const rstl::string& name)
: CAnimTreeSingleChild(node, name)
, mTimeScale(timeScale)
, mCurAccelTime(0.f)
, mTargetAccelTime(time)
, mInitialTime(node->GetSteadyStateAnimInfo().GetDuration() - node->GetTimeRemaining()) {}
#endif
#endif

#endif // _CANIMTREETIMESCALE
