#include "rstl/RstlVersions.h"
#include "Kyoto/Alloc/Assert.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "rstl/macros.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#if RSTL_CORRUPTION
#else
#include "Kyoto/Basics/CBasics.hpp"
#endif
#include "Kyoto/CFrameDelayedKiller.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/CVParamTransfer.hpp"
#include "Kyoto/Graphics/CCubeMaterial.hpp"
#include "Kyoto/Graphics/CCubeModel.hpp"
#include "Kyoto/Graphics/CCubeSurface.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/Graphics/DolphinGPUMemory.hpp"
#endif
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Graphics/DolphinGPUMemory.hpp"
#include "Kyoto/IObjectStore.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#if RSTL_CORRUPTION
#include <rstl/types.h>
#else
#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/Platform.h"
#include "Kyoto/TToken.hpp"
#include "rstl/RstlVersions.h"
#endif

#include "dolphin/gx.h"
#include "dolphin/gx/GXGeometry.h"
#include "dolphin/os.h"
#include "dolphin/os/OSCache.h"
#include "dolphin/types.h"

#if defined(TARGET_PC)
#include "Metaforce/Game/CModelSectionReader.hpp"
#include <borealis/log.hpp>
#include <limits>

namespace {
constexpr borealis::Log Log{"CModel"};
} // namespace
#endif

static bool sIsTextureTimeoutEnabled = true;
uint CModel::sTotalMemory = 0;
#if !RSTL_CORRUPTION
#if RSTL_PRIME1
CModel* CModel::sThisFrameList = nullptr;
CModel* CModel::sOneFrameList = nullptr;
CModel* CModel::sTwoFrameList = nullptr;
#else
CModel::SShader* CModel::sThisFrameList = nullptr;
CModel::SShader* CModel::sOneFrameList = nullptr;
CModel::SShader* CModel::sTwoFrameList = nullptr;
#endif

#endif
static uint sFrameCounter = 0;
#if RSTL_CORRUPTION
static const uint skVersion = 5;

#endif
namespace {

// GX matrix arrays must be 32-byte aligned.
struct SSkinMatrices {
#if RSTL_CORRUPTION
  CTransform4f mModelView;
#else
  ATTRIBUTE_ALIGN_DECL(32, CTransform4f mModelView);
#endif
  float mNormal[3][3];
#if RSTL_CORRUPTION
} ATTRIBUTE_ALIGN(32);
#else
};
#endif

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

#if RSTL_CORRUPTION
static uchar* MemoryFromPartData(uchar*& dataCur, int*& secSizeCur) {
  uchar* ret = *secSizeCur != 0 ? dataCur : nullptr;
  dataCur += *secSizeCur;
  secSizeCur++;
  return ret;
}
#else
#if !defined(TARGET_PC)
static uchar* MemoryFromPartData(uchar*& dataCur, int*& secSizeCur) {
  uchar* ret = *secSizeCur != 0 ? dataCur : nullptr;
  dataCur += *secSizeCur;
  secSizeCur++;
  return ret;
}
#endif

#if !RSTL_PRIME1
CModel::SShader::SShader(TModelData data, CModel* owner)
: mData(data), mOwner(owner), mPrev(nullptr), mNext(nullptr) {}

CModel::SShader::SShader(const SShader& other)
: mTextures(other.mTextures)
, mData(other.mData)
, mOwner(other.mOwner)
, mPrev(nullptr)
, mNext(nullptr) {}

CModel::SShader::~SShader() { RemoveFromList(); }

void CModel::SShader::MoveToThisFrameList() {
  if (sThisFrameList == this) {
    return;
  }
  RemoveFromList();

  SShader* head = sThisFrameList;
  if (head != nullptr) {
    mNext = head;
    head->mPrev = this;
  }
  sThisFrameList = this;
}

