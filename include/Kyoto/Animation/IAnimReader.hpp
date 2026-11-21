#ifndef _IANIMREADER
#define _IANIMREADER
#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"
#endif

#if RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#endif
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "Kyoto/Animation/CAdvancementDeltas.hpp"
#include "Kyoto/Animation/CCharAnimTime.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/Animation/CPrimitive.hpp"
#endif
#include "Kyoto/Animation/CSteadyStateAnimInfo.hpp"

#include "Kyoto/Particles/CParticleData.hpp"
#if !RSTL_CORRUPTION

#include "rstl/auto_ptr.hpp"
#endif

#include "rstl/math.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/ownership_transfer.hpp"
#include "rstl/string.hpp"

#if !RSTL_CORRUPTION
struct SAdvancementResults {
#endif

#if !RSTL_CORRUPTION
  CCharAnimTime mRemTime;
  CAdvancementDeltas mDeltas;
#endif
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_CORRUPTION
class CSegIdMaskMap;


struct SAdvancementResults {
  CCharAnimTime mRemTime;
  CAdvancementDeltas mDeltas;

#else
#if RSTL_PRIME1
  const CAdvancementDeltas& GetAdvancementDeltas() const { return mDeltas; }
#endif
#endif
  SAdvancementResults() {}
#if RSTL_CORRUPTION
  SAdvancementResults(const CCharAnimTime& time, const CAdvancementDeltas& deltas)
#else
  explicit SAdvancementResults(const CCharAnimTime& time)
  : mRemTime(time)
#if !RSTL_PRIME1
  , mDeltas(CVector3f::Zero(), CQuaternion::NoRotation())
#endif
  {
  }
  SAdvancementResults(const CCharAnimTime& time, const CAdvancementDeltas& deltas)
#endif
  : mRemTime(time), mDeltas(deltas) {}

  const CCharAnimTime& GetRemainder() const { return mRemTime; }
#if RSTL_CORRUPTION
  explicit SAdvancementResults(const CCharAnimTime& time)
  : mRemTime(time), mDeltas(CVector3f::Zero(), CQuaternion::NoRotation()) {}

  // Guessed name
#endif
  static SAdvancementResults RemainderOnly(const CCharAnimTime& time) {
    return SAdvancementResults(time);
  }
};
CHECK_SIZEOF(SAdvancementResults, 0x24)

struct CAnimTreeEffectiveContribution {
  float mContributionWeight;
#if RSTL_CORRUPTION
  CPrimitive mPrimitive;
#else
  rstl::string mName;
#endif
  CSteadyStateAnimInfo mSsInfo;
  CCharAnimTime mRemTime;
#if RSTL_CORRUPTION
  uint x58_; // Guessed name; copied by the native copy constructor.
#else
  u32 mDbIdx;
#endif

public:
#if RSTL_CORRUPTION
  CAnimTreeEffectiveContribution(float cweight, const CPrimitive& primitive,
                                 const CSteadyStateAnimInfo& ssInfo, const CCharAnimTime& remTime)
  : mContributionWeight(cweight), mPrimitive(primitive), mSsInfo(ssInfo), mRemTime(remTime) {}
#else
  CAnimTreeEffectiveContribution(float cweight, const rstl::string& name,
                                 const CSteadyStateAnimInfo& ssInfo, const CCharAnimTime& remTime,
                                 u32 dbIdx)
  : mContributionWeight(cweight), mName(name), mSsInfo(ssInfo), mRemTime(remTime), mDbIdx(dbIdx) {}

#endif
  float GetContributionWeight() const { return mContributionWeight; }
#if RSTL_CORRUPTION
  const CPrimitive& GetPrimitive() const { return mPrimitive; }
#else
  const rstl::string& GetPrimitiveName() const { return mName; }
#endif
  const CSteadyStateAnimInfo& GetSteadyStateAnimInfo() const { return mSsInfo; }
  const CCharAnimTime& GetTimeRemaining() const { return mRemTime; }
#if !RSTL_CORRUPTION
  u32 GetAnimDatabaseIndex() const { return mDbIdx; }
#endif
  float GetPhase() const {
    return rstl::min_val(rstl::max_val(1.f - mRemTime / mSsInfo.GetDuration(), 0.f), 1.f);
  }
};
#if RSTL_CORRUPTION
CHECK_SIZEOF(CAnimTreeEffectiveContribution, 0x60)
#endif

class CSegId;
class CSegIdList;
#if RSTL_CORRUPTION
class CAssetPOINode;
class CAssetSoundPOINode;
#endif
class CBoolPOINode;
#if !RSTL_CORRUPTION
class CInt32POINode;
#endif
class CParticlePOINode;
class CSoundPOINode;
class CSegStatementSet;
class CCharLayoutInfo;
class CJointData_LinearStorage;
#if RSTL_CORRUPTION
class CVisibilityAnimationDataReader;
#endif

class IAnimReader {
public:
  virtual ~IAnimReader();
  virtual bool IsCAnimTreeNode() const;
  virtual SAdvancementResults VAdvanceView(const CCharAnimTime& a) = 0;

