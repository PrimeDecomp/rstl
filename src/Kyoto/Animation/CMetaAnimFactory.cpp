#include "rstl/RstlVersions.h"
#include "rstl/RstlVersions.h"
#include "Kyoto/Alloc/CMemory.hpp"
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
    return rs_new_line(21) CMetaAnimPlay(in);
  case kMAT_Blend:
    return rs_new_line(27) CMetaAnimBlend(in);
  case kMAT_PhaseBlend:
    return rs_new_line(33) CMetaAnimPhaseBlend(in);
  case kMAT_Random:
    return rs_new_line(39) CMetaAnimRandom(in);
  case kMAT_Sequence:
    return rs_new_line(45) CMetaAnimSequence(in);
  default:
    return rstl::rc_ptr< IMetaAnim >();
  }
}
