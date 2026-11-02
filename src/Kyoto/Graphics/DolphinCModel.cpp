#include "rstl/RstlVersions.h"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/CFrameDelayedKiller.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/CVParamTransfer.hpp"
#include "Kyoto/Graphics/CCubeMaterial.hpp"
#include "Kyoto/Graphics/CCubeModel.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#if !RSTL_PRIME1
#include "Kyoto/Graphics/DolphinGPUMemory.hpp"
#endif

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Graphics/CCubeSurface.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#if RSTL_PRIME1
#include "dolphin/gx/GXGeometry.h"
#include "dolphin/os/OSCache.h"
#include "dolphin/types.h"
#endif

#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/IObjectStore.hpp"
#include "Kyoto/TToken.hpp"
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
#include "dolphin/gx.h"
#include "dolphin/os.h"
#endif

static bool sIsTextureTimeoutEnabled = true;
uint CModel::sTotalMemory = 0;
#if RSTL_PRIME1
CModel* CModel::sThisFrameList = nullptr;
CModel* CModel::sOneFrameList = nullptr;
CModel* CModel::sTwoFrameList = nullptr;
#else
CModel::SShader* CModel::sThisFrameList = nullptr;
CModel::SShader* CModel::sOneFrameList = nullptr;
CModel::SShader* CModel::sTwoFrameList = nullptr;
#endif

static uint sFrameCounter = 0;
#if !RSTL_PRIME1
namespace {
// GX matrix arrays must be 32-byte aligned.
struct SSkinMatrices {
  ATTRIBUTE_ALIGN_DECL(32, CTransform4f mModelView);
  float mNormal[3][3];
};

class CSkinMatricesGuard {
public:
  CSkinMatricesGuard(void* matrices) : mActive(matrices != nullptr) {}
  ~CSkinMatricesGuard() {
    if (mActive) {
      GPUMemory::ReleaseAllocation();
      GXSetCurrentMtx(GX_PNMTX0);
    }
  }

private:
  bool mActive;
};
} // namespace
#endif

static uchar* MemoryFromPartData(uchar*& dataCur, int*& secSizeCur) {
  uchar* ret = *secSizeCur != 0 ? dataCur : nullptr;
  dataCur += *secSizeCur;
  secSizeCur++;
  return ret;
}

#if RSTL_PRIME1
CModel::CModel(const rstl::auto_ptr< uchar >& data, int length, IObjectStore& store)
: mData(data.release())
, mDataLen(length)
, mModelInstance(nullptr)
, mCurrentMatxIdx(0)
, mLastMaterialFrame(0)
#else
CModel::SShader::SShader(uchar* data, CModel* owner)
: mData(data), mOwner(owner), mPrev(nullptr), mNext(nullptr) {}

CModel::SShader::SShader(const SShader& other)
: mTextures(other.mTextures)
, mData(other.mData)
, mOwner(other.mOwner)
#endif