  virtual CCharAnimTime VGetTimeRemaining() const = 0;
  virtual CSteadyStateAnimInfo VGetSteadyStateAnimInfo() const = 0;
  virtual bool VHasOffset(const CSegId& seg) const = 0;
  virtual CVector3f VGetOffset(const CSegId& seg) const = 0;
  virtual CQuaternion VGetRotation(const CSegId& seg) const = 0;
  virtual uint VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, uint capacity,
#if RSTL_CORRUPTION
                               uint iterator, const CSegIdMaskMap& mask) const = 0;
#else
                               uint iterator, int additive) const = 0;

  virtual uint VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, uint capacity,
                                uint iterator, int additive) const = 0;

#endif
  virtual uint VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut,
#if RSTL_CORRUPTION
                                   uint capacity, uint iterator, const CSegIdMaskMap& mask) const = 0;
#else
                                   uint capacity, uint iterator, int additive) const = 0;

#endif
  virtual uint VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, uint capacity,
#if RSTL_CORRUPTION
                                uint iterator, const CSegIdMaskMap& mask) const = 0;
  virtual uint VGetAssetSoundPOIList(const CCharAnimTime& time, CAssetSoundPOINode* listOut,
                                     uint capacity, uint iterator,
                                     const CSegIdMaskMap& mask) const = 0; // Guessed name
  virtual uint VGetAssetPOIList(const CCharAnimTime& time, CAssetPOINode* listOut, uint capacity,
                                uint iterator, const CSegIdMaskMap& mask) const = 0; // Guessed name
  virtual bool VGetBoolPOIState(uint nameHash) const = 0;
  virtual CParticleData::EParentedMode VGetParticlePOIState(uint nameHash) const = 0;
#else
                                uint iterator, int additive) const = 0;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  virtual bool VGetBoolPOIState(const char* name) const = 0;
  virtual s32 VGetInt32POIState(const char* name) const = 0;
  virtual CParticleData::EParentedMode VGetParticlePOIState(const char* name) const = 0;
#else
  virtual bool VGetBoolPOIState(uint nameHash) const = 0;
  virtual s32 VGetInt32POIState(uint nameHash) const = 0;
  virtual CParticleData::EParentedMode VGetParticlePOIState(uint nameHash) const = 0;
#endif

#endif
  virtual void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut) const = 0;
  virtual void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut,
                                   const CCharAnimTime& time) const = 0;
#if RSTL_CORRUPTION
  // Guessed names.
  virtual void VGetJointData_Linear(const CCharLayoutInfo& layout, CJointData_LinearStorage& data,
                                    const CCharAnimTime& time) const;
  virtual void VGetJointData_Linear(const CCharLayoutInfo& layout,
                                    CJointData_LinearStorage& data) const;
  virtual const CVisibilityAnimationDataReader*
  VGetVisibilityDataAtCurrentTime() const = 0; // Guessed name
  virtual const CVisibilityAnimationDataReader*
  VGetVisibilityDataAtTime(const CCharAnimTime& time) const = 0; // Guessed name
#else
#if !RSTL_PRIME1
  // Guessed names.
  virtual void VGetJointData_Linear(const CCharLayoutInfo& layout, CJointData_LinearStorage& data,
                                    const CCharAnimTime& time) const;
  virtual void VGetJointData_Linear(const CCharLayoutInfo& layout,
                                    CJointData_LinearStorage& data) const;
