#include "Kyoto/Particles/CParticleSortedSystem.hpp"

#include "Kyoto/Alloc/Assert.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Particles/CParticleGlobals.hpp"
#include "Kyoto/Particles/CSortedParticleSystemDescription.hpp"
#include "Kyoto/Particles/CSpawnSystemKeyframeData.hpp"



ushort CParticleSortedSystem::sSeed = 99;

static const CVector3f skOne(1.f, 1.f, 1.f);

CParticleSortedSystem::CParticleSortedSystem(TToken< CSortedParticleSystemDescription > description,
                                             CElementGen::EOptionalSystemFlags flags,
                                             bool modelsUseLights)
: mDescription(description)
, mRandom(sSeed)
, mCurFrame(0)
, mCurSeconds(0.0)
, mGeneratorRate(1.f)
, mParticleCount(0)
, mPrevFrame(-1)
, mOptionalFlags(flags)
, mModelsUseLights(modelsUseLights)
, mParticleEmission(true)
, mGlobalTranslation(CVector3f::Zero())
, mGlobalScale(CVector3f::Zero())
, mGlobalOrientation(CTransform4f::Identity())
, mBounds(CAABox::MakeMaxInvertedBox()) {}

CParticleSortedSystem::~CParticleSortedSystem() {
  for (rstl::reserved_vector< CParticleGen*, 8 >::iterator it = mChildren.begin();
       it != mChildren.end(); ++it) {
    delete *it;
  }
}

bool CParticleSortedSystem::Update(double dt) {
  CGlobalRandom random(mRandom);
  bool updated = false;
  double frameTime = mCurFrame * (1.0 / 60.0);
  mCurSeconds += dt;
  while (frameTime < mCurSeconds) {
    CParticleGlobals::SetEmitterTime(mCurFrame);
    UpdateChildParticleSystems(1.0 / 60.0);
    frameTime += 1.0 / 60.0;
    updated = true;
    ++mCurFrame;
  }
  if (updated) {
    BuildParticleSystemBounds();
  }
  return updated;
}

void CParticleSortedSystem::Render() {
  if (!mChildren.empty()) {
    CElementGen* visible[8];
    int count = 0;
    for (rstl::reserved_vector< CParticleGen*, 8 >::iterator it = mChildren.begin();
         it != mChildren.end(); ++it) {
      if ((*it)->ShouldDraw()) {
        visible[count++] = static_cast< CElementGen* >(*it);
      }
    }
    CElementGen::RenderParticlesFlameThrower(visible, count, &mGlobalTranslation,
                                             &mGlobalOrientation, &mGlobalScale, nullptr);
  }
}

void CParticleSortedSystem::SetOrientation(const CTransform4f& orientation) {
  for (rstl::reserved_vector< CParticleGen*, 8 >::iterator it = mChildren.begin();
       it != mChildren.end(); ++it) {
    (*it)->SetOrientation(orientation);
  }
}

void CParticleSortedSystem::SetTranslation(const CVector3f& translation) {
  for (rstl::reserved_vector< CParticleGen*, 8 >::iterator it = mChildren.begin();
       it != mChildren.end(); ++it) {
    (*it)->SetTranslation(translation);
  }
}

void CParticleSortedSystem::SetGlobalOrientation(const CTransform4f& orientation) {
  mGlobalOrientation = orientation;
}

void CParticleSortedSystem::SetGlobalTranslation(const CVector3f& translation) {
  mGlobalTranslation = translation;
}

void CParticleSortedSystem::SetGlobalScale(const CVector3f& scale) { mGlobalScale = scale; }

void CParticleSortedSystem::SetLocalScale(const CVector3f& scale) {
  for (rstl::reserved_vector< CParticleGen*, 8 >::iterator it = mChildren.begin();
       it != mChildren.end(); ++it) {
    (*it)->SetLocalScale(scale);
  }
}

void CParticleSortedSystem::SetParticleEmission(bool emission) {
  mParticleEmission = emission;
  for (rstl::reserved_vector< CParticleGen*, 8 >::iterator it = mChildren.begin();
       it != mChildren.end(); ++it) {
    (*it)->SetParticleEmission(emission);
  }
}

void CParticleSortedSystem::SetModulationColor(const CColor& color) {
  for (rstl::reserved_vector< CParticleGen*, 8 >::iterator it = mChildren.begin();
       it != mChildren.end(); ++it) {
    (*it)->SetModulationColor(color);
  }
}

void CParticleSortedSystem::SetGeneratorRate(float rate) {
  mGeneratorRate = rate;
  for (rstl::reserved_vector< CParticleGen*, 8 >::iterator it = mChildren.begin();
       it != mChildren.end(); ++it) {
    (*it)->SetGeneratorRate(rate);
  }
}

const CTransform4f& CParticleSortedSystem::GetOrientation() const {
  if (mChildren.empty()) {
    return CTransform4f::Identity();
  }
  return mChildren.front()->GetOrientation();
}

const CVector3f& CParticleSortedSystem::GetTranslation() const {
  if (mChildren.empty()) {
    return CVector3f::Zero();
  }
  return mChildren.front()->GetTranslation();
}

const CTransform4f& CParticleSortedSystem::GetGlobalOrientation() const {
  return mGlobalOrientation;
}

const CVector3f& CParticleSortedSystem::GetGlobalTranslation() const { return mGlobalTranslation; }

const CVector3f& CParticleSortedSystem::GetGlobalScale() const { return mGlobalScale; }