, mPrev(nullptr)
#if RSTL_PRIME1
, mNext(sThisFrameList)
, mLastFrame(CGraphics::GetFrameCounter() - 2) {
#else
, mNext(nullptr) {}
#endif

#if RSTL_PRIME1
  uint sectionSizeStart = 0x2c;
  uchar* dataPtr = data.get();
  const uint flags = *reinterpret_cast< const uint* >(dataPtr + 8);
  const uint version = *reinterpret_cast< const uint* >(dataPtr + 4);
  const uint visorFlags = (flags >> 1) & 1;
  const bool hasShortUvs = (flags >> 2) & 1;
  if (version == 1) {
    sectionSizeStart = 0x28;
  }
#else
CModel::SShader::~SShader() { RemoveFromList(); }
#endif

#if RSTL_PRIME1
  const int sectionCount = *reinterpret_cast< const int* >(dataPtr + 0x24);
  uchar* dataCur;
  int* sectionSizes = reinterpret_cast< int* >(dataPtr + sectionSizeStart);
  int numMatSets = 1;
  if (version >= 2) {
    numMatSets = *reinterpret_cast< const int* >(dataPtr + 0x28);
  }
  int* secSizeCur = sectionSizes;
  dataCur = dataPtr + ((sectionSizeStart + sectionCount * 4 + 31) & ~31);
  mMatSets.reserve(numMatSets);
  for (int i = 0; i < numMatSets; ++i) {
    mMatSets.push_back(SShader(MemoryFromPartData(dataCur, secSizeCur)));
    SShader& shader = mMatSets.back();
    CCubeModel::MakeTexturesFromMats(shader.mData, shader.mTextures, store, true);
    mDataLen += shader.mTextures.size() * 12;
  }

  const void* positions = reinterpret_cast< const void* >(MemoryFromPartData(dataCur, secSizeCur));
  const void* normals = reinterpret_cast< const void* >(MemoryFromPartData(dataCur, secSizeCur));
  const void* vtxColors = reinterpret_cast< const void* >(MemoryFromPartData(dataCur, secSizeCur));
  uint surfaceCount;
  const void* floatUvs = reinterpret_cast< const void* >(MemoryFromPartData(dataCur, secSizeCur));
  const void* shortUvs = nullptr;
  if (hasShortUvs) {
    shortUvs = reinterpret_cast< const void* >(MemoryFromPartData(dataCur, secSizeCur));
  }

  uint* surfaceInfo = reinterpret_cast< uint* >(MemoryFromPartData(dataCur, secSizeCur));
  surfaceCount = CBasics::SwapBytes(*surfaceInfo);
  mSurfaces.reserve(surfaceCount);

  for (uint i = 0; i < surfaceCount; ++i) {
    mSurfaces.push_back(MemoryFromPartData(dataCur, secSizeCur));
  }

  mModelInstance = rs_new CCubeModel(
      &mSurfaces, &mMatSets.front().mTextures, mMatSets.front().mData, positions,
      normals, vtxColors, floatUvs, shortUvs, *reinterpret_cast< const CAABox* >(dataPtr + 0xc),
      visorFlags ? 1 : 0, true, -1);
  sThisFrameList = this;
  if (mNext != nullptr) {
    mNext->mPrev = this;
  }
  mDataLen += mSurfaces.size() * 4;
  sTotalMemory += mDataLen;
  DCFlushRange(mData.get(), length);
}

CModel::~CModel() {
  RemoveFromList();
  RemoveFromTotal(mDataLen);
  const int frame = CGraphics::GetFrameCounter();
  if (mLastFrame == frame) {
    CFrameDelayedKiller::ScheduleDeletion(CFrameDelayedKiller::kWhichFrame_NextFrame,
                                          mData.release());
  } else if (mLastFrame == frame - 1) {
    CFrameDelayedKiller::ScheduleDeletion(CFrameDelayedKiller::kWhichFrame_ThisFrame,
                                          mData.release());
  }
}

void CModel::Draw(const CModelFlags& flags) const {
  if (flags.GetOtherFlags() & CModelFlags::kF_DrawNormal) {
    mModelInstance->DrawNormal(nullptr, nullptr, kSS_All);
  }

  CCubeMaterial::ResetCachedMaterials();
  MoveToThisFrameList();
  VerifyCurrentShader(flags.GetShaderSet());
  mModelInstance->Draw(flags);
}

void CModel::DrawUnsortedParts(const CModelFlags& flags) const {
  if (flags.GetOtherFlags() & CModelFlags::kF_DrawNormal) {
    mModelInstance->DrawNormal(nullptr, nullptr, kSS_Unsorted);
  }

  CCubeMaterial::ResetCachedMaterials();
  MoveToThisFrameList();
  VerifyCurrentShader(flags.GetShaderSet());
  mModelInstance->DrawNormal(flags);
}

void CModel::DrawSortedParts(const CModelFlags& flags) const {
  if (flags.GetOtherFlags() & CModelFlags::kF_DrawNormal) {
    mModelInstance->DrawNormal(nullptr, nullptr, kSS_Sorted);
  }

  CCubeMaterial::ResetCachedMaterials();
  MoveToThisFrameList();
  VerifyCurrentShader(flags.GetShaderSet());
  mModelInstance->DrawAlpha(flags);
}

void CModel::Draw(const float* positions, const float* normals, const CModelFlags& flags) const {
  if (flags.GetOtherFlags() & CModelFlags::kF_DrawNormal) {
    mModelInstance->DrawNormal(positions, normals, kSS_All);
  }
  CCubeMaterial::ResetCachedMaterials();
  MoveToThisFrameList();
  VerifyCurrentShader(flags.GetShaderSet());
  mModelInstance->Draw(positions, normals, flags);
}

