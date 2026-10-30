#ifndef _CMANAGEDPARTICLEGEN
#define _CMANAGEDPARTICLEGEN
#include "Kyoto/CDependencyGroupToken.hpp"
#include "Kyoto/Particles/CParticleGen.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/single_ptr.hpp"

// Class name from Corruption diagnostics; its Echoes correspondence is reconstructed.
// Owns an effect token, optional dependencies and a lazily created generator.
// Method and member names are guessed.
class CManagedParticleGen {
public:
  CManagedParticleGen(const CToken& effect, const CDependencyGroupToken& group);
  ~CManagedParticleGen();

  bool Update(double dt);
  void Render() const;
  void SetOrientation(const CTransform4f& orientation);
  void SetTranslation(const CVector3f& translation);
  void SetParticleEmission(bool emitting);
  void SetGeneratorRate(float rate);
  void Load(bool createGenerator);
  void Unload();

private:
  void CreateGenerator();

  CToken mEffect;
  SObjectTag mTag;
  rstl::optional_object< CDependencyGroupToken > mDependencies;
  rstl::single_ptr< CParticleGen > mGenerator;
  float mMaxIdleTime;
  float mIdleTime;
  bool mKeepAlive : 1;
  bool mCreatePending : 1;
  bool mUseSystemDeletable : 1;
};
CHECK_SIZEOF(CManagedParticleGen, 0x44)

#endif // _CMANAGEDPARTICLEGEN
