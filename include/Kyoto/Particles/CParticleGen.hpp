#ifndef _CPARTICLEGEN
#define _CPARTICLEGEN
#include "rstl/RstlVersions.h"

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#include "Kyoto/SObjectTag.hpp"
#else
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Graphics/CLight.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Particles/CWarp.hpp"
#endif

#include "rstl/list.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/pair.hpp"

#if RSTL_PRIME1
class CAABox;
class CColor;
class CLight;
class CTransform4f;
class CVector3f;
#endif

class CWarp;

class CParticleGen {
public:
#if !RSTL_PRIME1
  CParticleGen() : mDrawFlags(0) {}
#endif

  virtual ~CParticleGen() = 0;
  virtual const bool Update(double) = 0;
  virtual void Render() = 0;
  virtual void SetOrientation(const CTransform4f& orientation) = 0;
  virtual void SetTranslation(const CVector3f& translation) = 0;
  virtual void SetGlobalOrientation(const CTransform4f& orientation) = 0;
  virtual void SetGlobalTranslation(const CVector3f& translation) = 0;
  virtual void SetGlobalScale(const CVector3f& scale) = 0;
  virtual void SetLocalScale(const CVector3f& scale) = 0;
  virtual void SetParticleEmission(bool emission) = 0;
  virtual void SetModulationColor(const CColor& col) = 0;
  virtual void SetGeneratorRate(float rate) {}
#if !RSTL_PRIME1
  // Names of the draw-flag methods are hypotheses based on the sDrawFlags/sDrawMask Wii exports.
  virtual void SetDrawFlags(uint flags) { mDrawFlags = flags; }
#endif

  virtual const CTransform4f& GetOrientation() const = 0;
  virtual const CVector3f& GetTranslation() const = 0;
  virtual const CTransform4f& GetGlobalOrientation() const = 0;
  virtual const CVector3f& GetGlobalTranslation() const = 0;
  virtual const CVector3f& GetGlobalScale() const = 0;
  virtual bool GetParticleEmission() const = 0;
  virtual const CColor& GetModulationColor() const = 0;
  virtual float GetGeneratorRate() const { return 1.f; }
#if !RSTL_PRIME1
  virtual int GetEmitterTime() const = 0;
  virtual uint GetDrawFlags() const { return mDrawFlags; }
  virtual bool ShouldDraw() const;
  virtual int GetSystemCount() const = 0;
#endif

  virtual bool IsSystemDeletable() const = 0;
  virtual rstl::optional_object< CAABox > GetBounds() const = 0;
  virtual int GetParticleCount() const = 0;
  virtual bool SystemHasLight() const = 0;
  virtual CLight GetLight() const = 0;
  virtual void DestroyParticles() = 0;
  virtual void AddModifier(CWarp*);
  virtual uint Get4CharId() const = 0;

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static FourCC ResType() { return 'PART'; }
#else
  static uint sDrawFlags;
  static uint sDrawMask;
#endif

protected:
  rstl::list< CWarp* > mModifiersList;
#if !RSTL_PRIME1
private:
  uint mDrawFlags;
#endif

};

inline CParticleGen::~CParticleGen() {}

#if !RSTL_PRIME1
inline bool CParticleGen::ShouldDraw() const {
  const uint mask = sDrawMask;
  const uint flags = sDrawFlags;
  const uint drawFlags = GetDrawFlags();
  return (drawFlags & mask) == flags;
}
#endif

#endif // _CPARTICLEGEN
