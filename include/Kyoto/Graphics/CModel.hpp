#ifndef _CMODEL
#define _CMODEL
#include "rstl/RstlVersions.h"

#include "Kyoto/CFactoryFnReturn.hpp"
#include "types.h"

#include "Kyoto/Graphics/CCubeModel.hpp"
#include "Kyoto/CFactoryMgr.hpp"
#include "Kyoto/Graphics/CCubeSurface.hpp"

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
    uchar* mData;
#if !RSTL_PRIME1
    CModel* mOwner;
    SShader* mPrev;
    SShader* mNext;
#endif

#if RSTL_PRIME1
    SShader(uchar* data) : mData(data) {};
#else
    SShader(uchar* data, CModel* owner);
    SShader(const SShader& other);
    ~SShader();
#endif

#if RSTL_PRIME1
    void UnlockTextures();
#else
    void UnlockTextures() const;
#endif
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
  void Draw(const float* positions, const float* normals, const CModelFlags& flags) const;
  void DolphinDrawFlat(EDrawFlatFlags flags) const;
  void PreDrawModel(const CModelFlags& flags) const;

  bool IsLoaded(int matIdx) const;
  int GetNumMaterialSets() const { return mMatSets.size(); }
  const CAABox& GetAABB() const;
  const CAABox& GetBoundingBox() const { return mModelInstance->GetBoundingBox(); }
#if RSTL_PRIME1
  bool IsDefinitelyOpaque() const {
    return mModelInstance.get() != nullptr && !mModelInstance->GetAlphaSurfaces().IsValid();
  }
  void UnlockTextures() {
    rstl::vector< SShader >::iterator matIter;
    for (matIter = mMatSets.begin() + 1; matIter != mMatSets.end(); ++matIter) {
      matIter->UnlockTextures();
    }
    mModelInstance->UnlockTextures();
  }
#else
  bool IsDefinitelyOpaque() const;
  void UnlockTextures();
#endif

  const float* GetPositions() const;
  const float* GetNormals() const;
  const CCubeModel* GetModelInstance() const { return mModelInstance.get(); }
  const CCubeModel* GetCubeModel() const { return mModelInstance.get(); }

  void UpdateLastFrame() const;
  void VerifyCurrentShader(int shader) const;

  // Retail buffer relocation methods; names are inferred from their implementations.
  rstl::auto_ptr< uchar > GetData();
  uint GetDataSize() const;
  void RemapData(uchar* data);
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
  CCubeModel::SurfaceList mSurfaces;

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

};
CHECK_SIZEOF(CModel, RSTL_PRIME1 ? 0x3c : 0x34)

RSTL_PRIME1_CONST CFactoryFnReturn FModelFactory(const SObjectTag& tag,
                                                 const rstl::auto_ptr< uchar >& ptr, int len,
                                                 const CVParamTransfer& xfer);

#endif // _CMODEL