const CColor& CParticleSortedSystem::GetModulationColor() const {
  if (mChildren.empty()) {
    return CColor::White();
  }
  return mChildren.front()->GetModulationColor();
}

float CParticleSortedSystem::GetGeneratorRate() const { return mGeneratorRate; }

bool CParticleSortedSystem::IsSystemDeletable() const {
  for (rstl::reserved_vector< CParticleGen*, 8 >::const_iterator it = mChildren.begin();
       it != mChildren.end(); ++it) {
    if (!(*it)->IsSystemDeletable()) {
      return false;
    }
  }
  return true;
}

rstl::optional_object< CAABox > CParticleSortedSystem::GetBounds() const {
  if (GetParticleCount() <= 0) {
    return rstl::optional_object< CAABox >();
  }
  return mBounds;
}

int CParticleSortedSystem::GetSystemCount() const {
  int count = GetParticleCount() > 0 ? 1 : 0;
  for (rstl::reserved_vector< CParticleGen*, 8 >::const_iterator it = mChildren.begin();
       it != mChildren.end(); ++it) {
    count += (*it)->GetSystemCount();
  }
  return count;
}

bool CParticleSortedSystem::SystemHasLight() const { return false; }

CLight CParticleSortedSystem::GetLight() const {
  return CLight::BuildLocalAmbient(CVector3f::Zero(), CColor::White());
}

void CParticleSortedSystem::DestroyParticles() {
  for (rstl::reserved_vector< CParticleGen*, 8 >::iterator it = mChildren.begin();
       it != mChildren.end(); ++it) {
    (*it)->DestroyParticles();
  }
}

void CParticleSortedSystem::BuildParticleSystemBounds() {
  mParticleCount = 0;
  for (rstl::reserved_vector< CParticleGen*, 8 >::iterator it = mChildren.begin();
       it != mChildren.end(); ++it) {
    CParticleGen* child = *it;
    mParticleCount += child->Get4CharId() == 'PART'
                          ? static_cast< CElementGen* >(child)->GetParticleCountAll()
                          : child->GetParticleCount();
  }
  mBounds = CAABox::MakeMaxInvertedBox();
  for (rstl::reserved_vector< CParticleGen*, 8 >::iterator it = mChildren.begin();
       it != mChildren.end(); ++it) {
    rstl::optional_object< CAABox > bounds = (*it)->GetBounds();
    if (bounds) {
      const CAABox& box = *bounds;
      mBounds.AccumulateBounds(box.GetMinPoint());
      mBounds.AccumulateBounds(box.GetMaxPoint());
    }
  }
}

void CParticleSortedSystem::UpdateChildParticleSystems(double dt) {
  if (close_enough(dt, 0.0, 1e-7)) {
    return;
  }
  CGlobalRandom random(mRandom);
  if (mDescription->mSPWN && mPrevFrame != mCurFrame &&
      uint(mCurFrame) <= mDescription->mSPWN->GetEndFrame()) {
    rstl::vector< CSpawnSystemKeyframeData::CSpawnSystemKeyframeInfo >& spawns =
        mDescription->mSPWN->GetSpawnedSystemsAtFrame(mCurFrame);
    if (!spawns.empty()) {
      ushort backupSeed = sSeed;
      for (int i = 0; i < spawns.size(); ++i) {
        CSpawnSystemKeyframeData::CSpawnSystemKeyframeInfo& info = spawns[i];
        const ushort seed = (i + mCurFrame + 1) * 100 + mRandom.GetSeed();
        if (info.GetType() == 'PART') {
          if (mChildren.size() < mChildren.capacity()) {
            CParticleGen* child = CElementGen::ConstructChildParticleSystem(
                *info.GetToken(), info.GetType(), seed, mOptionalFlags, mModelsUseLights,
                mParticleEmission, CVector3f::Zero(), CTransform4f::Identity(), mGlobalTranslation,
                mGlobalOrientation, mGlobalScale, CColor::White(), CVector3f::One());
            if (child) {
              mChildren.push_back(child);
            }
          } else {
            const TToken< CSortedParticleSystemDescription >& description = mDescription;
            const char* message = CBasics::Stringize(
                "BUG: Trying to create more than %d systems in Sorted Particle System (%x).  "
                "Ignoring extra systems.\n",
                mChildren.capacity(), description.GetTag().GetId());
            RS_WARNING_DEBUGGER_PRINT(message);
          }
        } else {
          const TToken< CSortedParticleSystemDescription >& description = mDescription;
          const char* message = CBasics::Stringize(
              "BUG: Sorted Particle System (%x) is attempting to create a non-generic particle "
              "system.  Ignoring non-generic system.\n",
              description.GetTag().GetId());
          RS_WARNING_DEBUGGER_PRINT(message);
        }
      }
      sSeed = backupSeed;
    }
  }

  rstl::reserved_vector< CParticleGen*, 8 >::iterator it = mChildren.begin();
  while (it != mChildren.end()) {
    CParticleGen* child = *it;
    child->Update(dt);
    if (child->IsSystemDeletable()) {
      delete child;
      it = mChildren.erase(it);
    } else {
      ++it;
    }
  }
  mPrevFrame = mCurFrame;
}

uint CParticleSortedSystem::Get4CharId() const { return 'SRSC'; }

int CParticleSortedSystem::GetEmitterTime() const { return mCurFrame; }

int CParticleSortedSystem::GetParticleCount() const { return mParticleCount; }

bool CParticleSortedSystem::GetParticleEmission() const { return mParticleEmission; }
