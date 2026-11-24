#ifndef _CWORLDLIGHT
#define _CWORLDLIGHT

#include "rstl/RstlVersions.h"
#include "rstl/construct.hpp"
#include "rstl/static_assert.hpp"

#include "Kyoto/Graphics/CLight.hpp"
#include "Kyoto/Math/CVector3f.hpp"

class CInputStream;

class CWorldLight {
  static const CVector3f kDefaultPosition;
  static const CVector3f kDefaultDirection;
#if RSTL_CORRUPTION_STANDALONE
  static const CVector3f kDefaultUp; // Guessed name
#endif

public:
  enum EWorldLightType {
    kWLT_LocalAmbient,
    kWLT_Directional,
    kWLT_Custom,
    kWLT_Spot,
    kWLT_Spot2,
    kWLT_LocalAmbient2
  };

  explicit CWorldLight(CInputStream& in);
  CLight GetAsCGraphicsLight() const;
  EWorldLightType GetType() const { return mType; }
  const CVector3f& GetPosition() const { return mPosition; }
  const CVector3f& GetDirection() const { return mDirection; }
  float GetCutoffAngle() const { return mCutoffAngle; }
  float GetCutOffAngle() const { return mCutoffAngle; }
  bool DoesCastShadows() const { return mCastShadows; }
#if RSTL_CORRUPTION_STANDALONE
  const CVector3f& GetUp() const { return mUp; }
  float GetSpot2ScaleX() const { return mSpot2ScaleX; }
  float GetSpot2ScaleZ() const { return mSpot2ScaleZ; }
#endif

#if RSTL_PRIME1
public:
#else
private:
#endif
  EWorldLightType mType;
#if RSTL_CORRUPTION_STANDALONE
  CColor mColor;
#else
  CVector3f mColor;
#endif
  CVector3f mPosition;
  CVector3f mDirection;
#if RSTL_CORRUPTION_STANDALONE
  CVector3f mUp; // Guessed name
#endif
  float mQ;
  float mCutoffAngle;
  float mUnknownFloat0;
  bool mCastShadows;
  float mUnknownFloat1;
  EFalloffType mFalloff;
  float mUnknownFloat2;
#if RSTL_CORRUPTION_STANDALONE
  float mAlphaOffset;
  float mAlphaScale;
  uint mUnknownWord0;
  float mSpot2ScaleX; // Guessed name: scales the area marker drawn for Spot2 lights
  float mSpot2ScaleZ; // Guessed name
  uint mUnknownWord1;
#elif !RSTL_PRIME1
  uint mUnknownWord;
#endif
};
CHECK_SIZEOF(CWorldLight, RSTL_PRIME1 ? 0x44 : !RSTL_CORRUPTION_STANDALONE ? 0x48 : 0x60)

#if RSTL_KYOTO_CONSTRUCTION_POLICIES
namespace rstl {
RSTL_DECLARE_TRIVIALLY_DESTRUCTIBLE(CWorldLight)
RSTL_DECLARE_BITWISE_CONSTRUCTION(CWorldLight)
#if !RSTL_CORRUPTION_STANDALONE
RSTL_DECLARE_VECTOR_CONSTRUCTION(CWorldLight)
RSTL_DECLARE_VECTOR_RESERVE_CONSTRUCTION(CWorldLight)
RSTL_DECLARE_VECTOR_ASSIGNMENT_CONSTRUCTION(CWorldLight)
#endif
} // namespace rstl
#endif

#endif // _CWORLDLIGHT
