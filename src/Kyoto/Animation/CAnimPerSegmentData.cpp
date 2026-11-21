#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "Kyoto/Animation/CSegId.hpp"
#endif
#include "Kyoto/Math/CQuaternion.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/Math/CVector3f.hpp"
#endif
#include "Kyoto/Streams/CInputStream.hpp"

#include "rstl/vector.hpp"

class CAnimPerSegmentData { // Guessed name
public:
  CAnimPerSegmentData(CInputStream& in);

private:
#if !RSTL_CORRUPTION
  CSegId mSegId;
#endif
  rstl::vector< CQuaternion > mRotations;
#if RSTL_CORRUPTION
  rstl::vector< CVector3f > mTranslations;
#endif
};

#if RSTL_CORRUPTION
CAnimPerSegmentData::CAnimPerSegmentData(CInputStream& in) : mRotations(in), mTranslations(in) {}
#else
CAnimPerSegmentData::CAnimPerSegmentData(CInputStream& in) : mSegId(in), mRotations(in) {}
#endif
