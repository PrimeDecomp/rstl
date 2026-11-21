#ifndef _CVECTORPIDCONTROLLER
#define _CVECTORPIDCONTROLLER

#include "rstl/static_assert.hpp"
#include "Kyoto/Math/CPIDController.hpp"
#include "Kyoto/Math/CVector3f.hpp"



// Guessed name
class CVectorPIDController {
public:
  CVectorPIDController(const CVector3f& initialValue, CPIDController::EMode mode,
                       float proportionalGain, float integralGain, float derivativeGain);
  void Update(const CVector3f& target, const CVector3f& value, float dt); // Guessed name
  void OffsetHistory(const CVector3f& offset);                            // Guessed name
  CVector3f GetOutput() const {                                           // Guessed name
    return CVector3f(mX.GetOutput(), mY.GetOutput(), mZ.GetOutput());
  }

private:
  CPIDController mX; // Guessed name
  CPIDController mY; // Guessed name
  CPIDController mZ; // Guessed name
};
CHECK_SIZEOF(CVectorPIDController, 0xc0)


#endif // _CVECTORPIDCONTROLLER