void CModel::VerifyCurrentShader(int shader) const {
  if (shader >= mMatSets.size()) {
    shader = 0;
  }
  if (shader == mCurrentMatxIdx) {
    if (mLastMaterialFrame != 0 && mLastMaterialFrame <= sFrameCounter) {
      for (int i = 0; i < mMatSets.size(); ++i) {
        if (i != shader) {
          mMatSets[i].UnlockTextures();
        }
      }
      mLastMaterialFrame = 0;
    }
  } else {
    mCurrentMatxIdx = shader;
    SShader& material = mMatSets[shader];
    mModelInstance->RemapMaterialData(material.mData, &material.mTextures);
    if (mMatSets.size() > 1) {
      mLastMaterialFrame = sFrameCounter + 2;
    }
  }
}

const CFactoryFnReturn FModelFactory(const SObjectTag& tag, const rstl::auto_ptr< uchar >& ptr,
                                     int len, const CVParamTransfer& xfer) {
  rstl::rc_ptr< IVParamObj > obj = xfer.mObj;
  CSimplePool* pool = static_cast< TObjOwnerParam< CSimplePool* >* >(obj.GetPtr())->GetData();
  GXInvalidateVtxCache();
  return rs_new CModel(ptr, len, *pool);
}

const float* CModel::GetPositions() const {
  return static_cast< const float* >(mModelInstance->GetPositions());
}

const float* CModel::GetNormals() const {
  return static_cast< const float* >(mModelInstance->GetNormals());
}

void CModel::Touch(int shader) const {
  MoveToThisFrameList();
  VerifyCurrentShader(shader);
  if (mModelInstance->TryLockTextures()) {
    const rstl::vector< TCachedToken< CTexture > >& textures = mModelInstance->GetTextures();
    for (AUTO(it, textures.begin()); it != textures.end(); ++it) {
      if (it->GetObject()) {
        (void)it->GetObject()->LoadToMRAM();
      }
    }
  }
}

bool CModel::IsLoaded(int shader) const {
  VerifyCurrentShader(shader);

  const rstl::vector< TCachedToken< CTexture > >& textures = mModelInstance->GetTextures();
  for (AUTO(it, textures.begin()); it != textures.end(); ++it) {
    if (!it->IsLoaded()) {
      return false;
    }
  }
  return true;
}

