#ifndef _CANGULARPIDCONTROLLER
#define _CANGULARPIDCONTROLLER

#include "rstl/static_assert.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVectorPIDController.hpp"

// Guessed name
class CAngularPIDController {
public:
  CAngularPIDController(const CTransform4f& initialRotation, CPIDController::EMode mode,
                        float proportionalGain, float integralGain, float derivativeGain);
  void UpdateRotation(const CTransform4f& rotation, const CTransform4f& targetRotation,
                      float dt);                                                // Guessed name
  CTransform4f BuildRotationTransformFromAngles(const CVector3f& angles) const; // Guessed name
  const CVector3f& GetAngles() const { return mAngles; } // Guessed name
  CVector3f GetControllerOutput() const { return mController.GetOutput(); } // Guessed name

private:
  CVector3f ExtractAnglesFromRotation(const CTransform4f& rotation) const; // Guessed name
  float NearestEquivalentAngle(float angle, float reference) const;        // Guessed name
  float GetAngleWrapCorrection(float angle) const;                         // Guessed name

  CVector3f mAngles;                // Guessed name
  CVector3f mAngularVelocity;       // Guessed name
  CVectorPIDController mController; // Guessed name
};
CHECK_SIZEOF(CAngularPIDController, 0xd8)

#endif // _CANGULARPIDCONTROLLER
