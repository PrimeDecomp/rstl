#ifndef _CMODEL
#define _CMODEL
#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"
#endif

#if RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#else
#include "Kyoto/CFactoryFnReturn.hpp"
#endif
#include "rstl/types.h"

#include "Kyoto/CFactoryMgr.hpp"
#if !RSTL_CORRUPTION
#include "Kyoto/Graphics/CCubeModel.hpp"
#include "Kyoto/Graphics/CCubeSurface.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"

#endif
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"

class CAABox;
class CCubeModel;

class CModelFlags;
class IObjectStore;
#if RSTL_CORRUPTION
class CSimplePool;
#else
class CModel {
  struct SShader;
  friend struct SShader;
#endif

#if RSTL_CORRUPTION
class CModel {
#else
  struct SShader {
    rstl::vector< TCachedToken< CTexture > > mTextures;
    TModelData mData;
#if !RSTL_PRIME1
    CModel* mOwner;
    SShader* mPrev;
    SShader* mNext;
#endif

#if RSTL_PRIME1
    SShader(TModelData data) : mData(data) {};
#else
    SShader(TModelData data, CModel* owner);
    SShader(const SShader& other);
    ~SShader();
#endif

    void UnlockTextures() RSTL_ECHOES_CONST;
    void RemoveFromList();
    void MoveToThisFrameList();

  };
#endif
  static uint sTotalMemory;
#if !RSTL_CORRUPTION
#if RSTL_PRIME1
  static CModel* sThisFrameList;
  static CModel* sOneFrameList;
  static CModel* sTwoFrameList;
#else
  static SShader* sThisFrameList;
  static SShader* sOneFrameList;
  static SShader* sTwoFrameList;
#endif
#endif

public:
  enum EDrawFlatFlags {
    // Guessed names; values select the existing unsorted/sorted surface lists.
    kDF_Unsorted = 0,
    kDF_Sorted = 1,
    kDF_All = 2,
  };

  CModel(const rstl::auto_ptr< uchar >& data, int length, IObjectStore& store);
  ~CModel();
  void Touch(int) const;
  void Draw(const CModelFlags&) const;
  void Draw(u64 mask, const CModelFlags& flags) const;

  void DrawUnsortedParts(const CModelFlags& flags) const;
  void DrawSortedParts(const CModelFlags& flags) const;
#if !RSTL_CORRUPTION
  void Draw(TModelPositions positions, TModelNormals normals, const CModelFlags& flags) const;
#endif
  void DolphinDrawFlat(EDrawFlatFlags flags) const;
  void PreDrawModel(const CModelFlags& flags) const;

  bool IsLoaded(int matIdx) const;
#if !RSTL_CORRUPTION
  int GetNumMaterialSets() const { return mMatSets.size(); }
  const CAABox& GetAABB() const;
  const CAABox& GetBoundingBox() const { return mModelInstance->GetBoundingBox(); }
#endif
  bool IsDefinitelyOpaque() const;
#if RSTL_CORRUPTION
  bool HasUnsortedSurfaces() const; // Guessed name
  bool HasSortedSurfaces() const;   // Guessed name
  const CAABox& GetAABB() const;
#endif
  void UnlockTextures();

  const float* GetPositions() const;
  const float* GetNormals() const;
  const CCubeModel* GetModelInstance() const { return mModelInstance.get(); }
#if !RSTL_CORRUPTION
  const CCubeModel* GetCubeModel() const { return mModelInstance.get(); }

#endif
  void UpdateLastFrame() const;
  void VerifyCurrentShader(int shader) const;
  // Guessed name
  rstl::auto_ptr< uchar > GetData();
#if !RSTL_CORRUPTION
#if !defined(TARGET_PC)
  // Guessed name
  void RemapData(uchar* data);
#endif
#endif
  uint GetDataSize() const;
#if RSTL_CORRUPTION
  // Guessed name
  void RemapData(CSimplePool* pool, uchar* data);

#endif
  static void DisableTextureTimeout();
  static void EnableTextureTimeout();
  static void FrameDone();
  static void AddToTotal(uint amt) { sTotalMemory += amt; }
  static void RemoveFromTotal(uint amt) { sTotalMemory -= amt; }
  static uint GetTotalMemory() { return sTotalMemory; }

#if !RSTL_CORRUPTION
  void RemoveFromList() const;
  void MoveToThisFrameList() const;

#endif
private:
  void* SetupSkinMatrices() const;

  rstl::single_ptr< uchar > mData;
  uint mDataLen;
#if RSTL_CORRUPTION
  rstl::vector< void* > mSurfaces;
#else
#if !defined(TARGET_PC)
  CCubeModel::SurfaceList mSurfaces;
#endif

  mutable rstl::vector< SShader > mMatSets;
#endif
  rstl::single_ptr< CCubeModel > mModelInstance;
#if !RSTL_CORRUPTION
#if RSTL_PRIME1
  mutable short mCurrentMatxIdx;
  mutable short mLastMaterialFrame;
  mutable CModel* mPrev;
  mutable CModel* mNext;
#endif

#endif
  mutable uint mLastFrame;
#if RSTL_CORRUPTION
  mutable uint mCurrentMatxIdx : 16;
  uint mInitialTextureLocksReleased : 1;
  uint mHasSkinMatrices : 1;
#else
#if !RSTL_PRIME1
  mutable uint mCurrentMatxIdx : 16;
  uint mInitialTextureLocksReleased : 1;
  uint mHasSkinMatrices : 1;
#endif
#if defined(TARGET_PC)
  uint mResourceSize;
#endif
#endif
};
#if RSTL_CORRUPTION
CHECK_SIZEOF(CModel, 0x24)
#else
CHECK_SIZEOF(CModel, RSTL_PRIME1 ? 0x3c : 0x34)
#endif

#if RSTL_CORRUPTION
CFactoryFnReturn FModelFactory(const SObjectTag& tag, const rstl::auto_ptr< uchar >& ptr, int len,
#else
#if RSTL_PRIME1
inline bool CModel::IsDefinitelyOpaque() const {
  return mModelInstance.get() != nullptr && !mModelInstance->GetAlphaSurfaces().IsValid();
}

inline void CModel::UnlockTextures() {
  rstl::vector< SShader >::iterator matIter;
  for (matIter = mMatSets.begin() + 1; matIter != mMatSets.end(); ++matIter) {
    matIter->UnlockTextures();
  }
  mModelInstance->UnlockTextures();
}

inline void CModel::PreDrawModel(const CModelFlags& flags) const {
  CCubeMaterial::ResetCachedMaterials();
  MoveToThisFrameList();
  VerifyCurrentShader(flags.GetShaderSet());
}
#endif

RSTL_PRIME1_CONST CFactoryFnReturn FModelFactory(const SObjectTag& tag,
                                                 const rstl::auto_ptr< uchar >& ptr, int len,
#endif
                                                 const CVParamTransfer& xfer);

#endif // _CMODEL
