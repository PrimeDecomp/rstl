#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/RstlVersions.h"
#endif
#include "Kyoto/Animation/CAnimTreeBlend.hpp"

rstl::ownership_transfer< IAnimReader > CAnimTreeBlend::VClone() const {
  return rs_new_line(36) CAnimTreeBlend(CharacterSpaceBlend(), Cast(mA->Clone()), Cast(mB->Clone()),
                               mBlendWeight, mName);
}

float CAnimTreeBlend::VGetBlendingWeight() const { return mBlendWeight; }

CCharAnimTime CAnimTreeBlend::VGetTimeRemaining() const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return rstl::max_val(mA->GetTimeRemaining(), mB->GetTimeRemaining());
#else
  return rstl::max_val< const CCharAnimTime& >(mA->VGetTimeRemaining(), mB->VGetTimeRemaining());
#endif
}

CSteadyStateAnimInfo CAnimTreeBlend::VGetSteadyStateAnimInfo() const {
  CSteadyStateAnimInfo infoA = mA->GetSteadyStateAnimInfo();
  CSteadyStateAnimInfo infoB = mB->GetSteadyStateAnimInfo();
  CVector3f offsetA = infoA.GetOffset();
  CVector3f offsetB = infoB.GetOffset();
  CCharAnimTime durationA = infoA.GetDuration();
  CCharAnimTime durationB = infoB.GetDuration();
#if RSTL_CORRUPTION
  CVector3f offset(0.f, 0.f, 0.f);
#else
  CVector3f offset;
#endif
  if (durationA < durationB) {
#if RSTL_CORRUPTION
    const float scale = durationB / durationA;
#else
    RSTL_ECHOES_CONST float scale = durationB / durationA;
#endif
    offset = offsetA * scale * mBlendWeight + offsetB * (1.f - mBlendWeight);
  } else if (durationB < durationA) {
#if RSTL_CORRUPTION
    const float scale = durationA / durationB;
#else
    RSTL_ECHOES_CONST float scale = durationA / durationB;
#endif
    offset = offsetA * mBlendWeight + offsetB * scale * (1.f - mBlendWeight);
  } else {
    offset = offsetA + offsetB;
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return CSteadyStateAnimInfo(infoB.IsLooping(),
                              rstl::max_val(infoA.GetDuration(), infoB.GetDuration()), offset);
#else
  return CSteadyStateAnimInfo(
      infoB.IsLooping(),
      rstl::max_val< const CCharAnimTime& >(infoA.GetDuration(), infoB.GetDuration()), offset);
#endif
}

rstl::string CAnimTreeBlend::CreatePrimitiveName(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                                 const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                                 float weight) {
  return rstl::string_l("");
}

void CAnimTreeBlend::SetBlendingWeight(float weight) { mBlendWeight = weight; }

SAdvancementResults CAnimTreeBlend::VAdvanceView(const CCharAnimTime& time) {

  IncAdvancementDepth();
  SAdvancementResults resA = mA->AdvanceView(time);
#if !RSTL_CORRUPTION
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION < RSTL_GM8E_02
  const CAdvancementDeltas& deltasA = resA.GetAdvancementDeltas();
#endif
#endif
#endif
  SAdvancementResults resB = mB->AdvanceView(time);
#if !RSTL_CORRUPTION
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION < RSTL_GM8E_02
  const CAdvancementDeltas& deltasB = resB.GetAdvancementDeltas();
#endif
#endif
#endif
  DecAdvancementDepth();
  if (ShouldCullTree()) {
    if (GetBlendingWeight() < 0.5f)
      mCullSelector = 1;
    else
      mCullSelector = 2;
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_CORRUPTION
  const CCharAnimTime remainder =
      rstl::max_val< const CCharAnimTime& >(resA.GetRemainder(), resB.GetRemainder());
  const CAdvancementDeltas& deltasA = resA.mDeltas;
  const CAdvancementDeltas& deltasB = resB.mDeltas;
  switch (GetBlendRoot() & (kBlendRoot_Offset | kBlendRoot_Rotation)) {
  case kBlendRoot_Offset:
    return SAdvancementResults(
        remainder, CAdvancementDeltas::BlendTranslationOnly(deltasA, deltasB, GetBlendingWeight()));
  case kBlendRoot_Rotation:
    return SAdvancementResults(
        remainder, CAdvancementDeltas::BlendRotationOnly(deltasA, deltasB, GetBlendingWeight()));
  case kBlendRoot_Offset | kBlendRoot_Rotation:
    return SAdvancementResults(
        remainder, CAdvancementDeltas::Blend(deltasA, deltasB, GetBlendingWeight()));
  default:
#else
#if RSTL_PRIME1
  CCharAnimTime remainder = rstl::max_val(resA.GetRemainder(), resB.GetRemainder());
#else
  const CCharAnimTime remainder =
      rstl::max_val< const CCharAnimTime& >(resA.GetRemainder(), resB.GetRemainder());
#endif
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_GM8E_02
  const CAdvancementDeltas& deltasA = resA.GetAdvancementDeltas();
  const CAdvancementDeltas& deltasB = resB.GetAdvancementDeltas();
#endif
#else
  const CAdvancementDeltas& deltasA = resA.mDeltas;
  const CAdvancementDeltas& deltasB = resB.mDeltas;
#endif
  if (GetBlendRoot() & kBlendRoot_Offset)
    return SAdvancementResults(remainder,
                               CAdvancementDeltas::Blend(deltasA, deltasB, GetBlendingWeight()));

#endif
  return resB;
}
#if RSTL_CORRUPTION
}
#endif
