#include "rstl/RstlVersions.h"
#include "Kyoto/Particles/CManagedParticleGen.hpp"

#if RSTL_CORRUPTION
#include "Kyoto/Alloc/Assert.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#endif
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CParticleElectric.hpp"
#include "Kyoto/Particles/CParticleSpawnSystem.hpp"
#include "Kyoto/Particles/CParticleSwoosh.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/Particles/CParticleSortedSystem.hpp"


#else
#include "Kyoto/Particles/CSortedParticleSystem.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#endif

CManagedParticleGen::CManagedParticleGen(const CToken& effect, const CDependencyGroupToken& group)
: mEffect(effect)
#if RSTL_CORRUPTION
, mAssetTag(mEffect.GetTag())
#else
, mTag(mEffect.GetTag())
#endif
, mDependencies(group)
, mGenerator(nullptr)
, mMaxIdleTime(0.2f)
, mIdleTime(0.f)
, mKeepAlive(false)
, mCreatePending(false)
, mUseSystemDeletable(false) {}

CManagedParticleGen::~CManagedParticleGen() {}

bool CManagedParticleGen::Update(double dt) {
  if (mGenerator.get() != nullptr) {
    mGenerator->Update(dt);
    bool done = false;
    if (!mKeepAlive) {
      if (mUseSystemDeletable) {
        done = mGenerator->IsSystemDeletable();
      } else {
        bool hasParticles = mGenerator->GetParticleCount() != 0;
        if (!hasParticles) {
          mIdleTime += static_cast< float >(dt);
        } else {
          mIdleTime = 0.f;
        }
        if (mIdleTime > mMaxIdleTime) {
          done = true;
        }
      }
      if (done) {
        mGenerator = nullptr;
        Unload();
      }
    }
  } else if (mCreatePending && mDependencies && mDependencies->IsLocked() &&
             mDependencies->IsLoaded()) {
    mCreatePending = false;
    CreateGenerator();
    mIdleTime = 0.f;
  }
  return false;
}

void CManagedParticleGen::Render() const {
  if (mGenerator.get() != nullptr) {
    mGenerator->Render();
  }
}

void CManagedParticleGen::SetOrientation(const CTransform4f& orientation) {
  if (mGenerator.get() != nullptr) {
    mGenerator->SetOrientation(orientation);
  }
}

void CManagedParticleGen::SetTranslation(const CVector3f& translation) {
  if (mGenerator.get() != nullptr) {
    mGenerator->SetTranslation(translation);
  }
}

void CManagedParticleGen::SetParticleEmission(bool emitting) {
  if (mGenerator.get() != nullptr) {
    mGenerator->SetParticleEmission(emitting);
  }
}

void CManagedParticleGen::SetGeneratorRate(float rate) {
  if (mGenerator.get() != nullptr) {
    mGenerator->SetGeneratorRate(rate);
  }
}

void CManagedParticleGen::Load(bool createGenerator) {
  if (mDependencies && !mDependencies->IsLocked()) {
    mDependencies->Lock();
  }
  if (mGenerator.get() == nullptr) {
    mCreatePending = createGenerator;
  } else {
    mCreatePending = false;
  }
}

void CManagedParticleGen::Unload() {
  if (mDependencies && mDependencies->IsLocked()) {
    mDependencies->Unlock();
  }
  mCreatePending = false;
}

#if RSTL_CORRUPTION
const CElementGen* CManagedParticleGen::GetElementGen() const {
  RS_VERIFY_THROW(309, mAssetTag.GetType() == CElementGen::ResType(), false,
                  "Token types don't match in CManagedParticleGen::GetElementGen().");
  return static_cast< const CElementGen* >(mGenerator.get());
}

CElementGen* CManagedParticleGen::ElementGen() {
  RS_VERIFY_THROW(317, mAssetTag.GetType() == CElementGen::ResType(), false,
                  "Token types don't match in CManagedParticleGen::ElementGen().");
  return static_cast< CElementGen* >(mGenerator.get());
}

const CParticleSwoosh* CManagedParticleGen::GetSwoosh() const {
  RS_VERIFY_THROW(325, mAssetTag.GetType() == CParticleSwoosh::ResType(), false,
                  "Token types don't match in CManagedParticleGen::GetSwoosh().");
  return static_cast< const CParticleSwoosh* >(mGenerator.get());
}

