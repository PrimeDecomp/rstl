#ifndef _CMODEL
#define _CMODEL
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"

#include "Kyoto/CFactoryFnReturn.hpp"
#include "rstl/types.h"

#include "Kyoto/CFactoryMgr.hpp"
#include "Kyoto/Graphics/CCubeModel.hpp"
#include "Kyoto/Graphics/CCubeSurface.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"

#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"

class CAABox;
class CCubeModel;

class CModelFlags;
class IObjectStore;
class CModel {
  struct SShader;
  friend struct SShader;

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
  static uint sTotalMemory;
#if RSTL_PRIME1
  static CModel* sThisFrameList;
  static CModel* sOneFrameList;
  static CModel* sTwoFrameList;
#else
  static SShader* sThisFrameList;
  static SShader* sOneFrameList;
  static SShader* sTwoFrameList;
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
  void Draw(TModelPositions positions, TModelNormals normals, const CModelFlags& flags) const;
  void DolphinDrawFlat(EDrawFlatFlags flags) const;
  void PreDrawModel(const CModelFlags& flags) const;

  bool IsLoaded(int matIdx) const;
  int GetNumMaterialSets() const { return mMatSets.size(); }
  const CAABox& GetAABB() const;
  const CAABox& GetBoundingBox() const { return mModelInstance->GetBoundingBox(); }
  bool IsDefinitelyOpaque() const;
  void UnlockTextures();

  const float* GetPositions() const;
  const float* GetNormals() const;
  const CCubeModel* GetModelInstance() const { return mModelInstance.get(); }
  const CCubeModel* GetCubeModel() const { return mModelInstance.get(); }

  void UpdateLastFrame() const;
  void VerifyCurrentShader(int shader) const;

  rstl::auto_ptr< uchar > GetData();
#if !defined(TARGET_PC)
  // Retail buffer relocation methods; names are inferred from their implementations.
  void RemapData(uchar* data);
#endif
  uint GetDataSize() const;
  static void DisableTextureTimeout();
  static void EnableTextureTimeout();
  static void FrameDone();
  static void AddToTotal(uint amt) { sTotalMemory += amt; }
  static void RemoveFromTotal(uint amt) { sTotalMemory -= amt; }
  static uint GetTotalMemory() { return sTotalMemory; }

  void RemoveFromList() const;
  void MoveToThisFrameList() const;

private:
  void* SetupSkinMatrices() const;

  rstl::single_ptr< uchar > mData;
  uint mDataLen;
#if !defined(TARGET_PC)
  CCubeModel::SurfaceList mSurfaces;
#endif

  mutable rstl::vector< SShader > mMatSets;
  rstl::single_ptr< CCubeModel > mModelInstance;
#if RSTL_PRIME1
  mutable short mCurrentMatxIdx;
  mutable short mLastMaterialFrame;
  mutable CModel* mPrev;
  mutable CModel* mNext;
#endif

  mutable uint mLastFrame;
#if !RSTL_PRIME1
  mutable uint mCurrentMatxIdx : 16;
  uint mInitialTextureLocksReleased : 1;
  uint mHasSkinMatrices : 1;
#endif
#if defined(TARGET_PC)
  uint mResourceSize;
#endif
};
CHECK_SIZEOF(CModel, RSTL_PRIME1 ? 0x3c : 0x34)

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
                                                 const CVParamTransfer& xfer);

#endif // _CMODEL