void CModel::SShader::RemoveFromList() {
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

void CModel::SShader::UnlockTextures() const {
  for (rstl::vector< TCachedToken< CTexture > >::const_iterator it = mTextures.begin();
       it != mTextures.end(); ++it) {
    const_cast< TCachedToken< CTexture >& >(*it).Unlock();
  }
}
#endif
#endif

CModel::CModel(const rstl::auto_ptr< uchar >& data, int length, IObjectStore& store)
: mData(data.release())
, mDataLen(length)
, mModelInstance(nullptr)
#if RSTL_PRIME1
, mCurrentMatxIdx(0)
, mLastMaterialFrame(0)
, mPrev(nullptr)
, mNext(sThisFrameList)
#endif
, mLastFrame(CGraphics::GetFrameCounter() - 2)
#if RSTL_CORRUPTION
, mCurrentMatxIdx(0)
, mInitialTextureLocksReleased(false)
, mHasSkinMatrices(false) {
  uchar* byte_data = data.get();
  RS_VERIFY_THROW_DIRECT(87, uint32 ( byte_data ) == OSRoundUp32B ( uint32 ( byte_data ) ), false,
                         "Somehow got unaligned data");
  if (*reinterpret_cast< const uint* >(byte_data) != 0xdeadbabe) {
    RS_VERIFY_THROW(92, false, false, "Bad CModel signature");
  }
  // Guessed name
  struct SFakeHeader {
    uint magic;
    uint version;
  };
  const SFakeHeader* fakeHeader = reinterpret_cast< const SFakeHeader* >(byte_data);
  RS_VERIFY_THROW_DIRECT(94, fakeHeader->version == skVersion, false,
                         CBasics::Stringize("Bad model version %d (expected %d)",
                                           fakeHeader->version, skVersion));

  mHasSkinMatrices = *reinterpret_cast< const uint* >(byte_data + 8) & 1;
  const int sectionCount = *reinterpret_cast< const int* >(byte_data + 0x24);
  const uint flags = *reinterpret_cast< const uint* >(byte_data + 8);
  const uint visorFlags = (flags >> 1) & 1;
  const int numMatSets = *reinterpret_cast< const int* >(byte_data + 0x28);
  uchar* dataCur = byte_data + OSRoundUp32B(0x2c + sectionCount * 4);
  int* secSizeCur = reinterpret_cast< int* >(byte_data + 0x2c);
  const void* materialData = MemoryFromPartData(dataCur, secSizeCur);
  for (int i = 1; i < numMatSets; ++i) {
    MemoryFromPartData(dataCur, secSizeCur);
  }

  const void* positions = MemoryFromPartData(dataCur, secSizeCur);
  const void* normals = MemoryFromPartData(dataCur, secSizeCur);
  const void* vtxColors = MemoryFromPartData(dataCur, secSizeCur);
  const void* floatUvs = MemoryFromPartData(dataCur, secSizeCur);
  const uint surfaceCount =
      CBasics::SwapBytes(*reinterpret_cast< uint* >(MemoryFromPartData(dataCur, secSizeCur)));
  mSurfaces.reserve(surfaceCount);
  for (uint i = 0; i < surfaceCount; ++i) {
    mSurfaces.push_back_unsafe(MemoryFromPartData(dataCur, secSizeCur));
  }

  mModelInstance = rs_new_line(131)
      CCubeModel(static_cast< CSimplePool* >(&store), &mSurfaces, materialData, positions, normals,
                 vtxColors, floatUvs, *reinterpret_cast< const CAABox* >(byte_data + 0xc),
      visorFlags ? 1 : 0, true, -1);
  mModelInstance->TryLockTextures();
#else
#if !RSTL_PRIME1
, mCurrentMatxIdx(0)
, mInitialTextureLocksReleased(false)
, mHasSkinMatrices(false)
#endif
{
#if defined(TARGET_PC)
  if (length < 0 || mData.null()) {
    Log.fatal("Invalid CMDL resource");
  }
  mResourceSize = length;
  CModelSectionReader reader({mData.get(), mResourceSize});
  mMatSets.reserve(reader.GetMaterialSetCount());
  for (uint i = 0; i < reader.GetMaterialSetCount(); ++i) {
    mMatSets.push_back(SShader(reader.Next() IF_NOT_RSTL_PRIME1(, this)));
    auto& shader = mMatSets.back();
    CCubeModel::MakeTexturesFromMats(shader.mData, shader.mTextures, store, true);
  }
  // Guessed name
  const auto positions = reader.Next();
  const auto normals = reader.Next();
  const auto colors = reader.Next();
  const auto texCoords = reader.Next();
  const auto packedTexCoords = (reader.GetFlags() & 4) ? reader.Next() : std::span< uchar >{};
  const auto surfaceInfo = reader.Next();
  const uint surfaceCount = ReadModelSurfaceCount(surfaceInfo, reader.GetRemainingSections());

  uint materialCount = std::numeric_limits< uint >::max();
  for (const auto& shader : mMatSets) {
    const uint count = ValidateModelMaterials(shader.mData);
    if (count < materialCount) {
      materialCount = count;
    }
  }

  std::vector< TModelData > surfaces;
  surfaces.reserve(surfaceCount);
  for (uint i = 0; i < surfaceCount; ++i) {
    const auto surface = reader.Next();
    CCubeSurface::ReadData(surface, nullptr, materialCount);
    surfaces.emplace_back(surface);
  }

  const auto arrays = PrepareModelArrays(positions, normals, colors, texCoords, packedTexCoords,
                                         (reader.GetFlags() & 2) != 0);
  mModelInstance =
      rs_new CCubeModel(surfaces, &mMatSets.front().mTextures, mMatSets.front().mData, arrays,
                        reader.GetBounds(), (reader.GetFlags() >> 1) & 1, true, -1);
  size_t memory = mResourceSize + sizeof(CModel) + sizeof(CCubeModel) +
                  mMatSets.capacity() * sizeof(SShader) + mModelInstance->GetSurfaceStorageSize();
  for (const auto& shader : mMatSets) {
    memory += shader.mTextures.capacity() * sizeof(TCachedToken< CTexture >);
  }
  if (memory > std::numeric_limits< uint >::max()) {
    Log.fatal("CMDL memory accounting overflow");
  }
  mDataLen = memory;
#else

#if RSTL_PRIME1
  uint sectionSizeStart = 0x2c;
#endif
  uchar* dataPtr = data.get();
#if !RSTL_PRIME1
  mHasSkinMatrices = *reinterpret_cast< const uint* >(dataPtr + 8) & 1;
#endif
  const uint flags = *reinterpret_cast< const uint* >(dataPtr + 8);
#if RSTL_PRIME1
  const uint version = *reinterpret_cast< const uint* >(dataPtr + 4);
#endif
  const uint visorFlags = (flags >> 1) & 1;
  const uint hasShortUvs = (flags >> 2) & 1;
#if RSTL_PRIME1
  if (version == 1) {
    sectionSizeStart = 0x28;
  }
#endif
  const int sectionCount = *reinterpret_cast< const int* >(dataPtr + 0x24);
#if RSTL_PRIME1
  uchar* dataCur;
  int* sectionSizes = reinterpret_cast< int* >(dataPtr + sectionSizeStart);
  int numMatSets = 1;
  if (version >= 2) {
    numMatSets = *reinterpret_cast< const int* >(dataPtr + 0x28);
  }
  int* secSizeCur = sectionSizes;
  dataCur = dataPtr + ((sectionSizeStart + sectionCount * 4 + 31) & ~31);
#else
  const int numMatSets = *reinterpret_cast< const int* >(dataPtr + 0x28);
  uchar* dataCur = dataPtr + ((0x2c + sectionCount * 4 + 31) & ~31);
  int* secSizeCur = reinterpret_cast< int* >(dataPtr + 0x2c);
#endif
  mMatSets.reserve(numMatSets);
  for (int i = 0; i < numMatSets; ++i) {
#if RSTL_PRIME1
    mMatSets.push_back(SShader(MemoryFromPartData(dataCur, secSizeCur)));
#else
    mMatSets.push_back_unsafe(SShader(MemoryFromPartData(dataCur, secSizeCur), this));
#endif
    SShader& shader = mMatSets.back();
    CCubeModel::MakeTexturesFromMats(shader.mData, shader.mTextures, store, true);
#if !RSTL_PRIME1
    for (int j = 0; j < shader.mTextures.size(); ++j) {
      shader.mTextures[j].Lock();
    }
#endif
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
#if RSTL_PRIME1
    mSurfaces.push_back(MemoryFromPartData(dataCur, secSizeCur));
#else
    mSurfaces.push_back_unsafe(CCubeSurface(MemoryFromPartData(dataCur, secSizeCur)));
#endif
  }

  mModelInstance = rs_new CCubeModel(
      &mSurfaces, &mMatSets.front().mTextures, mMatSets.front().mData, positions, normals,
      vtxColors, floatUvs, shortUvs, *reinterpret_cast< const CAABox* >(dataPtr + 0xc),
      visorFlags ? 1 : 0, true, -1);
#endif
#if RSTL_PRIME1
  sThisFrameList = this;
  if (mNext != nullptr) {
    mNext->mPrev = this;
  }
#endif
#if !defined(TARGET_PC)
  mDataLen += mSurfaces.size() * 4;
#endif
#endif
  AddToTotal(mDataLen);
  DCFlushRange(mData.get(), length);
}

CModel::~CModel() {
#if RSTL_PRIME1
  RemoveFromList();
#endif
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

#if RSTL_CORRUPTION
void CModel::PreDrawModel(const CModelFlags& flags) const {
  CCubeMaterial::ResetCachedMaterials();
  UpdateLastFrame();
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
#else
#if !RSTL_PRIME1
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
  CGX_SET_ARRAY(GX_POS_MTX_ARRAY, &matrices->mModelView, sizeof(SSkinMatrices),
                sizeof(SSkinMatrices), true);
  CGX_SET_ARRAY(GX_NRM_MTX_ARRAY, normal, sizeof(SSkinMatrices),
                sizeof(SSkinMatrices) - offsetof(SSkinMatrices, mNormal), true);
  for (int i = 1; i < 10; ++i) {
    GXLoadPosMtxIndx(0, i * 3);
    CGX::LoadNrmMtxIndx3x3(0, i * 3);
  }
  return matrices;
}
#endif
#endif

void CModel::Draw(const CModelFlags& flags) const {
#if RSTL_CORRUPTION
  PreDrawModel(flags);
  if (mModelInstance->AreRequestedTexturesLoaded()) {
  CSkinMatricesGuard guard(SetupSkinMatrices());
    if (flags.GetOtherFlags() & CModelFlags::kF_DrawNormal) {
      mModelInstance->DrawNormal(kSS_All);
    }
#else
#if !RSTL_PRIME1
  PreDrawModel(flags);
#endif
  if (flags.GetOtherFlags() & CModelFlags::kF_DrawNormal) {
    mModelInstance->DrawNormal(IF_RSTL_PRIME1(TModelPositions(), TModelNormals(), ) kSS_All);
  }
#if RSTL_PRIME1
  PreDrawModel(flags);
#else
  CSkinMatricesGuard guard(SetupSkinMatrices());
#endif
#endif
  mModelInstance->Draw(flags);
}
#if RSTL_CORRUPTION
}
#endif

#if !RSTL_PRIME1
void CModel::Draw(u64 mask, const CModelFlags& flags) const {
  PreDrawModel(flags);
  if (flags.GetOtherFlags() & CModelFlags::kF_DrawNormal) {
    mModelInstance->DrawNormal(kSS_All);
  }
  CSkinMatricesGuard guard(SetupSkinMatrices());
  mModelInstance->Draw(mask, flags);
}
#endif

void CModel::DrawUnsortedParts(const CModelFlags& flags) const {
#if RSTL_CORRUPTION
  if (!HasUnsortedSurfaces()) {
    return;
#else
#if !RSTL_PRIME1
  PreDrawModel(flags);
#endif
  if (flags.GetOtherFlags() & CModelFlags::kF_DrawNormal) {
    mModelInstance->DrawNormal(IF_RSTL_PRIME1(TModelPositions(), TModelNormals(), ) kSS_Unsorted);
#endif
  }
#if RSTL_CORRUPTION
  PreDrawModel(flags);
  if (mModelInstance->AreRequestedTexturesLoaded()) {
  CSkinMatricesGuard guard(SetupSkinMatrices());
    if (flags.GetOtherFlags() & CModelFlags::kF_DrawNormal) {
      mModelInstance->DrawNormal(kSS_Unsorted);
    }
#else
#if RSTL_PRIME1
  PreDrawModel(flags);
#else
  CSkinMatricesGuard guard(SetupSkinMatrices());
#endif
#endif
  mModelInstance->DrawNormal(flags);
#if RSTL_CORRUPTION
  }
#endif
}

void CModel::DrawSortedParts(const CModelFlags& flags) const {
#if RSTL_CORRUPTION
  if (!HasSortedSurfaces()) {
    return;
  }
  PreDrawModel(flags);
  if (mModelInstance->AreRequestedTexturesLoaded()) {
    CSkinMatricesGuard guard(SetupSkinMatrices());
    if (flags.GetOtherFlags() & CModelFlags::kF_DrawNormal) {
      mModelInstance->DrawNormal(kSS_Sorted);
}
    mModelInstance->DrawAlpha(flags);
  }
}
#else
#if !RSTL_PRIME1
  PreDrawModel(flags);
#endif
  if (flags.GetOtherFlags() & CModelFlags::kF_DrawNormal) {
    mModelInstance->DrawNormal(IF_RSTL_PRIME1(TModelPositions(), TModelNormals(), ) kSS_Sorted);
  }
#if RSTL_PRIME1
  PreDrawModel(flags);
#else
  CSkinMatricesGuard guard(SetupSkinMatrices());
#endif
  mModelInstance->DrawAlpha(flags);
}

#if RSTL_PRIME1
void CModel::Draw(TModelPositions positions, TModelNormals normals,
                  const CModelFlags& flags) const {
  if (flags.GetOtherFlags() & CModelFlags::kF_DrawNormal) {
    mModelInstance->DrawNormal(positions, normals, kSS_All);
  }
  PreDrawModel(flags);
  mModelInstance->Draw(positions, normals, flags);
}
#endif
#endif

#if RSTL_CORRUPTION
bool CModel::HasSortedSurfaces() const {
  return mModelInstance->GetAlphaSurfaces().IsValid();
}

bool CModel::HasUnsortedSurfaces() const {
  return mModelInstance->GetNormalSurfaces().IsValid();
}

void CModel::DolphinDrawFlat(EDrawFlatFlags flags) const {
  CCubeMaterial::ResetCachedMaterials();
  UpdateLastFrame();
  CSkinMatricesGuard guard(SetupSkinMatrices());
  mModelInstance->DrawFlat(flags);
}
#else
#if !RSTL_PRIME1
void CModel::DolphinDrawFlat(EDrawFlatFlags flags) const {
  CCubeMaterial::ResetCachedMaterials();
  UpdateLastFrame();
  CSkinMatricesGuard guard(SetupSkinMatrices());
  mModelInstance->DrawFlat(flags);
}
#endif
#endif

#if RSTL_CORRUPTION
CFactoryFnReturn FModelFactory(const SObjectTag& tag, const rstl::auto_ptr< uchar >& ptr, int len,
#else
void CModel::VerifyCurrentShader(int shader) const {
  if (shader >= mMatSets.size()) {
    shader = 0;
  }
#if RSTL_PRIME1
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
#else
  SShader& material = mMatSets[shader];
  if (shader != mCurrentMatxIdx) {
    mModelInstance->RemapMaterialData(material.mData, &material.mTextures);
    mCurrentMatxIdx = shader;
  }
  if (mInitialTextureLocksReleased) {
    material.MoveToThisFrameList();
  }
#endif
}

RSTL_PRIME1_CONST CFactoryFnReturn FModelFactory(const SObjectTag& tag,
                                                 const rstl::auto_ptr< uchar >& ptr, int len,
#endif
                                                 const CVParamTransfer& xfer) {
  rstl::rc_ptr< IVParamObj > obj = xfer.GetObj();
  CSimplePool* pool = static_cast< TObjOwnerParam< CSimplePool* >* >(obj.GetPtr())->GetData();
  GXInvalidateVtxCache();
  return rs_new_line(335) CModel(ptr, len, *pool);
}

const float* CModel::GetPositions() const {
  return static_cast< const float* >(mModelInstance->GetPositions());
}

const float* CModel::GetNormals() const {
  return static_cast< const float* >(mModelInstance->GetNormals());
}

void CModel::Touch(int shader) const {
#if RSTL_CORRUPTION
  UpdateLastFrame();
  mModelInstance->TryLockTextures();
#else
#if RSTL_PRIME1
  MoveToThisFrameList();
#else
  UpdateLastFrame();
#endif
  VerifyCurrentShader(shader);
#if RSTL_PRIME1
  if (mModelInstance->TryLockTextures()) {
    const rstl::vector< TCachedToken< CTexture > >& textures = mModelInstance->GetTextures();
    for (AUTO(it, textures.begin()); it != textures.end(); ++it) {
      if (it->GetObject()) {
        (void)it->GetObject()->LoadToMRAM();
      }
    }
  }
#else
  mModelInstance->TryLockTextures();
#endif
#endif
}

bool CModel::IsLoaded(int shader) const {
  VerifyCurrentShader(shader);
#if RSTL_CORRUPTION
  return mModelInstance->AreRequestedTexturesLoaded();
#else

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

#if RSTL_CORRUPTION
void CModel::FrameDone() { ++sFrameCounter; }
#else
#if RSTL_PRIME1
void CModel::MoveToThisFrameList() const {
  const_cast< CModel* >(this)->UpdateLastFrame();
  if (sThisFrameList == this) {
    return;
  }
  RemoveFromList();

  if (sThisFrameList != nullptr) {
    mNext = sThisFrameList;
    mNext->mPrev = const_cast< CModel* >(this);
  }
  sThisFrameList = const_cast< CModel* >(this);
}

void CModel::RemoveFromList() const {
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

void CModel::SShader::UnlockTextures() {
  for (rstl::vector< TCachedToken< CTexture > >::const_iterator it = mTextures.begin();
       it != mTextures.end(); ++it) {
    const_cast< TCachedToken< CTexture >& >(*it).Unlock();
  }
}
#endif

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
    }
    model->mModelInstance->UnlockTextures();
    model->mPrev = model->mNext = nullptr;
    model = next;
  }
#else
  for (SShader* shader = sTwoFrameList; shader != nullptr;) {
    SShader* next = shader->mNext;
    CCubeModel* instance = shader->mOwner->mModelInstance.get();
    if (instance->GetTexturesPtr() == &shader->mTextures) {
      instance->UnlockTextures();
    } else {
      shader->UnlockTextures();
    }
    shader->mNext = nullptr;
    shader->mPrev = nullptr;
    shader = next;
  }
#endif

  sTwoFrameList = sOneFrameList;
  sOneFrameList = sThisFrameList;
  sThisFrameList = nullptr;
}
#endif

void CModel::DisableTextureTimeout() { sIsTextureTimeoutEnabled = false; }

void CModel::EnableTextureTimeout() { sIsTextureTimeoutEnabled = true; }

#if RSTL_CORRUPTION
uint CModel::GetDataSize() const { return mDataLen; }
#else
uint CModel::GetDataSize() const { return IF_TARGET_PC_OR(mResourceSize, mDataLen); }
#endif

rstl::auto_ptr< uchar > CModel::GetData() { return rstl::auto_ptr< uchar >(mData.get()); }

namespace {
#if RSTL_CORRUPTION
inline void RemapPointer(const void*& pointer, uintptr_t offset) {
  if (pointer != nullptr) {
    pointer = reinterpret_cast< void* >(reinterpret_cast< uintptr_t >(pointer) + offset);
  }
}
#else
#ifdef __MWERKS__
inline void RemapPointer(const void*& pointer, uintptr_t offset) {
  if (pointer != nullptr) {
    pointer = reinterpret_cast< void* >(reinterpret_cast< uintptr_t >(pointer) + offset);
  }
}
#endif
#endif

template < typename T >
inline void RemapPointer(T*& pointer, uintptr_t offset) {
#if RSTL_CORRUPTION
  RemapPointer(reinterpret_cast< const void*& >(pointer), offset);
#else
#ifdef __MWERKS__
  RemapPointer(reinterpret_cast< const void*& >(pointer), offset);
#else
  if (pointer != nullptr) {
    pointer = reinterpret_cast< T* >(reinterpret_cast< uintptr_t >(pointer) + offset);
  }
#endif
#endif
}
} // namespace

#if RSTL_CORRUPTION
void CModel::RemapData(CSimplePool* pool, uchar* data) {
  uintptr_t offset =
      reinterpret_cast< uintptr_t >(data) - reinterpret_cast< uintptr_t >(mData.release());
  mData = data;

  const CCubeModel::ModelInstance& instance = mModelInstance->GetModelInstance();
  const uchar* materialData = static_cast< const uchar* >(instance.GetMaterialPointer());
  const uchar* positions = static_cast< const uchar* >(instance.GetVertexPointer());
  const uchar* normals = static_cast< const uchar* >(instance.GetNormalPointer());
  const uchar* colors = static_cast< const uchar* >(instance.GetColorPointer());
  const uchar* uvs = static_cast< const uchar* >(instance.GetTCPointer());
  const CAABox bounds = mModelInstance->GetBoundingBox();
  uchar flags = mModelInstance->GetModelFlags();
  bool texturesLoaded = mModelInstance->ShouldDeferTextureLoad();
  const int index = mModelInstance->GetModelIndex();
  RemapPointer(materialData, offset);
  RemapPointer(positions, offset);
  RemapPointer(normals, offset);
  RemapPointer(colors, offset);
  RemapPointer(uvs, offset);
  for (int i = 0; i < mSurfaces.size(); ++i) {
    RemapPointer(mSurfaces[i], offset);
  }

  mModelInstance = rs_new_line(452)
      CCubeModel(pool, &mSurfaces, materialData, positions, normals, colors, uvs, bounds, flags,
                 texturesLoaded, index);
  UpdateLastFrame();
}
#else
#if !defined(TARGET_PC)
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
    RemapPointer(IF_RSTL_PRIME1_ELSE(mSurfaces[i], mSurfaces[i].mData), offset);
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
#endif
#endif

void CModel::UpdateLastFrame() const { mLastFrame = CGraphics::GetFrameCounter(); }

#if !RSTL_PRIME1
bool CModel::IsDefinitelyOpaque() const {
  return mModelInstance.get() != nullptr && !mModelInstance->GetAlphaSurfaces().IsValid();
}
#endif

const CAABox& CModel::GetAABB() const { return mModelInstance->GetBoundingBox(); }

#if RSTL_CORRUPTION
void CModel::UnlockTextures() {
  if (mInitialTextureLocksReleased) {
    return;
  }

  mInitialTextureLocksReleased = true;
      }

void CModel::VerifyCurrentShader(int shader) const {}
#else
#if !RSTL_PRIME1
void CModel::UnlockTextures() {
  if (mInitialTextureLocksReleased) {
    return;
  }

  mInitialTextureLocksReleased = true;
  for (int i = 0; i < mMatSets.size(); ++i) {
    if (IS_TARGET_PC) {
      // It might be loaded already
      // TODO verify this
      if (mModelInstance->GetTexturesPtr() == &mMatSets[i].mTextures) {
        mModelInstance->UnlockTextures();
        continue;
      }
    }
    mMatSets[i].UnlockTextures();
  }
}
#endif
#endif
