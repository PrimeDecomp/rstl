#ifndef _CMANAGEDPARTICLEGEN
#define _CMANAGEDPARTICLEGEN
#include "rstl/RstlVersions.h"
#include "rstl/static_assert.hpp"
#include "Kyoto/CDependencyGroupToken.hpp"
#include "Kyoto/Particles/CParticleGen.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/single_ptr.hpp"
#if RSTL_CORRUPTION



class CElementGen;
class CParticleSwoosh;
class CParticleElectric;
class CParticleSortedSystem;
class CParticleSpawnSystem;
#endif

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

#if RSTL_CORRUPTION
  const CElementGen* GetElementGen() const;
  CElementGen* ElementGen();
  const CParticleSwoosh* GetSwoosh() const;
  CParticleSwoosh* Swoosh();
  const CParticleElectric* GetElectric() const;
  CParticleElectric* Electric();
  const CParticleSortedSystem* GetSortedSystem() const;
  CParticleSortedSystem* SortedSystem();
  const CParticleSpawnSystem* GetSpawnSystem() const;
  CParticleSpawnSystem* SpawnSystem();

#endif
  CToken mEffect;
#if RSTL_CORRUPTION
  SObjectTag mAssetTag;
#else
  SObjectTag mTag;
#endif
  rstl::optional_object< CDependencyGroupToken > mDependencies;
  rstl::single_ptr< CParticleGen > mGenerator;
  float mMaxIdleTime; // Guessed name
  float mIdleTime;
  bool mKeepAlive : 1; // Guessed name
  bool mCreatePending : 1;
  bool mUseSystemDeletable : 1; // Guessed name
};
#if RSTL_CORRUPTION
CHECK_SIZEOF(CManagedParticleGen, 0x50)
#else
CHECK_SIZEOF(CManagedParticleGen, 0x44)
#endif

#endif // _CMANAGEDPARTICLEGEN
