#ifndef _CPARTICLESORTEDSYSTEM
#define _CPARTICLESORTEDSYSTEM

#include "rstl/static_assert.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CParticleGen.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/reserved_vector.hpp"



class CSortedParticleSystemDescription;

class CParticleSortedSystem : public CParticleGen {
public:
  static FourCC ResType() { return 'SRSC'; }

  CParticleSortedSystem(TToken< CSortedParticleSystemDescription > desc,
                        CElementGen::EOptionalSystemFlags flags, bool modelsUseLights);
  // CParticleGen
  ~CParticleSortedSystem() override;

  bool Update(double dt) override;
  void Render() override;
  void SetOrientation(const CTransform4f& orientation) override;
  void SetTranslation(const CVector3f& translation) override;
  void SetGlobalOrientation(const CTransform4f& orientation) override;
  void SetGlobalTranslation(const CVector3f& translation) override;
  void SetGlobalScale(const CVector3f& scale) override;
  void SetLocalScale(const CVector3f& scale) override;
  void SetParticleEmission(bool emission) override;
  void SetModulationColor(const CColor& col) override;
  void SetGeneratorRate(float rate) override;
  const CTransform4f& GetOrientation() const override;
  const CVector3f& GetTranslation() const override;
  const CTransform4f& GetGlobalOrientation() const override;
  const CVector3f& GetGlobalTranslation() const override;
  const CVector3f& GetGlobalScale() const override;
  bool GetParticleEmission() const override;
  const CColor& GetModulationColor() const override;
  float GetGeneratorRate() const override;
  int GetEmitterTime() const override;
  int GetSystemCount() const override;
  bool IsSystemDeletable() const override;
  rstl::optional_object< CAABox > GetBounds() const override;
  int GetParticleCount() const override;
  bool SystemHasLight() const override;
  CLight GetLight() const override;
  void DestroyParticles() override;
  uint Get4CharId() const override;

  static ushort GetGlobalSeed() { return sSeed; }
  static void SetGlobalSeed(ushort seed) { sSeed = seed; }

private:
  // Guessed names
  void UpdateChildParticleSystems(double dt);
  void BuildParticleSystemBounds();

  static ushort sSeed; // Guessed name
  // Guessed names
  TLockedToken< CSortedParticleSystemDescription > mDescription;
  rstl::reserved_vector< CParticleGen*, 8 > mChildren;
  CRandom16 mRandom;
  int mCurFrame;
  double mCurSeconds;
  float mGeneratorRate;
  int mParticleCount;
  int mPrevFrame;
  CElementGen::EOptionalSystemFlags mOptionalFlags;
  bool mModelsUseLights : 1;
  bool mParticleEmission : 1;
  CVector3f mGlobalTranslation;
  CVector3f mGlobalScale;
  CTransform4f mGlobalOrientation;
  CAABox mBounds;
};
CHECK_SIZEOF(CParticleSortedSystem, 0xd8)


#endif // _CPARTICLESORTEDSYSTEM
