#ifndef MP1_CSTEADYSTATEANIMINFO_HPP
#define MP1_CSTEADYSTATEANIMINFO_HPP
#include "rstl/RstlVersions.h"

#include "rstl/static_assert.hpp"
#include "Kyoto/Animation/CCharAnimTime.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#if RSTL_CORRUPTION
#include "rstl/vector.hpp"
#else
class CSteadyStateAnimInfo {
  CCharAnimTime mDuration;
  CVector3f mOffset;
  bool mLooping;
#endif

#if RSTL_CORRUPTION


class CSteadyStateAnimInfo {
#endif
public:
  CSteadyStateAnimInfo(bool looping, const CCharAnimTime& duration, const CVector3f& offset)
  : mDuration(duration), mOffset(offset), mLooping(looping) {}
#if RSTL_CORRUPTION
  CSteadyStateAnimInfo(bool looping, const CVector3f& offset,
                       const rstl::vector< CCharAnimTime >& segmentDurations);
#endif

  CCharAnimTime GetDuration() const { return mDuration; }
#if RSTL_CORRUPTION
  CCharAnimTime GetDurationFromIndex(int index) const; // Guessed name
#endif
  CVector3f GetOffset() const { return mOffset; }
  const bool IsLooping() const { return mLooping; }
#if RSTL_CORRUPTION
  const rstl::vector< CCharAnimTime >& GetSegmentDurations() const { // Guessed name
    return mSegmentDurations;
  }
#else
};
CHECK_SIZEOF(CSteadyStateAnimInfo, 0x18)
#endif

#if RSTL_CORRUPTION
private:
  CCharAnimTime mDuration;
  rstl::vector< CCharAnimTime > mSegmentDurations; // Guessed name
  CVector3f mOffset;
  bool mLooping;
};
CHECK_SIZEOF(CSteadyStateAnimInfo, 0x28)


#endif
#endif // MP1_CSTEADYSTATEANIMINFO_HPP