#endif

#endif
  virtual rstl::ownership_transfer< IAnimReader > VClone() const = 0;
  virtual rstl::optional_object< rstl::ownership_transfer< IAnimReader > > VSimplified();
#if !RSTL_CORRUPTION
  rstl::optional_object< rstl::ownership_transfer< IAnimReader > > Simplified() {
    return VSimplified();
  }
#endif
  virtual void VSetPhase(float phase) = 0;
  virtual SAdvancementResults VGetAdvancementResults(const CCharAnimTime& time,
                                                     const CCharAnimTime& startOffset) const;
  rstl::ownership_transfer< IAnimReader > Clone() const { return VClone(); }
#if RSTL_CORRUPTION
  rstl::optional_object< rstl::ownership_transfer< IAnimReader > > Simplified() {
    return VSimplified();
  }
#endif
  SAdvancementResults AdvanceView(const CCharAnimTime& time) { return VAdvanceView(time); }
  CCharAnimTime GetTimeRemaining() const { return VGetTimeRemaining(); }
  CSteadyStateAnimInfo GetSteadyStateAnimInfo() const { return VGetSteadyStateAnimInfo(); }
#if !RSTL_PRIME1
  bool GetBoolPOIState(uint nameHash) const { return VGetBoolPOIState(nameHash); }
#endif

  uint GetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, uint capacity,
#if RSTL_CORRUPTION
                      uint iterator, const CSegIdMaskMap& mask) const;
#else
                      uint iterator, int additive) const;

  uint GetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, uint capacity,
                       uint iterator, int additive) const;

#endif
  uint GetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, uint capacity,
#if RSTL_CORRUPTION
                          uint iterator, const CSegIdMaskMap& mask) const;
#else
                          uint iterator, int additive) const;

#endif
  uint GetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, uint capacity,
#if RSTL_CORRUPTION
                       uint iterator, const CSegIdMaskMap& mask) const;
  uint GetAssetSoundPOIList(const CCharAnimTime& time, CAssetSoundPOINode* listOut, uint capacity,
                            uint iterator, const CSegIdMaskMap& mask) const; // Guessed name
  uint GetAssetPOIList(const CCharAnimTime& time, CAssetPOINode* listOut, uint capacity,
                       uint iterator, const CSegIdMaskMap& mask) const;                      // Guessed name
  const CVisibilityAnimationDataReader* GetVisibilityDataAtCurrentTime() const; // Guessed name
  const CVisibilityAnimationDataReader*
  GetVisibilityDataAtTime(const CCharAnimTime& time) const; // Guessed name
};
CHECK_SIZEOF(IAnimReader, 0x4)
#else
                       uint iterator, int additive) const;
#endif

// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_CORRUPTION
#if RSTL_PRIME1
  bool HasOffset(const CSegId& seg) const { return VHasOffset(seg); }
  CVector3f GetOffset(const CSegId& seg) const { return VGetOffset(seg); }
  CQuaternion GetRotation(const CSegId& seg) const { return VGetRotation(seg); }
  SAdvancementResults GetAdvancementResults(const CCharAnimTime& a, const CCharAnimTime& b) const {
    return VGetAdvancementResults(a, b);
  }
  void GetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut) const {
    VGetSegStatementSet(list, setOut);
  }
  void GetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut,
                          const CCharAnimTime& time) const {
    VGetSegStatementSet(list, setOut, time);
  }
  void SetPhase(float phase) { VSetPhase(phase); }
  bool GetBoolPOIState(const char* name) const { return VGetBoolPOIState(name); }
  s32 GetInt32POIState(const char* name) const { return VGetInt32POIState(name); }
  CParticleData::EParentedMode GetParticlePOIState(const char* name) const {
    return VGetParticlePOIState(name);
  }
#endif
};
DECLARE_FULL_SIZE_FOR(IAnimReader, 0x4)
CHECK_SIZEOF(IAnimReader, IAnimReader_FULL_SIZE)

#endif
#endif // _IANIMREADER
