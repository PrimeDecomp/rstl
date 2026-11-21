#include "Kyoto/Audio/CSoundEvaluator.hpp"

#include "Kyoto/Alloc/Assert.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/COutputStream.hpp"



CSoundEvaluator::CSoundEvaluator() {}

CSoundEvaluator::~CSoundEvaluator() {}

CSoundEvaluatorConstant::CSoundEvaluatorConstant(CInputStream& in) : mValue(in.ReadFloat()) {}

CSoundEvaluatorConstant::~CSoundEvaluatorConstant() {}

float CSoundEvaluatorConstant::Evaluate(float time) const { return mValue; }

void CSoundEvaluatorConstant::WriteToStream(COutputStream& out) const {
  out.WriteInt32(1);
  out.Put(mValue);
}

CSoundEvaluatorSpline::CSoundEvaluatorSpline(CInputStream& in) : mSpline(in) {}

CSoundEvaluatorSpline::~CSoundEvaluatorSpline() {}

float CSoundEvaluatorSpline::Evaluate(float time) const { return mSpline.EvaluateAt(time); }

void CSoundEvaluatorSpline::WriteToStream(COutputStream& out) const {
  out.WriteInt32(2);
  mSpline.WriteToStream(out);
}

rstl::rc_ptr< CSoundEvaluator > CSoundEvaluatorFactory::GetSoundEvaluator(CInputStream& in) {
  switch (in.ReadInt32()) {
  case 0:
    return rstl::rc_ptr< CSoundEvaluator >(nullptr);
  case 1:
    return rstl::rc_ptr< CSoundEvaluator >(rs_new_line(84) CSoundEvaluatorConstant(in));
  case 2:
    return rstl::rc_ptr< CSoundEvaluator >(rs_new_line(90) CSoundEvaluatorSpline(in));
  default:
    RS_VERIFY_FAILURE(97, "false", "0",
                      "CSoundEvaluatorFactory::GetSoundEvaluator -- Unknown sound evaluator type!");
    return rstl::rc_ptr< CSoundEvaluator >(nullptr);
  }
}
