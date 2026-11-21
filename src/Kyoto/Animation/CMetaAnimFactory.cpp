#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/RstlVersions.h"
#endif
#include "Kyoto/Animation/CMetaAnimFactory.hpp"
#include "Kyoto/Animation/CCharAnimTime.hpp"
#include "Kyoto/Animation/CMetaAnimBlend.hpp"
#include "Kyoto/Animation/CMetaAnimPhaseBlend.hpp"
#include "Kyoto/Animation/CMetaAnimPlay.hpp"
#include "Kyoto/Animation/CMetaAnimRandom.hpp"
#include "Kyoto/Animation/CMetaAnimSequence.hpp"
#include "Kyoto/Animation/IMetaAnim.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

rstl::rc_ptr< IMetaAnim > CMetaAnimFactory::CreateMetaAnim(CInputStream& in) {
#if RSTL_CORRUPTION
  EMetaAnimType type = static_cast< EMetaAnimType >(in.ReadInt32());
#else
  EMetaAnimType type = static_cast< EMetaAnimType >(in.ReadLong());
#endif
  switch (type) {
  case kMAT_Play:
#if RSTL_CORRUPTION
    return rs_new_line(21) CMetaAnimPlay(in);
#else
    return rs_new CMetaAnimPlay(in);
#endif
  case kMAT_Blend:
#if RSTL_CORRUPTION
    return rs_new_line(27) CMetaAnimBlend(in);
#else
    return rs_new CMetaAnimBlend(in);
#endif
  case kMAT_PhaseBlend:
#if RSTL_CORRUPTION
    return rs_new_line(33) CMetaAnimPhaseBlend(in);
#else
    return rs_new CMetaAnimPhaseBlend(in);
#endif
  case kMAT_Random:
#if RSTL_CORRUPTION
    return rs_new_line(39) CMetaAnimRandom(in);
#else
    return rs_new CMetaAnimRandom(in);
#endif
  case kMAT_Sequence:
#if RSTL_CORRUPTION
    return rs_new_line(45) CMetaAnimSequence(in);
#else
    return rs_new CMetaAnimSequence(in);
#endif
  default:
    return rstl::rc_ptr< IMetaAnim >();
  }
}
