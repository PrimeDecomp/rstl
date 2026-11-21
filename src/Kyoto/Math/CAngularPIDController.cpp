#include "Kyoto/Math/CAngularPIDController.hpp"

#include "Kyoto/Math/CMath.hpp"

#include "math.h"



CPIDController::CPIDController(float initialValue, EMode mode, float proportionalGain,
                               float integralGain, float derivativeGain)
: mMode(mode)
, mProportionalGain(proportionalGain)
, mIntegralGain(integralGain)
, mDerivativeGain(derivativeGain)
, mOutput(0.f)
, mPreviousOutput(0.f)
, mValue(initialValue)
, mPreviousValue(initialValue)
, mOlderValue(initialValue)
, mTarget(0.f)
, mError(0.f)
, mPreviousError(0.f)
, mOlderError(0.f) {}

void CPIDController::Update(float target, float value, float dt) {
  mPreviousOutput = mOutput;
  mOlderValue = mPreviousValue;
  mPreviousValue = mValue;
  mOlderError = mPreviousError;
  mPreviousError = mError;
  mTarget = target;
  mValue = value;
  mError = mTarget - mValue;

  switch (mMode) {
  case kM_Error:
    mProportionalTerm = mProportionalGain * (mError - mPreviousError);
    mIntegralTerm = mError * (mIntegralGain * dt);
    mDerivativeTerm = (mDerivativeGain / dt) * (mOlderError + (mError - 2.f * mPreviousError));
    mOutput = mDerivativeTerm + (mIntegralTerm + (mPreviousOutput + mProportionalTerm));
    break;
  case kM_Measurement:
    mProportionalTerm = -mProportionalGain * (mValue - mPreviousValue);
    mIntegralTerm = mError * (mIntegralGain * dt);
    mDerivativeTerm = -(mDerivativeGain / dt) * (mOlderValue + (mValue - 2.f * mPreviousValue));
    mOutput = mDerivativeTerm + (mIntegralTerm + (mPreviousOutput + mProportionalTerm));
    break;
  }
}

void CPIDController::OffsetHistory(float offset) {
  mTarget += offset;
  mValue += offset;
  mPreviousValue += offset;
  mOlderValue += offset;
}

CVectorPIDController::CVectorPIDController(const CVector3f& initialValue,
                                           CPIDController::EMode mode, float proportionalGain,
                                           float integralGain, float derivativeGain)
: mX(initialValue.GetX(), mode, proportionalGain, integralGain, derivativeGain)
, mY(initialValue.GetY(), mode, proportionalGain, integralGain, derivativeGain)
, mZ(initialValue.GetZ(), mode, proportionalGain, integralGain, derivativeGain) {}

void CVectorPIDController::Update(const CVector3f& target, const CVector3f& value, float dt) {
  mX.Update(target.GetX(), value.GetX(), dt);
  mY.Update(target.GetY(), value.GetY(), dt);
  mZ.Update(target.GetZ(), value.GetZ(), dt);
}

void CVectorPIDController::OffsetHistory(const CVector3f& offset) {
  mX.OffsetHistory(offset.GetX());
  mY.OffsetHistory(offset.GetY());
  mZ.OffsetHistory(offset.GetZ());
}

CAngularPIDController::CAngularPIDController(const CTransform4f& initialRotation,
                                             CPIDController::EMode mode, float proportionalGain,
                                             float integralGain, float derivativeGain)
: mAngles(ExtractAnglesFromRotation(initialRotation))
, mAngularVelocity(CVector3f::Zero())
, mController(mAngles, mode, proportionalGain, integralGain, derivativeGain) {}

CVector3f CAngularPIDController::ExtractAnglesFromRotation(const CTransform4f& rotation) const {
  CVector3f angles;
  angles.SetY(asin(rotation.GetColumn(kDZ)[kDY]));
  angles.SetX(atan2f(-rotation.GetColumn(kDX)[kDY], rotation.GetColumn(kDY)[kDY]));
  angles.SetZ(atan2f(rotation.GetColumn(kDZ)[kDX], rotation.GetColumn(kDZ)[kDZ]));
  return angles;
}

CTransform4f
CAngularPIDController::BuildRotationTransformFromAngles(const CVector3f& angles) const {
  const float sx = CMath::FastSinR(angles.GetX());
  const float cx = CMath::FastCosR(angles.GetX());
  const float sy = CMath::FastSinR(angles.GetY());
  const float cy = CMath::FastCosR(angles.GetY());
  const float sz = CMath::FastSinR(angles.GetZ());
  const float cz = CMath::FastCosR(angles.GetZ());
  return CTransform4f::FromColumns(
      CVector3f(cx * cz + sz * (sx * sy), -sx * cy, -cx * sz + cz * (sx * sy)),
      CVector3f(sx * cz - sz * (cx * sy), cx * cy, -sx * sz - cz * (cx * sy)),
      CVector3f(cy * sz, sy, cy * cz), CVector3f::Zero());
}

float CAngularPIDController::NearestEquivalentAngle(float angle, float reference) const {
  const float distance = CMath::AbsF(angle - reference);
  const float positiveDistance = CMath::AbsF((2.f * M_PIF + angle) - reference);
  const float negativeDistance = CMath::AbsF((angle - 2.f * M_PIF) - reference);
  if (positiveDistance < distance) {
    angle += 2.f * M_PIF;
  } else if (negativeDistance < distance) {
    angle -= 2.f * M_PIF;
  }
  return angle;
}

float CAngularPIDController::GetAngleWrapCorrection(float angle) const {
  float correction = 0.f;
  if (angle > M_PIF) {
    correction = -2.f * M_PIF;
  } else if (angle < -M_PIF) {
    correction = 2.f * M_PIF;
  }
  return correction;
}

void CAngularPIDController::UpdateRotation(const CTransform4f& rotation,
                                           const CTransform4f& targetRotation, float dt) {
  const CVector3f previousAngles = mAngles;
  CVector3f targetAngles = ExtractAnglesFromRotation(targetRotation);
  targetAngles.SetX(NearestEquivalentAngle(targetAngles.GetX(), previousAngles.GetX()));
  targetAngles.SetZ(NearestEquivalentAngle(targetAngles.GetZ(), previousAngles.GetZ()));
  mController.Update(targetAngles, previousAngles, dt);
  mAngularVelocity += dt * mController.GetOutput();
  mAngles = previousAngles + dt * mAngularVelocity;

  CVector3f correction = CVector3f::Zero();
  const float xCorrection = GetAngleWrapCorrection(mAngles.GetX());
  mAngles.SetX(xCorrection + mAngles.GetX());
  correction.SetX(xCorrection);
  const float zCorrection = GetAngleWrapCorrection(mAngles.GetZ());
  mAngles.SetZ(zCorrection + mAngles.GetZ());
  correction.SetZ(zCorrection);
  mController.OffsetHistory(correction);
}