void CModel::MoveToThisFrameList() const {
  const_cast< CModel* >(this)->UpdateLastFrame();
#else
void CModel::SShader::MoveToThisFrameList() {
#endif

  if (sThisFrameList == this) {
    return;
  }
  RemoveFromList();

#if RSTL_PRIME1
  if (sThisFrameList != nullptr) {
    mNext = sThisFrameList;
    mNext->mPrev = const_cast< CModel* >(this);
#else
  SShader* head = sThisFrameList;
  if (head != nullptr) {
    mNext = head;
    head->mPrev = this;
#endif

  }

#if RSTL_PRIME1
  sThisFrameList = const_cast< CModel* >(this);
#else
  sThisFrameList = this;
#endif

}

#if RSTL_PRIME1
void CModel::RemoveFromList() const {
#else
void CModel::SShader::RemoveFromList() {
#endif

  if (mPrev != nullptr) {
    mPrev->mNext = mNext;
  } else if (this == sThisFrameList) {
    sThisFrameList = mNext;
  } else if (this == sOneFrameList) {
    sOneFrameList = mNext;
  } else if (this == sTwoFrameList) {
    sTwoFrameList = mNext;
  }

  if (mNext != nullptr) {
    mNext->mPrev = mPrev;
  }

  mPrev = nullptr;
  mNext = nullptr;
}

#if RSTL_PRIME1
void CModel::SShader::UnlockTextures() {
  for (AUTO(it, mTextures.begin()); it != mTextures.end(); ++it) {
    it->Unlock();
#else
void CModel::SShader::UnlockTextures() const {
  for (rstl::vector< TCachedToken< CTexture > >::const_iterator it = mTextures.begin();
       it != mTextures.end(); ++it) {
    const_cast< TCachedToken< CTexture >& >(*it).Unlock();
#endif

  }
#if !RSTL_PRIME1
}

CModel::CModel(const rstl::auto_ptr< uchar >& data, int length, IObjectStore& store)
: mData(data.release())
, mDataLen(length)
, mModelInstance(nullptr)
, mLastFrame(CGraphics::GetFrameCounter() - 2)
, mCurrentMatxIdx(0)
, mInitialTextureLocksReleased(false)
, mHasSkinMatrices(false) {
  uchar* dataPtr = data.get();
  mHasSkinMatrices = *reinterpret_cast< const uint* >(dataPtr + 8) & 1;
  const int sectionCount = *reinterpret_cast< const int* >(dataPtr + 0x24);
  const uint flags = *reinterpret_cast< const uint* >(dataPtr + 8);
  const uint visorFlags = (flags >> 1) & 1;
  const uint hasShortUvs = (flags >> 2) & 1;
  const int numMatSets = *reinterpret_cast< const int* >(dataPtr + 0x28);

  uchar* dataCur = dataPtr + ((0x2c + sectionCount * 4 + 31) & ~31);
  int* secSizeCur = reinterpret_cast< int* >(dataPtr + 0x2c);
  mMatSets.reserve(numMatSets);
  for (int i = 0; i < numMatSets; ++i) {
    mMatSets.push_back_unsafe(SShader(MemoryFromPartData(dataCur, secSizeCur), this));
    SShader& shader = mMatSets.back();
    CCubeModel::MakeTexturesFromMats(shader.mData, shader.mTextures, store, true);
    for (int j = 0; j < shader.mTextures.size(); ++j) {
      shader.mTextures[j].Lock();
    }
    mDataLen += shader.mTextures.size() * 12;
  }

  const void* positions = MemoryFromPartData(dataCur, secSizeCur);
  const void* normals = MemoryFromPartData(dataCur, secSizeCur);
  const void* vtxColors = MemoryFromPartData(dataCur, secSizeCur);
  const void* floatUvs = MemoryFromPartData(dataCur, secSizeCur);
  const void* shortUvs = nullptr;
  if (hasShortUvs) {
    shortUvs = MemoryFromPartData(dataCur, secSizeCur);
  }

  const uint surfaceCount =
      CBasics::SwapBytes(*reinterpret_cast< uint* >(MemoryFromPartData(dataCur, secSizeCur)));
  mSurfaces.reserve(surfaceCount);
  for (uint i = 0; i < surfaceCount; ++i) {
    mSurfaces.push_back_unsafe(CCubeSurface(MemoryFromPartData(dataCur, secSizeCur)));
  }

  mModelInstance = rs_new CCubeModel(
      &mSurfaces, &mMatSets.front().mTextures, mMatSets.front().mData, positions, normals,
      vtxColors, floatUvs, shortUvs, *reinterpret_cast< const CAABox* >(dataPtr + 0xc),
      visorFlags ? 1 : 0, true, -1);
  mDataLen += mSurfaces.size() * 4;
  AddToTotal(mDataLen);
  DCFlushRange(mData.get(), length);
}

CModel::~CModel() {
  RemoveFromTotal(mDataLen);
  const int frame = CGraphics::GetFrameCounter();
  if (mLastFrame == frame) {
    CFrameDelayedKiller::ScheduleDeletion(CFrameDelayedKiller::kWhichFrame_NextFrame,
                                          mData.release());
  } else if (mLastFrame == frame - 1) {
    CFrameDelayedKiller::ScheduleDeletion(CFrameDelayedKiller::kWhichFrame_ThisFrame,
                                          mData.release());
  }
}

void CModel::PreDrawModel(const CModelFlags& flags) const {
  CCubeMaterial::ResetCachedMaterials();
  UpdateLastFrame();
  VerifyCurrentShader(flags.GetShaderSet());
}

void* CModel::SetupSkinMatrices() const {
  if (!mHasSkinMatrices) {
    return nullptr;
  }

  SSkinMatrices* matrices =
      static_cast< SSkinMatrices* >(GPUMemory::EnsureAllocation(sizeof(SSkinMatrices)));
  matrices->mModelView = CGraphics::GetGXModelView();
  float (*normal)[3] = matrices->mNormal;
  const Mtx& invXpose = CGraphics::GetGXModelViewInvXpose();
  for (int i = 0; i < 3; ++i) {
    normal[i][0] = invXpose[i][0];
    normal[i][1] = invXpose[i][1];
    normal[i][2] = invXpose[i][2];
  }
  DCFlushRange(matrices, 0x54);
  CGX::SetArray(GX_POS_MTX_ARRAY, &matrices->mModelView, sizeof(SSkinMatrices));
  CGX::SetArray(GX_NRM_MTX_ARRAY, normal, sizeof(SSkinMatrices));
  for (int i = 1; i < 10; ++i) {
    GXLoadPosMtxIndx(0, i * 3);
    GXLoadNrmMtxIndx3x3(0, i * 3);
  }
  return matrices;
}

void CModel::Draw(const CModelFlags& flags) const {
  PreDrawModel(flags);
  if (flags.GetOtherFlags() & CModelFlags::kF_DrawNormal) {
    mModelInstance->DrawNormal(kSS_All);
  }
  CSkinMatricesGuard guard(SetupSkinMatrices());
  mModelInstance->Draw(flags);
}

void CModel::Draw(u64 mask, const CModelFlags& flags) const {
  PreDrawModel(flags);
  if (flags.GetOtherFlags() & CModelFlags::kF_DrawNormal) {
    mModelInstance->DrawNormal(kSS_All);
  }
  CSkinMatricesGuard guard(SetupSkinMatrices());
  mModelInstance->Draw(mask, flags);
}

void CModel::DrawUnsortedParts(const CModelFlags& flags) const {
  PreDrawModel(flags);
  if (flags.GetOtherFlags() & CModelFlags::kF_DrawNormal) {
    mModelInstance->DrawNormal(kSS_Unsorted);
  }
  CSkinMatricesGuard guard(SetupSkinMatrices());
  mModelInstance->DrawNormal(flags);
}

void CModel::DrawSortedParts(const CModelFlags& flags) const {
  PreDrawModel(flags);
  if (flags.GetOtherFlags() & CModelFlags::kF_DrawNormal) {
    mModelInstance->DrawNormal(kSS_Sorted);
  }
  CSkinMatricesGuard guard(SetupSkinMatrices());
  mModelInstance->DrawAlpha(flags);
}

void CModel::DolphinDrawFlat(EDrawFlatFlags flags) const {
  CCubeMaterial::ResetCachedMaterials();
  UpdateLastFrame();
  CSkinMatricesGuard guard(SetupSkinMatrices());
  mModelInstance->DrawFlat(flags);
}

void CModel::VerifyCurrentShader(int shader) const {
  if (shader >= mMatSets.size()) {
    shader = 0;
  }
  SShader& material = mMatSets[shader];
  if (shader != mCurrentMatxIdx) {
    mModelInstance->RemapMaterialData(material.mData, &material.mTextures);
    mCurrentMatxIdx = shader;
  }
  if (mInitialTextureLocksReleased) {
    material.MoveToThisFrameList();
  }
}

CFactoryFnReturn FModelFactory(const SObjectTag& tag, const rstl::auto_ptr< uchar >& ptr, int len,
                               const CVParamTransfer& xfer) {
  rstl::rc_ptr< IVParamObj > obj = xfer.GetObj();
  CSimplePool* pool = static_cast< TObjOwnerParam< CSimplePool* >* >(obj.GetPtr())->GetData();
  GXInvalidateVtxCache();
  return rs_new CModel(ptr, len, *pool);
}

const float* CModel::GetPositions() const {
  return static_cast< const float* >(mModelInstance->GetPositions());
}

const float* CModel::GetNormals() const {
  return static_cast< const float* >(mModelInstance->GetNormals());
}

void CModel::Touch(int shader) const {
  UpdateLastFrame();
  VerifyCurrentShader(shader);
  mModelInstance->TryLockTextures();
}

bool CModel::IsLoaded(int shader) const {
  VerifyCurrentShader(shader);

  const rstl::vector< TCachedToken< CTexture > >& textures = mModelInstance->GetTextures();
  for (rstl::vector< TCachedToken< CTexture > >::const_iterator it = textures.begin();
       it != textures.end(); ++it) {
    if (!it->IsLoaded()) {
      return false;
    }
  }
  return true;
#endif

}

void CModel::FrameDone() {
  ++sFrameCounter;
  if (!sIsTextureTimeoutEnabled) {
    return;
  }

#if RSTL_PRIME1
  for (CModel* model = sTwoFrameList; model != nullptr;) {
    CModel* next = model->mNext;
    model->VerifyCurrentShader(0);
    for (AUTO(it, model->mMatSets.begin() + 1); it != model->mMatSets.end(); ++it) {
      it->UnlockTextures();
#else
  for (SShader* shader = sTwoFrameList; shader != nullptr;) {
    SShader* next = shader->mNext;
    CCubeModel* instance = shader->mOwner->mModelInstance.get();
    if (instance->GetTexturesPtr() == &shader->mTextures) {
      instance->UnlockTextures();
    } else {
      shader->UnlockTextures();
#endif

    }
#if RSTL_PRIME1
    model->mModelInstance->UnlockTextures();
    model->mPrev = model->mNext = nullptr;
    model = next;
#else
    shader->mNext = nullptr;
    shader->mPrev = nullptr;
    shader = next;
#endif

  }

  sTwoFrameList = sOneFrameList;
  sOneFrameList = sThisFrameList;
  sThisFrameList = nullptr;
}

void CModel::DisableTextureTimeout() { sIsTextureTimeoutEnabled = false; }

void CModel::EnableTextureTimeout() { sIsTextureTimeoutEnabled = true; }

uint CModel::GetDataSize() const { return mDataLen; }

rstl::auto_ptr< uchar > CModel::GetData() { return rstl::auto_ptr< uchar >(mData.get()); }

namespace {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#ifdef __MWERKS__
inline void RemapPointer(const void*& pointer, uintptr_t offset) {
  if (pointer != nullptr) {
    pointer = reinterpret_cast< void* >(reinterpret_cast< uintptr_t >(pointer) + offset);
  }
}
#endif
#else
inline void RemapPointer(const void*& pointer, uintptr_t offset) {
  if (pointer != nullptr) {
    pointer = reinterpret_cast< void* >(reinterpret_cast< uintptr_t >(pointer) + offset);
  }
}
#endif

template < typename T >
inline void RemapPointer(T*& pointer, uintptr_t offset) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#ifdef __MWERKS__
  RemapPointer(reinterpret_cast< const void*& >(pointer), offset);
#else
  if (pointer != nullptr) {
    pointer = reinterpret_cast< T* >(reinterpret_cast< uintptr_t >(pointer) + offset);
  }
#endif
#else
  RemapPointer(reinterpret_cast< const void*& >(pointer), offset);
#endif

}
} // namespace

void CModel::RemapData(uchar* data) {
  uintptr_t offset =
      reinterpret_cast< uintptr_t >(data) - reinterpret_cast< uintptr_t >(mData.release());
  mData = data;
  for (int i = 0; i < mMatSets.size(); ++i) {
    RemapPointer(mMatSets[i].mData, offset);
  }

  const CCubeModel::ModelInstance& instance = mModelInstance->GetModelInstance();
  const uchar* positions = static_cast< const uchar* >(instance.GetVertexPointer());
  const uchar* normals = static_cast< const uchar* >(instance.GetNormalPointer());
  const uchar* colors = static_cast< const uchar* >(instance.GetColorPointer());
  const uchar* uvs = static_cast< const uchar* >(instance.GetTCPointer());
  const uchar* packedUvs = static_cast< const uchar* >(instance.GetPackedTCPointer());
  const CAABox bounds = mModelInstance->GetBoundingBox();
  uchar flags = mModelInstance->GetModelFlags();
  bool texturesLoaded = mModelInstance->AreTexturesLoaded();
  const int index = mModelInstance->GetModelIndex();
  RemapPointer(positions, offset);
  RemapPointer(normals, offset);
  RemapPointer(colors, offset);
  RemapPointer(uvs, offset);
  RemapPointer(packedUvs, offset);
  for (int i = 0; i < mSurfaces.size(); ++i) {
#if RSTL_PRIME1
    RemapPointer(mSurfaces[i], offset);
#else
    RemapPointer(mSurfaces[i].mData, offset);
#endif

  }

  mModelInstance =
      rs_new CCubeModel(&mSurfaces, &mMatSets.front().mTextures, mMatSets.front().mData, positions,
                        normals, colors, uvs, packedUvs, bounds, flags, texturesLoaded, index);
#if RSTL_PRIME1
  MoveToThisFrameList();
#else
  UpdateLastFrame();
#endif

}

void CModel::UpdateLastFrame() const { mLastFrame = CGraphics::GetFrameCounter(); }

#if !RSTL_PRIME1
bool CModel::IsDefinitelyOpaque() const {
  return mModelInstance.get() != nullptr && !mModelInstance->GetAlphaSurfaces().IsValid();
}
#endif

#if !RSTL_PRIME1
const CAABox& CModel::GetAABB() const { return mModelInstance->GetBoundingBox(); }

void CModel::UnlockTextures() {
  if (mInitialTextureLocksReleased) {
    return;
  }

  mInitialTextureLocksReleased = true;
  for (int i = 0; i < mMatSets.size(); ++i) {
    mMatSets[i].UnlockTextures();
  }
}
#endif
