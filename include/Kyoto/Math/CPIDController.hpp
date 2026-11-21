#ifndef _CPIDCONTROLLER
#define _CPIDCONTROLLER

#include "rstl/static_assert.hpp"

// Guessed name
class CPIDController {
public:
  // Guessed name
  enum EMode {
    kM_Error,       // Guessed name
    kM_Measurement, // Guessed name
  };

  CPIDController(float initialValue, EMode mode, float proportionalGain, float integralGain,
                 float derivativeGain);
  void Update(float target, float value, float dt); // Guessed name
  void OffsetHistory(float offset);                 // Guessed name
  float GetOutput() const { return mOutput; }       // Guessed name

private:
  EMode mMode;             // Guessed name
  float mProportionalGain; // Guessed name
  float mIntegralGain;     // Guessed name
  float mDerivativeGain;   // Guessed name
  float mOutput;           // Guessed name
  float mPreviousOutput;   // Guessed name
  float mValue;            // Guessed name
  float mPreviousValue;    // Guessed name
  float mOlderValue;       // Guessed name
  float mTarget;           // Guessed name
  float mError;            // Guessed name
  float mPreviousError;    // Guessed name
  float mOlderError;       // Guessed name
  float mProportionalTerm; // Guessed name
  float mIntegralTerm;     // Guessed name
  float mDerivativeTerm;   // Guessed name
};
CHECK_SIZEOF(CPIDController, 0x40)

#endif // _CPIDCONTROLLER
