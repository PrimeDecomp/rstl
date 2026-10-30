#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

#include "rstl/vector.hpp"

class CAnimPerSegmentData {
public:
  CAnimPerSegmentData(CInputStream& in);

private:
  CSegId mSegId;
  rstl::vector< CQuaternion > mRotations;
};

CAnimPerSegmentData::CAnimPerSegmentData(CInputStream& in) : mSegId(in), mRotations(in) {}

