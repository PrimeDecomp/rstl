#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CSkinnedModel.hpp"

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#include "Kyoto/Alloc/CCircularBuffer.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#else
#include "Kyoto/Animation/CCharLayoutInfo.hpp"
#include "Kyoto/Animation/CPoseAsTransforms_Linear.hpp"
#endif

#include "Kyoto/Animation/CSkinRules.hpp"
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#include "Kyoto/Animation/CVertexMorphEffect.hpp"
#else
#include "Kyoto/Graphics/CCubeMaterial.hpp"
#include "Kyoto/Graphics/CCubeModel.hpp"
#include "Kyoto/Graphics/CCubeSurface.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#endif

#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#if !RSTL_PRIME1
#include "Kyoto/Graphics/DolphinGPUMemory.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#endif

#if RSTL_PRIME1
#include "dolphin/base/PPCArch.h"
#endif

#include "dolphin/gx.h"
#include "dolphin/os.h"
#if RSTL_PRIME1
#include "rstl/list.hpp"
#include "rstl/optional_object.hpp"
#endif

CSkinnedModel::TPointGenFunc CSkinnedModel::sPointGen;
void* CSkinnedModel::sPointGenData;

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
struct SSkinnedAllocation {
  SSkinnedAllocation(void* ptr, int w1, ushort w2) : mPtr(ptr), mUnk1(w1), mUnk2(w2) {}
#else
namespace {
// Descriptive name for the scoped release of a temporary GX matrix allocation.
class CMatrixPoolGuard {
public:
  explicit CMatrixPoolGuard(bool active) : mActive(active) {}
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  void* mPtr;
  int mUnk1;
  ushort mUnk2;
};

namespace Skinning {
#if RSTL_VERSION >= RSTL_GM8P_00 && RSTL_VERSION != RSTL_GM8J_00
static void* sStaticSkinningData = nullptr;
static int sStaticSkinningDataSize = 0x80000;
#endif
static ushort skCurrentToken = 0;
static int sNumSkinnedObjects = 0;
static bool sSkinningInitialized = false;
#if RSTL_VERSION < RSTL_GM8P_00 || RSTL_VERSION == RSTL_GM8J_00
ATTRIBUTE_ALIGN_DECL(32, static char sStaticSkinningData[0x80000]);
#endif
static rstl::optional_object< CCircularBuffer > sSkinningBuffer;
static rstl::list< SSkinnedAllocation > sAllocations;
static bool sbDumpedSpinLockMessage = false;

void AddSkinnedRef();
void DelSkinnedRef();
#if RSTL_VERSION >= RSTL_GM8P_00 && RSTL_VERSION != RSTL_GM8J_00
void SetSkinningBuffer(void* buffer, int size);
#endif
} // namespace Skinning

#if RSTL_VERSION >= RSTL_GM8P_00 && RSTL_VERSION != RSTL_GM8J_00
void Skinning::SetSkinningBuffer(void* buffer, int size) {
  sStaticSkinningDataSize = size;
  sAllocations.clear();
  sStaticSkinningData = buffer;
  if (buffer != nullptr && sNumSkinnedObjects != 0) {
    sSkinningBuffer = CCircularBuffer(buffer, sStaticSkinningDataSize);
  }
}
#endif

void Skinning::AddSkinnedRef() {
  if (!sSkinningInitialized) {
    GXSetDrawSync(0xFFFF);
    while (GXReadDrawSync() != 0xFFFF) {
    }
    skCurrentToken = 1;
    sSkinningInitialized = true;
  }

#if RSTL_VERSION >= RSTL_GM8P_00
  if (sNumSkinnedObjects++ == 0) {
#if RSTL_VERSION == RSTL_GM8J_00
    sSkinningBuffer = CCircularBuffer(sStaticSkinningData, sizeof(sStaticSkinningData));
#else
    sSkinningBuffer = CCircularBuffer(sStaticSkinningData, sStaticSkinningDataSize);
#endif
  }
#else
  if (sNumSkinnedObjects == 0) {
    sSkinningBuffer = CCircularBuffer(sStaticSkinningData, sizeof(sStaticSkinningData));
  }
  ++sNumSkinnedObjects;
#endif
}

void Skinning::DelSkinnedRef() {
  --sNumSkinnedObjects;
  if (sNumSkinnedObjects == 0) {
    sSkinningBuffer.clear();
    sAllocations.clear();
#else
  ~CMatrixPoolGuard() {
    if (mActive) {
      GPUMemory::ReleaseAllocation();
#endif

  }
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_GM8P_00 && RSTL_VERSION != RSTL_GM8J_00
void CSkinnedModel::SetSkinningBuffer(void* buffer, int size) {
  Skinning::SetSkinningBuffer(buffer, size);
}
#endif
#else
private:
  bool mActive;
};

// Guessed names for the target's surface-drawing functors.
struct SDrawSurfaceFlat {
  void operator()(const CCubeModel& model, const CCubeSurface& surface,
                  const CModelFlags& flags) const;
};

struct SDrawSurface {
  void operator()(const CCubeModel& model, const CCubeSurface& surface,
                  const CModelFlags& flags) const;
};

struct SDrawFlat {
  void operator()(const CCubeModel& model, const CCubeSurface& surface,
                  const CModelFlags& flags) const;
};

struct SDrawMasked {
  explicit SDrawMasked(u64 mask) : mMask(mask) {}

  void operator()(const CCubeModel& model, const CCubeSurface& surface,
                  const CModelFlags& flags) const;

  u64 mMask;
};

struct SDrawFlatMasked {
  explicit SDrawFlatMasked(u64 mask) : mMask(mask) {}

  void operator()(const CCubeModel& model, const CCubeSurface& surface,
                  const CModelFlags& flags) const;

  u64 mMask;
};
} // namespace

template < class TDraw, class TFlatDraw >
void CSkinnedModel::DolphinDrawInternal(const SSkinningWorkspace& workspace, uint drawFlags,
                                        const CModelFlags& flags, const TDraw& draw,
                                        const TFlatDraw& flat) const {
  CTransform4f saved(CGraphics::GetModelMatrix());
  int currentBank = -1;
  mModel->PreDrawModel(flags);
  const CCubeModel* model = mModel->GetModelInstance();
  if (model == nullptr || !model->TryLockTextures()) {
    return;
  }

  CCubeSurface surfaces[] = {model->GetNormalSurfaces(), model->GetAlphaSurfaces()};
  model->SetArraysCurrent();
  CMatrixPoolGuard guard(workspace.mMatrices == nullptr);
  SSkinningMatrices* matrices = workspace.mMatrices;
  if (matrices == nullptr) {
    matrices = static_cast< SSkinningMatrices* >(
        GPUMemory::EnsureAllocation(mSkinRules->GetNumVirtualBones() * sizeof(SSkinningMatrices)));
    BuildSkinningMatrices(workspace.mTransforms, matrices, workspace.mUniformScale);
  }
  CGX::SetArray(GX_POS_MTX_ARRAY, matrices, sizeof(SSkinningMatrices));
  CGX::SetArray(GX_NRM_MTX_ARRAY, matrices->mNormal, sizeof(SSkinningMatrices));

  for (int i = 0; i < sizeof(surfaces) / sizeof(surfaces[0]); ++i) {
    if (!(drawFlags & kDF_Unsorted) && i == 0) {
      continue;
    }
    if (!(drawFlags & kDF_Sorted) && i == 1) {
      continue;
    }
    if (drawFlags & kDF_Flat) {
      for (CCubeSurface surface = surfaces[i]; surface.IsValid();
           surface = surface.GetNextSurface()) {
        LoadMatrixBank(currentBank, surface.GetMatrixBank());
        flat(*model, surface, flags);
      }
    } else {
      for (CCubeSurface surface = surfaces[i]; surface.IsValid();
           surface = surface.GetNextSurface()) {
        LoadMatrixBank(currentBank, surface.GetMatrixBank());
        draw(*model, surface, flags);
      }
    }
  }

  GXSetCurrentMtx(GX_PNMTX0);
  const int extraCoord = CCubeMaterial::GetExtraTexCoord();
  if (extraCoord != GX_TEXCOORD_NULL) {
    CGX::SetTexCoordGen(static_cast< GXTexCoordID >(extraCoord), GX_TG_MTX3x4, GX_TG_POS,
                        static_cast< GXTexMtx >(0), GX_FALSE,
                        static_cast< GXPTTexMtx >(CCubeMaterial::GetExtraPostTexMtx()));
    CCubeMaterial::ResetExtraTexCoord();
  }
  CGraphics::SetModelMatrix(saved);
}

CSkinnedModelState::CSkinnedModelState(int boneCount, bool transient) {
  mWorkspace.mTransforms =
      transient ? nullptr
                : static_cast< CTransform4f* >(CMemory::Alloc(boneCount * sizeof(CTransform4f)));
  mWorkspace.mMatrices = nullptr;
  mWorkspace.mBoneCount = boneCount;
  mWorkspace.mOwned = true;
  mWorkspace.mTransient = transient;
  mWorkspace.mUniformScale = false;
  if (mWorkspace.mTransient) {
    const uint transformSize = (boneCount * sizeof(CTransform4f) + 31) & ~31;
    const uint matrixSize = (boneCount * sizeof(SSkinningMatrices) + 31) & ~31;
    void* data = GPUMemory::EnsureAllocation(transformSize + matrixSize);
    mWorkspace.mTransforms = static_cast< CTransform4f* >(data);
    mWorkspace.mMatrices =
        reinterpret_cast< SSkinningMatrices* >(static_cast< uchar* >(data) + transformSize);
  }
}

CSkinnedModelState::CSkinnedModelState(const CSkinnedModelState& other) {
  mWorkspace.mTransforms = other.mWorkspace.mTransforms;
  mWorkspace.mMatrices = other.mWorkspace.mMatrices;
  mWorkspace.mBoneCount = other.mWorkspace.mBoneCount;
  mWorkspace.mOwned = other.mWorkspace.mOwned;
  mWorkspace.mTransient = other.mWorkspace.mTransient;
  mWorkspace.mUniformScale = other.mWorkspace.mUniformScale;
  other.mWorkspace.mOwned = false;
}

CSkinnedModelState::~CSkinnedModelState() {
  if (mWorkspace.mOwned) {
    if (mWorkspace.mTransient) {
      DCFlushRange(mWorkspace.mTransforms,
                   (mWorkspace.mBoneCount * sizeof(CTransform4f) + 31) & ~31);
      GPUMemory::ReleaseAllocation();
    } else {
      CMemory::Free(mWorkspace.mTransforms);
    }
  }
}
#endif

CSkinnedModel::CSkinnedModel(const TLockedToken< CModel >& model,
                             const TLockedToken< CSkinRules >& skinRules,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                             const TLockedToken< CCharLayoutInfo >& layoutInfo,
                             EDataOwnership ownership)
: mModel(model)
, mSkinRules(skinRules)
, mLayoutInfo(layoutInfo)
, mVertWorkspace()
, mNormalWorkspace()
, mOwned(ownership == kDO_Owned)
, mDisableWorkspaces(false) {
  Construct();
#else
                             const TLockedToken< CCharLayoutInfo >& layoutInfo)
: mModel(model), mSkinRules(skinRules), mLayoutInfo(layoutInfo) {}

CSkinnedModel::~CSkinnedModel() {}

CSkinnedModelState CSkinnedModel::MakeStorage(bool transient) const {
  return CSkinnedModelState(mSkinRules->GetNumVirtualBones(), transient);
#endif

}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CSkinnedModel::CSkinnedModel(const CSkinnedModel& other)
: mModel(other.mModel)
, mSkinRules(other.mSkinRules)
, mLayoutInfo(other.mLayoutInfo)
, mVertWorkspace()
, mNormalWorkspace()
, mOwned(other.mOwned)
, mDisableWorkspaces(false) {
  Construct();
#else
CSkinnedModelState CSkinnedModel::MakeDefaultStorage() const { return MakeStorage(false); }

void CSkinnedModel::StoreCalculation(CSkinnedModelState& state,
                                     const CPoseAsTransforms_Linear* pose) const {
  SSkinningWorkspace& workspace = state.mWorkspace;
  CTransform4f* transforms = workspace.mTransforms;
  const int count = mSkinRules->GetNumVirtualBones();
  bool uniformScale = false;
  if (pose == nullptr) {
    for (int i = 0; i < count; ++i) {
      *transforms = CTransform4f::Identity();
      ++transforms;
#endif

}
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CSkinnedModel::~CSkinnedModel() { Skinning::DelSkinnedRef(); }

void CSkinnedModel::Construct() {
  Skinning::AddSkinnedRef();
  if (!mOwned) {
    uint numPoints = mSkinRules->GetNumPoints();
    uint numNormals = mSkinRules->GetNumNormals();
    mVertWorkspace = rs_new float[numPoints * 12];
    mNormalWorkspace = rs_new float[numNormals * 12];
#else
  } else {
    mSkinRules->BuildAccumulatedTransforms(*pose, **mLayoutInfo, transforms);
    if (!pose->HasScale() && pose->HasUniformScale()) {
      uniformScale = true;
#endif

  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (mSkinRules->GetNumVirtualBones() == 1) {
    mDisableWorkspaces = true;
#else
  }
  workspace.mUniformScale = uniformScale;
  if (workspace.mMatrices != nullptr) {
    BuildSkinningMatrices(workspace.mTransforms, workspace.mMatrices, workspace.mUniformScale);
  }
  if (sPointGen != nullptr && mSkinRules->GetNumPoints() != 0) {
    sPointGen(*this, workspace, sPointGenData);
#endif

  }
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CSkinnedModel::Draw(const CModelFlags& flags) const {
  if (mDisableWorkspaces) {
    CTransform4f saved(CGraphics::GetModelMatrix());
    CGraphics::SetModelMatrix(saved * mSkinRules->GetVirtualBones()[0].GetTransform());
    mModel->Draw(flags);
    CGraphics::SetModelMatrix(saved);
  } else if (mVertWorkspace.null()) {
    mModel->Draw(flags);
  } else {
    mModel->Draw(mVertWorkspace.get(), mNormalWorkspace.get(), flags);
    PostDrawFunc();
#else
void CSkinnedModel::Draw(const CPoseAsTransforms_Linear* pose, const CModelFlags& flags) const {
  CSkinnedModelState state(MakeStorage(true));
  StoreCalculation(state, pose);
  DrawFromState(state, flags);
}

void CSkinnedModel::Draw(const CPoseAsTransforms_Linear* pose, TDrawFunc callback,
                         void* context) const {
  CSkinnedModelState state(MakeStorage(true));
  StoreCalculation(state, pose);
  callback(state.GetWorkspace(), context);
}

void CSkinnedModel::DolphinDrawWithFlags(const CPoseAsTransforms_Linear* pose, uint drawFlags,
                                         const CModelFlags& flags) const {
  CSkinnedModelState state(MakeStorage(true));
  StoreCalculation(state, pose);
  DolphinDrawFromWorkspace(state.GetWorkspace(), drawFlags, flags);
}

void CSkinnedModel::DolphinDrawFromWorkspace(const SSkinningWorkspace& workspace, uint drawFlags,
                                             const CModelFlags& flags) const {
  DolphinDrawInternal(workspace, drawFlags, flags, SDrawSurface(), SDrawFlat());
}

void CSkinnedModel::DolphinDrawFromWorkspace(const SSkinningWorkspace& workspace, uint drawFlags,
                                             const CModelFlags& flags, u64 mask) const {
  DolphinDrawInternal(workspace, drawFlags, flags, SDrawMasked(mask), SDrawFlatMasked(mask));
}

void CSkinnedModel::DrawFromState(const CSkinnedModelState& state, const CModelFlags& flags) const {
  DolphinDrawFromWorkspace(state.GetWorkspace(), kDF_Unsorted | kDF_Sorted, flags);
}

void CSkinnedModel::LoadMatrixBank(int& currentBank, int bank) const {
  if (bank != currentBank) {
    mSkinRules->LoadMatrixBank(bank);
    currentBank = bank;
#endif

  }
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CSkinnedModel::Draw(const TDrawFunc func, void* data) {
  if (mDisableWorkspaces) {
    CTransform4f saved(CGraphics::GetModelMatrix());
    CGraphics::SetModelMatrix(saved * mSkinRules->GetVirtualBones()[0].GetTransform());
    Draw(func, mModel->GetPositions(), mModel->GetNormals(), data);
    CGraphics::SetModelMatrix(saved);
  } else if (mVertWorkspace.null()) {
    Draw(func, mModel->GetPositions(), mModel->GetNormals(), data);
  } else {
    func(mVertWorkspace.get(), mNormalWorkspace.get(), data);
    uint vertSize = (mSkinRules->GetNumPoints() * 12 + 31) & ~31u;
    DCFlushRangeNoSync(mVertWorkspace.get(), vertSize);
    uint normSize = (mSkinRules->GetNumNormals() * 12 + 31) & ~31u;
    DCFlushRangeNoSync(mNormalWorkspace.get(), normSize);
    PPCSync();
    PostDrawFunc();
#else
void CSkinnedModel::BuildSkinningMatrices(const CTransform4f* transforms,
                                          SSkinningMatrices* matrices,
                                          const bool uniformScale) const {
  const rstl::vector< CVirtualBone >& bones = mSkinRules->GetVirtualBones();
  SSkinningMatrices* matrix = matrices;
  const CTransform4f* transform = transforms;
  for (rstl::vector< CVirtualBone >::const_iterator it = bones.begin(); it != bones.end();
       ++it, ++matrix, ++transform) {
    it->BuildSkinningMatrices(*transform, *matrix, uniformScale);
    DCFlushRange(matrix, sizeof(SSkinningMatrices));
#endif

  }
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CSkinnedModel::Draw(const float* positions, const float* normals,
                         const CModelFlags& flags) const {
  mModel->Draw(positions, normals, flags);
  PostDrawFunc();
}

void CSkinnedModel::Calculate(const CPoseAsTransforms& pose,
                              const rstl::optional_object< CVertexMorphEffect >& morphEffect,
                              const float* averagedNormals, float* workVerts) {
  size_t alignedNormSize = 0;
  size_t alignedVertSize = 0;
  size_t totalSize = 0;
  size_t vertSize = mSkinRules->GetNumPoints() * sizeof(CVector3f);
  size_t normSize = mSkinRules->GetNumNormals() * sizeof(CVector3f);
  float* verts;

  if (workVerts != nullptr) {
    verts = workVerts;
  } else {
    if (mDisableWorkspaces) {
      mSkinRules->BuildAccumulatedTransforms(pose, **mLayoutInfo);
      return;
    }
    AllocateStorage();
    verts = mVertWorkspace.get();
  }

  alignedNormSize = ((normSize + 31) & ~31u);
  alignedVertSize = ((vertSize + 31) & ~31u);
  totalSize = alignedVertSize + alignedNormSize;

  DCFlushRange(verts, totalSize);
  BOOL interruptState = OSDisableInterrupts();
  volatile void* pipe = GXRedirectWriteGatherPipe(verts);

  mSkinRules->InitLockedCacheState(**mModel);
  mSkinRules->BuildAccumulatedTransforms(pose, **mLayoutInfo);
  mSkinRules->BuildPoints(pipe);

  int numWords = mSkinRules->GetNumPoints() * 3;
  int padWords = ((numWords + 7) & ~7) - numWords;
  for (int i = 0; i < padWords; i++) {
    *reinterpret_cast< volatile u32* >(pipe) = 0;
  }

  mSkinRules->BuildNormals(pipe);
  GXRestoreWriteGatherPipe();
  OSRestoreInterrupts(interruptState);

  if (morphEffect.valid()) {
    (*morphEffect)
        .MorphVertices(reinterpret_cast< CVector3f* >(verts),
                       reinterpret_cast< const CVector3f* >(averagedNormals), mSkinRules, pose,
                       mSkinRules->GetNumPoints());
    DCFlushRange(verts, alignedVertSize);
  }

  if (sPointGen != nullptr) {
    const CVector3f* positions = reinterpret_cast< const CVector3f* >(verts);
    sPointGen(sPointGenData, positions, positions + mSkinRules->GetNumPoints(),
              mSkinRules->GetNumPoints());
    DCInvalidateRange(verts, totalSize);
  }
}

void CSkinnedModel::CalculateDefault() {
  mVertWorkspace = rstl::auto_ptr< float >();
  mNormalWorkspace = rstl::auto_ptr< float >();
}

void CSkinnedModel::TickAllocations() {
  int syncVal = GXReadDrawSync();
  if (syncVal > static_cast< int >(Skinning::skCurrentToken)) {
    syncVal -= 0x10000;
  }
  while (Skinning::sAllocations.size() != 0) {
    SSkinnedAllocation& front = Skinning::sAllocations.front();
    int tokenVal = static_cast< int >(front.mUnk2);
    if (tokenVal > static_cast< int >(Skinning::skCurrentToken)) {
      tokenVal -= 0x10000;
    }
    if (syncVal < tokenVal)
      break;
    Skinning::sSkinningBuffer->Free(front.mPtr, front.mUnk1);
    Skinning::sAllocations.pop_front();
  }
}

void* CSkinnedModel::EnsureAllocation(int size) {
  size = (size + 31) & ~31;
  void* ptr = Skinning::sSkinningBuffer->Alloc(size);
  if (ptr == nullptr && !Skinning::sbDumpedSpinLockMessage) {
    Skinning::sSkinningBuffer->GetAllocatedAmount();
    Skinning::sbDumpedSpinLockMessage = true;
  }
  s32 startTick = OSGetTick();
  while (ptr == nullptr) {
    TickAllocations();
    ptr = Skinning::sSkinningBuffer->Alloc(size);
    if (ptr == nullptr) {
      s32 currentTick = OSGetTick();
      if (OSTicksToMilliseconds(static_cast< uint >(currentTick - startTick)) > 60) {
#if RSTL_VERSION == RSTL_GM8J_00
        ushort token = GXReadDrawSync();
        for (AUTO(it, Skinning::sAllocations.begin()); it != Skinning::sAllocations.end(); ++it) {
        }
        Skinning::skCurrentToken = token;
        startTick = currentTick;
        GXSetDrawSync(token);
        int numSkinned = Skinning::sNumSkinnedObjects;
        for (int i = 0; i < numSkinned; ++i) {
          Skinning::DelSkinnedRef();
        }
        for (int i = 0; i < numSkinned; ++i) {
          Skinning::AddSkinnedRef();
        }
#else
        GXReadDrawSync();
        for (AUTO(it, Skinning::sAllocations.begin()); it != Skinning::sAllocations.end(); ++it) {
        }
        startTick = currentTick;
        GXSetDrawSync(Skinning::skCurrentToken);
        ++Skinning::skCurrentToken;
#endif
      }
    }
  }
  Skinning::sAllocations.push_back(SSkinnedAllocation(ptr, size, Skinning::skCurrentToken));
  return ptr;
}

void CSkinnedModel::AllocateStorage() {
  if (mOwned && (mVertWorkspace.null() || mNormalWorkspace.null())) {
    int vertexCount = mSkinRules->GetNumPoints();
    int normalCount = mSkinRules->GetNumNormals();
    TickAllocations();
    int normSize = (normalCount * 12 + 31) & ~31;
    int vertSize = (vertexCount * 12 + 31) & ~31;
    int totalSize = vertSize + normSize + 32;
    void* ptr = EnsureAllocation(totalSize);
    if (ptr == Skinning::sStaticSkinningData) {
      GXInvalidateVtxCache();
    }
    mVertWorkspace = rstl::auto_ptr< float >(static_cast< float* >(ptr));
    mNormalWorkspace =
        rstl::auto_ptr< float >(reinterpret_cast< float* >(static_cast< char* >(ptr) + vertSize));
    mVertWorkspace.release();
    mNormalWorkspace.release();
  }
}

void CSkinnedModel::PostDrawFunc() const {
  if (mOwned && !mVertWorkspace.null()) {
    mVertWorkspace = rstl::auto_ptr< float >();
    mNormalWorkspace = rstl::auto_ptr< float >();
    GXSetDrawSync(Skinning::skCurrentToken);
    ++Skinning::skCurrentToken;
  }
}

void CSkinnedModel::AddDummySkinnedModelRef() { Skinning::AddSkinnedRef(); }

void CSkinnedModel::RemoveDummySkinnedModelRef() { Skinning::DelSkinnedRef(); }

void CSkinnedModel::SetPointGeneratorFunc(void* data, void (*func)(void*, const CVector3f*,
                                                                   const CVector3f*, int)) {
  sPointGen = func;
  sPointGenData = data;
#else
void CSkinnedModel::SetPointGeneratorFunc(void* context, TPointGenFunc callback) {
  sPointGen = callback;
  sPointGenData = context;
#endif

}

void CSkinnedModel::ClearPointGeneratorFunc() { sPointGen = nullptr; }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
float* CSkinnedModel::AllocateNewWorkspace(float** nrmOut) {
  const CSkinRules* skinRules = *mSkinRules;
  int normalCount = skinRules->GetNumNormals();
  int vertexCount = skinRules->GetNumPoints();
  int alignedNormSize = (normalCount * 12 + 31) & ~31;
  int alignedVertSize = (vertexCount * 12 + 31) & ~31;
  int vertSize = vertexCount * 12;
  float* ptr = static_cast< float* >(
      CMemory::Alloc(((vertSize + 31) & ~31) + alignedNormSize, IAllocator::kHI_RoundUpLen));
  if (nrmOut != nullptr) {
#if NONMATCHING
    // Wrong variable used originally
    *nrmOut = reinterpret_cast< float* >(reinterpret_cast< char* >(ptr) + alignedVertSize);
#else
    *nrmOut = reinterpret_cast< float* >(reinterpret_cast< char* >(ptr) + alignedNormSize);
#endif
  }
  return ptr;
#else
CVector3f CSkinnedModel::GetSkinnedPosition(const SSkinningWorkspace& workspace, int vertex) const {
  const CTransform4f* transforms = workspace.mTransforms;
  const int bone = mSkinRules->GetVertexToBoneMap()[vertex];
  return transforms[bone] * reinterpret_cast< const CVector3f* >(mModel->GetPositions())[vertex];
#endif

}

#if !RSTL_PRIME1
CVector3f CSkinnedModel::GetSkinnedNormal(const SSkinningWorkspace& workspace, int vertex) const {
  const int bone = mSkinRules->GetVertexToBoneMap()[vertex];
  const CVector3f& normal = reinterpret_cast< const CVector3f* >(mModel->GetNormals())[vertex];
  const CTransform4f& transform = workspace.mTransforms[bone];
  if (workspace.mUniformScale) {
    return CUnitVector3f(transform.Rotate(normal));
  }
  return CUnitVector3f(transform.GetQuickInverse().TransposeRotate(normal));
}
#endif

#if !RSTL_PRIME1
namespace {
inline void SDrawSurfaceFlat::operator()(const CCubeModel& model, const CCubeSurface& surface,
                                         const CModelFlags& flags) const {
  const CCubeMaterial material = model.GetMaterial(surface);
  CGX::SetVtxDescv_Compressed(material.GetVertexDesc());
  CGX::CallDisplayList(surface.GetDisplayList(), surface.GetDisplayListSize());
}

inline void SDrawSurface::operator()(const CCubeModel& model, const CCubeSurface& surface,
                                     const CModelFlags& flags) const {
  model.DrawSurface(surface, flags);
}

inline void SDrawFlat::operator()(const CCubeModel& model, const CCubeSurface& surface,
                                  const CModelFlags& flags) const {
  SDrawSurfaceFlat()(model, surface, flags);
}

inline void SDrawMasked::operator()(const CCubeModel& model, const CCubeSurface& surface,
                                    const CModelFlags& flags) const {
  const CCubeMaterial material = model.GetMaterial(surface);
  if (material.GetMaterialMask() & mMask) {
    model.DrawSurface(surface, flags);
  }
}

inline void SDrawFlatMasked::operator()(const CCubeModel& model, const CCubeSurface& surface,
                                        const CModelFlags& flags) const {
  const CCubeMaterial material = model.GetMaterial(surface);
  if (material.GetMaterialMask() & mMask) {
    SDrawSurfaceFlat()(model, surface, flags);
  }
}
} // namespace
#endif