CParticleSwoosh* CManagedParticleGen::Swoosh() {
  RS_VERIFY_THROW(333, mAssetTag.GetType() == CParticleSwoosh::ResType(), false,
                  "Token types don't match in CManagedParticleGen::Swoosh().");
  return static_cast< CParticleSwoosh* >(mGenerator.get());
}

const CParticleElectric* CManagedParticleGen::GetElectric() const {
  RS_VERIFY_THROW(341, mAssetTag.GetType() == CParticleElectric::ResType(), false,
                  "Token types don't match in CManagedParticleGen::GetElectric().");
  return static_cast< const CParticleElectric* >(mGenerator.get());
}

CParticleElectric* CManagedParticleGen::Electric() {
  RS_VERIFY_THROW(349, mAssetTag.GetType() == CParticleElectric::ResType(), false,
                  "Token types don't match in CManagedParticleGen::Electric().");
  return static_cast< CParticleElectric* >(mGenerator.get());
}

const CParticleSortedSystem* CManagedParticleGen::GetSortedSystem() const {
  RS_VERIFY_THROW(357, mAssetTag.GetType() == CParticleSortedSystem::ResType(), false,
                  "Token types don't match in CManagedParticleGen::GetSortedSystem().");
  return static_cast< const CParticleSortedSystem* >(mGenerator.get());
}

CParticleSortedSystem* CManagedParticleGen::SortedSystem() {
  RS_VERIFY_THROW(365, mAssetTag.GetType() == CParticleSortedSystem::ResType(), false,
                  "Token types don't match in CManagedParticleGen::SortedSystem().");
  return static_cast< CParticleSortedSystem* >(mGenerator.get());
}

const CParticleSpawnSystem* CManagedParticleGen::GetSpawnSystem() const {
  RS_VERIFY_THROW(373, mAssetTag.GetType() == CParticleSpawnSystem::ResType(), false,
                  "Token types don't match in CManagedParticleGen::GetSpawnSystem().");
  return static_cast< const CParticleSpawnSystem* >(mGenerator.get());
}

CParticleSpawnSystem* CManagedParticleGen::SpawnSystem() {
  RS_VERIFY_THROW(381, mAssetTag.GetType() == CParticleSpawnSystem::ResType(), false,
                  "Token types don't match in CManagedParticleGen::SpawnSystem().");
  return static_cast< CParticleSpawnSystem* >(mGenerator.get());
}

#endif
void CManagedParticleGen::CreateGenerator() {
  CParticleGen* gen;
#if RSTL_CORRUPTION
  switch (mAssetTag.type) {
#else
  switch (mTag.type) {
#endif
  case 'PART': {
    TLockedToken< CGenDescription > desc(mEffect);
#if RSTL_CORRUPTION
    gen = rs_new_line(431) CElementGen(desc, CElementGen::kMOT_Normal, CElementGen::kOSF_One);
#else
    gen = rs_new CElementGen(desc, CElementGen::kMOT_Normal, CElementGen::kOSF_One);
#endif
    break;
  }
  case 'SWHC': {
    TToken< CSwooshDescription > desc(mEffect);
#if RSTL_CORRUPTION
    gen = rs_new_line(438) CParticleSwoosh(desc, 0);
#else
    gen = rs_new CParticleSwoosh(desc, 0);
#endif
    break;
  }
  case 'ELSC': {
    TToken< CElectricDescription > desc(mEffect);
#if RSTL_CORRUPTION
    gen = rs_new_line(444) CParticleElectric(desc);
#else
    gen = rs_new CParticleElectric(desc);
#endif
    break;
  }
  case 'SPSC': {
    TToken< CSpawnSystemDescription > desc(mEffect);
#if RSTL_CORRUPTION
    gen = rs_new_line(450) CParticleSpawnSystem(desc, CElementGen::kOSF_One, false);
#else
    gen = rs_new CParticleSpawnSystem(desc, CElementGen::kOSF_One, false);
#endif
    break;
  }
  case 'SRSC': {
    TToken< CSortedParticleSystemDescription > desc(mEffect);
#if RSTL_CORRUPTION
    gen = rs_new_line(456) CParticleSortedSystem(desc, CElementGen::kOSF_One, false);
#else
    gen = rs_new CSortedParticleSystem(desc, CElementGen::kOSF_One, false);
#endif
    break;
  }
  default:
#if RSTL_CORRUPTION
    gpfnWarningPrintf("Unsupported spawned particle asset type %d\n", mAssetTag.type);
#endif
    return;
  }
  mGenerator = gen;
}
