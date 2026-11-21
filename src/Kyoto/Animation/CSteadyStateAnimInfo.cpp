#include "rstl/RstlVersions.h"
#if RSTL_KYOTO_PROTO_UNIT
#include "Kyoto/Animation/CSteadyStateAnimInfo.hpp"



CSteadyStateAnimInfo::CSteadyStateAnimInfo(bool looping, const CVector3f& offset,
                                           const rstl::vector< CCharAnimTime >& segmentDurations)
: mDuration(CCharAnimTime::ZeroFlat())
, mSegmentDurations(segmentDurations)
, mOffset(offset)
, mLooping(looping) {
  for (rstl::vector< CCharAnimTime >::const_iterator it = segmentDurations.begin();
       it != segmentDurations.end(); ++it) {
    mDuration += *it;
  }
}

CCharAnimTime CSteadyStateAnimInfo::GetDurationFromIndex(int index) const {
  if (index > 0) {
    CCharAnimTime duration = CCharAnimTime::ZeroFlat();
    for (int i = index; i < mSegmentDurations.size(); ++i) {
      duration += mSegmentDurations[i];
    }
    return duration;
  }
  return mDuration;
}
#endif
