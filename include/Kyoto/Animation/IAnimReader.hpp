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

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
struct CAdvancementResults {
#else
typedef CAdvancementDeltas SAdvancementDeltas;

struct SAdvancementResults {
#endif

  CCharAnimTime mRemTime;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CAdvancementDeltas mDeltas;
  const CCharAnimTime& GetRemainder() const { return mRemTime; }
  const CAdvancementDeltas& GetAdvancementDeltas() const { return mDeltas; }
  CAdvancementResults() {}
  CAdvancementResults(const CCharAnimTime& time) : mRemTime(time) {}
  CAdvancementResults(const CCharAnimTime& time, const CAdvancementDeltas& deltas)
#else
  SAdvancementDeltas mDeltas;

  SAdvancementResults() {}
  SAdvancementResults(const CCharAnimTime& time, const SAdvancementDeltas& deltas)
#endif

  : mRemTime(time), mDeltas(deltas) {}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static CAdvancementResults RemainderOnly(const CCharAnimTime& time) {
    return CAdvancementResults(time);
#else
  const CCharAnimTime& GetRemainder() const { return mRemTime; }
  explicit SAdvancementResults(const CCharAnimTime& time)
  : mRemTime(time), mDeltas(CVector3f::Zero(), CQuaternion::NoRotation()) {}

  static SAdvancementResults RemainderOnly(const CCharAnimTime& time) {
    return SAdvancementResults(time);
#endif

  }
};
#if !RSTL_PRIME1
CHECK_SIZEOF(SAdvancementResults, 0x24)
#endif

#if RSTL_PRIME1
CHECK_SIZEOF(CAdvancementResults, 0x24)
#endif

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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  : mContributionWeight(cweight)
  , mName(name)
  , mSsInfo(ssInfo)
  , mRemTime(remTime)
  , mDbIdx(dbIdx) {}
#else
  : mContributionWeight(cweight), mName(name), mSsInfo(ssInfo), mRemTime(remTime), mDbIdx(dbIdx) {}
#endif

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
#if !RSTL_PRIME1
class CCharLayoutInfo;
class CJointData_LinearStorage;
#endif

class IAnimReader {
public:
  virtual ~IAnimReader();
  virtual bool IsCAnimTreeNode() const;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  virtual CAdvancementResults VAdvanceView(const CCharAnimTime& a) = 0;
#else
  virtual SAdvancementResults VAdvanceView(const CCharAnimTime& a) = 0;
#endif

  virtual CCharAnimTime VGetTimeRemaining() const = 0;
  virtual CSteadyStateAnimInfo VGetSteadyStateAnimInfo() const = 0;
  virtual bool VHasOffset(const CSegId& seg) const = 0;
  virtual CVector3f VGetOffset(const CSegId& seg) const = 0;
  virtual CQuaternion VGetRotation(const CSegId& seg) const = 0;
  virtual uint VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, uint capacity,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                               uint iterator, int) const = 0;
#else
                               uint iterator, int additive) const = 0;
#endif

  virtual uint VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, uint capacity,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                                uint iterator, int) const = 0;
#else
                                uint iterator, int additive) const = 0;
#endif

  virtual uint VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                                   uint capacity, uint iterator, int) const = 0;
#else
                                   uint capacity, uint iterator, int additive) const = 0;
#endif

  virtual uint VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, uint capacity,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                                uint iterator, int) const = 0;
  virtual bool VGetBoolPOIState(const char* name) const = 0;
  virtual s32 VGetInt32POIState(const char* name) const = 0;
  virtual CParticleData::EParentedMode VGetParticlePOIState(const char* name) const = 0;
#else
                                uint iterator, int additive) const = 0;
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
  virtual void VSetPhase(float) = 0;
  virtual CAdvancementResults VGetAdvancementResults(const CCharAnimTime& aTime,
                                                     const CCharAnimTime& bTime) const;
#else
  virtual void VSetPhase(float phase) = 0;
  virtual SAdvancementResults VGetAdvancementResults(const CCharAnimTime& time,
                                                     const CCharAnimTime& startOffset) const;

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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                      uint iterator, int unk) const;
#else
                      uint iterator, int additive) const;
#endif

  uint GetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, uint capacity,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                       uint iterator, int unk) const;
#else
                       uint iterator, int additive) const;
#endif

  uint GetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, uint capacity,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                          uint iterator, int unk) const;
#else
                          uint iterator, int additive) const;
#endif

  uint GetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, uint capacity,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                       uint iterator, int unk) const;

  rstl::ownership_transfer< IAnimReader > Clone() const { return VClone(); }
  CCharAnimTime GetTimeRemaining() const { return VGetTimeRemaining(); }
  bool HasOffset(const CSegId& seg) const { return VHasOffset(seg); }
  CVector3f GetOffset(const CSegId& seg) const { return VGetOffset(seg); }
  CQuaternion GetRotation(const CSegId& seg) const { return VGetRotation(seg); }
  CAdvancementResults GetAdvancementResults(const CCharAnimTime& a, const CCharAnimTime& b) const {
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
  CAdvancementResults AdvanceView(const CCharAnimTime& time) { return VAdvanceView(time); }
  bool GetBoolPOIState(const char* name) const { return VGetBoolPOIState(name); }
  s32 GetInt32POIState(const char* name) const { return VGetInt32POIState(name); }
  CParticleData::EParentedMode GetParticlePOIState(const char* name) const {
    return VGetParticlePOIState(name);
  }
#else
                       uint iterator, int additive) const;
#endif

};
#if !RSTL_PRIME1
DECLARE_FULL_SIZE_FOR(IAnimReader, 0x4)
CHECK_SIZEOF(IAnimReader, IAnimReader_FULL_SIZE)
#endif

#endif // _IANIMREADER
