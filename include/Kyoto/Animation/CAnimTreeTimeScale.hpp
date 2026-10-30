#ifndef _CANIMTREETIMESCALE
#define _CANIMTREETIMESCALE
#include "rstl/RstlVersions.h"

#include "Kyoto/Animation/CAnimTreeSingleChild.hpp"
#include "Kyoto/Animation/CTimeScaleFunctions.hpp"
#include "rstl/object_owner.hpp"

class CAnimTreeTimeScale : public CAnimTreeSingleChild {
public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CAdvancementResults VAdvanceView(const CCharAnimTime& time) override;
#else
  CAnimTreeTimeScale(const rstl::ncrc_ptr< CAnimTreeNode >& node, float scale,
                     const rstl::string& name)
  : CAnimTreeSingleChild(node, name)
  , mTimeScale(rs_new CConstantAnimationTimeScale(scale))
  , mCurAccelTime(0.f)
  , mTargetAccelTime(CCharAnimTime::Infinity())
  , mInitialTime(CCharAnimTime::ZeroFlat()) {}

  CAnimTreeTimeScale(const rstl::ncrc_ptr< CAnimTreeNode >& node,
                     const rstl::ownership_transfer< IVaryingAnimationTimeScale >& timeScale,
                     const CCharAnimTime& time, const rstl::string& name)
  : CAnimTreeSingleChild(node, name)
  , mTimeScale(timeScale)
  , mCurAccelTime(0.f)
  , mTargetAccelTime(time)
  , mInitialTime(node->GetSteadyStateAnimInfo().GetDuration() - node->GetTimeRemaining()) {}

  // IAnimReader
  // VAdvanceView is declared first so it, not the destructor, is the key function.
  SAdvancementResults VAdvanceView(const CCharAnimTime& time) override;
#endif

  ~CAnimTreeTimeScale() override;
  CCharAnimTime VGetTimeRemaining() const override;
  CSteadyStateAnimInfo VGetSteadyStateAnimInfo() const override;
  uint VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, uint capacity,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                       uint iterator, int unk) const override;
#else
                       uint iterator, int additive) const override;
#endif

  uint VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, uint capacity,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                        uint iterator, int unk) const override;
#else
                        uint iterator, int additive) const override;
#endif

  uint VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, uint capacity,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                           uint iterator, int unk) const override;
#else
                           uint iterator, int additive) const override;
#endif

  uint VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, uint capacity,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                        uint iterator, int unk) const override;
  bool VGetBoolPOIState(const char* name) const override;
  s32 VGetInt32POIState(const char* name) const override;
  CParticleData::EParentedMode VGetParticlePOIState(const char* name) const override;
#else
                        uint iterator, int additive) const override;
  bool VGetBoolPOIState(uint nameHash) const override;
  s32 VGetInt32POIState(uint nameHash) const override;
  CParticleData::EParentedMode VGetParticlePOIState(uint nameHash) const override;
#endif

  rstl::ownership_transfer< IAnimReader > VClone() const override;
  rstl::optional_object< rstl::ownership_transfer< IAnimReader > > VSimplified() override;
  void VSetPhase(float phase) override;
  // CAnimTreeNode
  CAnimTreeEffectiveContribution VGetContributionOfHighestInfluence() const override;
  rstl::rc_ptr< CAnimTreeNode > VGetBestUnblendedChild() const override;

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CAnimTreeTimeScale(const rstl::ncrc_ptr< CAnimTreeNode >& node, float timeScale,
                     const rstl::string& name)
  : CAnimTreeSingleChild(node, name)
  , mTimeScale(rs_new CConstantAnimationTimeScale(timeScale))
  , mCurAccelTime(0.f)
  , mTargetAccelTime(CCharAnimTime::Infinity())
  , mInitialTime(CCharAnimTime::ZeroFlat()) {}
  CAnimTreeTimeScale(const rstl::ncrc_ptr< CAnimTreeNode >& node,
                     const rstl::ownership_transfer< IVaryingAnimationTimeScale >& timeScale,
                     const CCharAnimTime& time, const rstl::string& name);
#else
  CCharAnimTime GetRealLifeTime(const CCharAnimTime& time) const;
#endif

  static rstl::string CreatePrimitiveName(const rstl::ncrc_ptr< CAnimTreeNode >& node, float scaleA,
                                          const CCharAnimTime& time, float scaleB);
#if RSTL_PRIME1
  CCharAnimTime GetRealLifeTime(const CCharAnimTime& time) const;
#endif

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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CHECK_SIZEOF(CAnimTreeTimeScale, 0x38)
#else
CHECK_CHILD_SIZEOF(CAnimTreeTimeScale, CAnimTreeSingleChild, 0x20)
#endif

#endif // _CANIMTREETIMESCALE
