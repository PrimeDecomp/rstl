#include "WorldFormat/CWorldLight.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/math.hpp"

const CVector3f CWorldLight::kDefaultPosition(0.f, 0.f, 0.f);
const CVector3f CWorldLight::kDefaultDirection(0.f, 1.f, 0.f);
#if RSTL_CORRUPTION
const CVector3f CWorldLight::kDefaultUp(0.f, 1.f, 0.f);
#endif

CWorldLight::CWorldLight(CInputStream& in)
: mType(static_cast< EWorldLightType >(in.Get< uint >()))
, mColor(in)
, mPosition(in)
, mDirection(in)
#if RSTL_CORRUPTION
, mUp(in)
#endif
, mQ(in.Get< float >())
, mCutoffAngle(in.Get< float >())
, mUnknownFloat0(in.Get< float >())
, mCastShadows(in.Get< bool >())
, mUnknownFloat1(in.Get< float >())
, mFalloff(static_cast< EFalloffType >(in.Get< uint >()))
, mUnknownFloat2(in.Get< float >())
#if RSTL_CORRUPTION
, mAlphaOffset(in.Get< float >())
, mAlphaScale(in.Get< float >())
, mSpot2ScaleX(in.Get< float >())
, mSpot2ScaleZ(in.Get< float >())
, mUnknownWord1(in.Get< uint >())
#endif
{
}

#if RSTL_CORRUPTION
CLight CWorldLight::GetAsCGraphicsLight() const {
  float q = mQ;
  if (mQ < FLT_EPSILON) {
    q = 10.f * FLT_EPSILON;
  }

  float r = q * mColor.GetRed();
  float g = q * mColor.GetGreen();
  float b = q * mColor.GetBlue();
  float maxColor = rstl::max_val(r, rstl::max_val(g, b));
  float intensity = (maxColor * q - mAlphaOffset) * mAlphaScale;
  float alpha = CMath::Clamp(0.f, intensity, 1.f);
  CColor color = mColor.WithAlphaOf(alpha);

  if (mType == kWLT_LocalAmbient || mType == kWLT_LocalAmbient2) {
    CColor ambientColor(rstl::min_val(1.f, r), rstl::min_val(1.f, g), rstl::min_val(1.f, b),
                        color.GetAlpha());
    CLight light = CLight::BuildLocalAmbient(mPosition, ambientColor);
    light.SetColorProcessingMode(true);
    return light;
  }

  if (mType == kWLT_Directional) {
    CLight light = CLight::BuildDirectional(mDirection, color);
    light.SetColorProcessingMode(true);
    return light;
  }

  if (mType == kWLT_Spot) {
    CLight light =
        CLight::BuildSpot(mPosition, mDirection.AsNormalized(), color, mCutoffAngle / 2.f);
    float quadratic = mFalloff == kFT_Quadratic ? 25000.f / q : 0.f;
    float linear = mFalloff == kFT_Linear ? (1.f / 0.004f) / q : 0.f;
    float constant = mFalloff == kFT_Constant ? 2.f / q : 0.f;
    light.SetAttenuation(constant, linear, quadratic);
    light.SetColorProcessingMode(true);
    return light;
  }

  if (mType == kWLT_Spot2) {
    CLight light = CLight::BuildSpot(mPosition, mDirection.AsNormalized(), color, 90.f);
    float quadratic = mFalloff == kFT_Quadratic ? 25000.f / q : 0.f;
    float linear = mFalloff == kFT_Linear ? (1.f / 0.004f) / q : 0.f;
    float constant = mFalloff == kFT_Constant ? 2.f / q : 0.f;
    light.SetAttenuation(constant, linear, quadratic);
    light.SetColorProcessingMode(true);
    return light;
  }

  float quadratic = mFalloff == kFT_Quadratic ? 25000.f / q : 0.f;
  float linear = mFalloff == kFT_Linear ? (1.f / 0.004f) / q : 0.f;
  float constant = mFalloff == kFT_Constant ? 2.f / q : 0.f;
  CLight light = CLight::BuildCustom(mPosition, CVector3f(1.f, 0.f, 0.f), color, constant, linear,
                                     quadratic, 1.f, 0.f, 0.f);
  light.SetColorProcessingMode(true);
  return light;
}
#else
#if RSTL_PRIME1
#define RSTL_WL_X 0
#define RSTL_WL_Y 1
#define RSTL_WL_Z 2
#else
#define RSTL_WL_X kDX
#define RSTL_WL_Y kDY
#define RSTL_WL_Z kDZ
#endif
CLight CWorldLight::GetAsCGraphicsLight() const {
  CVector3f floatColor = mColor;
  CColor color(floatColor[RSTL_WL_X], floatColor[RSTL_WL_Y], floatColor[RSTL_WL_Z]);

  float q = mQ;
  if (mQ < FLT_EPSILON) {
    q = 10.f * FLT_EPSILON;
  }

  if (mType == kWLT_LocalAmbient) {
#if RSTL_PRIME1
    floatColor *= q;
#else
    for (int i = 0; i < 3; ++i) {
      floatColor[i] *= q;
    }
#endif
    floatColor[RSTL_WL_X] = rstl::min_val(1.f, floatColor[RSTL_WL_X]);
    floatColor[RSTL_WL_Y] = rstl::min_val(1.f, floatColor[RSTL_WL_Y]);
    floatColor[RSTL_WL_Z] = rstl::min_val(1.f, floatColor[RSTL_WL_Z]);
    CColor ambientColor(floatColor[RSTL_WL_X], floatColor[RSTL_WL_Y], floatColor[RSTL_WL_Z]);
    return CLight::BuildLocalAmbient(mPosition, ambientColor);
  }

  if (mType == kWLT_Directional) {
    return CLight::BuildDirectional(mDirection, color);
  }

  if (mType == kWLT_Spot) {
    CLight light =
        CLight::BuildSpot(mPosition, mDirection.AsNormalized(), color, mCutoffAngle / 2.f);
    float quadratic = mFalloff == kFT_Quadratic ? 25000.f / q : 0.f;
    float linear = mFalloff == kFT_Linear ? (1.f / 0.004f) / q : 0.f;
    float constant = mFalloff == kFT_Constant ? 2.f / q : 0.f;
    light.SetAttenuation(constant, linear, quadratic);
    return light;
  }

  float quadratic = mFalloff == kFT_Quadratic ? 25000.f / q : 0.f;
  float linear = mFalloff == kFT_Linear ? (1.f / 0.004f) / q : 0.f;
  float constant = mFalloff == kFT_Constant ? 2.f / q : 0.f;
  return CLight::BuildCustom(mPosition, CVector3f(1.f, 0.f, 0.f), color, constant, linear,
                             quadratic, 1.f, 0.f, 0.f);
}
#undef RSTL_WL_X
#undef RSTL_WL_Y
#undef RSTL_WL_Z
#endif
