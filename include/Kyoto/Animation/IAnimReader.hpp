#ifndef _IANIMREADER
#define _IANIMREADER
#include "rstl/RstlVersions.h"

#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "Kyoto/Animation/CAdvancementDeltas.hpp"
#include "Kyoto/Animation/CCharAnimTime.hpp"
#include "Kyoto/Animation/CSteadyStateAnimInfo.hpp"

#include "Kyoto/Particles/CParticleData.hpp"

#include "rstl/auto_ptr.hpp"

#include "rstl/math.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/ownership_transfer.hpp"
#include "rstl/string.hpp"

struct SAdvancementResults {

  CCharAnimTime mRemTime;
  CAdvancementDeltas mDeltas;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const CCharAnimTime& GetRemainder() const { return mRemTime; }
  const CAdvancementDeltas& GetAdvancementDeltas() const { return mDeltas; }
#endif
  SAdvancementResults() {}
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  SAdvancementResults(const CCharAnimTime& time) : mRemTime(time) {}
#endif
  SAdvancementResults(const CCharAnimTime& time, const CAdvancementDeltas& deltas)
  : mRemTime(time), mDeltas(deltas) {}

// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
  const CCharAnimTime& GetRemainder() const { return mRemTime; }
  explicit SAdvancementResults(const CCharAnimTime& time)
  : mRemTime(time), mDeltas(CVector3f::Zero(), CQuaternion::NoRotation()) {}
#endif
  static SAdvancementResults RemainderOnly(const CCharAnimTime& time) {
    return SAdvancementResults(time);
  }
};
CHECK_SIZEOF(SAdvancementResults, 0x24)

struct CAnimTreeEffectiveContribution {
  float mContributionWeight;
  rstl::string mName;
  CSteadyStateAnimInfo mSsInfo;
  CCharAnimTime mRemTime;
  u32 mDbIdx;

public:
  CAnimTreeEffectiveContribution(float cweight, const rstl::string& name,
                                 const CSteadyStateAnimInfo& ssInfo, const CCharAnimTime& remTime,
                                 u32 dbIdx)
  : mContributionWeight(cweight)
  , mName(name)
  , mSsInfo(ssInfo)
  , mRemTime(remTime)
  , mDbIdx(dbIdx) {}

  float GetContributionWeight() const { return mContributionWeight; }
  const rstl::string& GetPrimitiveName() const { return mName; }
  const CSteadyStateAnimInfo& GetSteadyStateAnimInfo() const { return mSsInfo; }
  const CCharAnimTime& GetTimeRemaining() const { return mRemTime; }
  u32 GetAnimDatabaseIndex() const { return mDbIdx; }
  float GetPhase() const {
    return rstl::min_val(rstl::max_val(1.f - mRemTime / mSsInfo.GetDuration(), 0.f), 1.f);
  }
};

class CSegId;
class CSegIdList;
class CBoolPOINode;
class CInt32POINode;
class CParticlePOINode;
class CSoundPOINode;
class CSegStatementSet;
class CCharLayoutInfo;
class CJointData_LinearStorage;

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
                               uint iterator, int additive) const = 0;

  virtual uint VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, uint capacity,
                                uint iterator, int additive) const = 0;

  virtual uint VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut,
                                   uint capacity, uint iterator, int additive) const = 0;

  virtual uint VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, uint capacity,
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

  virtual void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut) const = 0;
  virtual void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut,
                                   const CCharAnimTime& time) const = 0;
#if !RSTL_PRIME1
  // Guessed names.
  virtual void VGetJointData_Linear(const CCharLayoutInfo& layout, CJointData_LinearStorage& data,
                                    const CCharAnimTime& time) const;
  virtual void VGetJointData_Linear(const CCharLayoutInfo& layout,
                                    CJointData_LinearStorage& data) const;
#endif

  virtual rstl::ownership_transfer< IAnimReader > VClone() const = 0;
  virtual rstl::optional_object< rstl::ownership_transfer< IAnimReader > > VSimplified();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  rstl::optional_object< rstl::ownership_transfer< IAnimReader > > Simplified();
#endif
  virtual void VSetPhase(float phase) = 0;
  virtual SAdvancementResults VGetAdvancementResults(const CCharAnimTime& time,
                                                     const CCharAnimTime& startOffset) const;
#if !RSTL_PRIME1

  rstl::ownership_transfer< IAnimReader > Clone() const { return VClone(); }
  rstl::optional_object< rstl::ownership_transfer< IAnimReader > > Simplified() {
    return VSimplified();
  }
  SAdvancementResults AdvanceView(const CCharAnimTime& time) { return VAdvanceView(time); }
  CCharAnimTime GetTimeRemaining() const { return VGetTimeRemaining(); }
  CSteadyStateAnimInfo GetSteadyStateAnimInfo() const { return VGetSteadyStateAnimInfo(); }
  bool GetBoolPOIState(uint nameHash) const { return VGetBoolPOIState(nameHash); }
#endif

  uint GetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, uint capacity,
                      uint iterator, int additive) const;

  uint GetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, uint capacity,
                       uint iterator, int additive) const;

  uint GetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, uint capacity,
                          uint iterator, int additive) const;

  uint GetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, uint capacity,
                       uint iterator, int additive) const;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1

  rstl::ownership_transfer< IAnimReader > Clone() const { return VClone(); }
  CCharAnimTime GetTimeRemaining() const { return VGetTimeRemaining(); }
  bool HasOffset(const CSegId& seg) const { return VHasOffset(seg); }
  CVector3f GetOffset(const CSegId& seg) const { return VGetOffset(seg); }
  CQuaternion GetRotation(const CSegId& seg) const { return VGetRotation(seg); }
  SAdvancementResults GetAdvancementResults(const CCharAnimTime& a, const CCharAnimTime& b) const {
    return VGetAdvancementResults(a, b);
  }
  CSteadyStateAnimInfo GetSteadyStateAnimInfo() const { return VGetSteadyStateAnimInfo(); }
  void GetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut) const {
    VGetSegStatementSet(list, setOut);
  }
  void GetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut,
                          const CCharAnimTime& time) const {
    VGetSegStatementSet(list, setOut, time);
  }
  void SetPhase(float phase) { VSetPhase(phase); }
  SAdvancementResults AdvanceView(const CCharAnimTime& time) { return VAdvanceView(time); }
  bool GetBoolPOIState(const char* name) const { return VGetBoolPOIState(name); }
  s32 GetInt32POIState(const char* name) const { return VGetInt32POIState(name); }
  CParticleData::EParentedMode GetParticlePOIState(const char* name) const {
    return VGetParticlePOIState(name);
  }
#endif
};
DECLARE_FULL_SIZE_FOR(IAnimReader, 0x4)
CHECK_SIZEOF(IAnimReader, IAnimReader_FULL_SIZE)

#endif // _IANIMREADER
