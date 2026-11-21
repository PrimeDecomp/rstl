#ifndef _CSOUNDEVALUATOR
#define _CSOUNDEVALUATOR

#include "rstl/types.h"

#include "Kyoto/Math/CMayaSpline.hpp"

#include "rstl/rc_ptr.hpp"



class CInputStream;
class COutputStream;

// Guessed name. A serialized value that is either a constant or a curve over time.
class CSoundEvaluator {
public:
  CSoundEvaluator();
  virtual ~CSoundEvaluator();

  virtual float Evaluate(float time) const = 0;
  virtual void WriteToStream(COutputStream& out) const = 0;
};

// Guessed name.
class CSoundEvaluatorConstant : public CSoundEvaluator {
public:
  explicit CSoundEvaluatorConstant(CInputStream& in);
  ~CSoundEvaluatorConstant() override;

  // CSoundEvaluator
  float Evaluate(float time) const override;
  void WriteToStream(COutputStream& out) const override;

private:
  float mValue; // Guessed name.
};

// Guessed name.
class CSoundEvaluatorSpline : public CSoundEvaluator {
public:
  explicit CSoundEvaluatorSpline(CInputStream& in);
  ~CSoundEvaluatorSpline() override;

  // CSoundEvaluator
  float Evaluate(float time) const override;
  void WriteToStream(COutputStream& out) const override;

private:
  CMayaSpline mSpline; // Guessed name.
};

class CSoundEvaluatorFactory {
public:
  static rstl::rc_ptr< CSoundEvaluator > GetSoundEvaluator(CInputStream& in);
};


#endif // _CSOUNDEVALUATOR
