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

#if !RSTL_PRIME1
class CAABox;
class CCubeModel;
#endif

class CModelFlags;
class IObjectStore;
class CModel {
#if !RSTL_PRIME1
  struct SShader;
  friend struct SShader;

  // Echoes tracks texture timeouts per material set instead of per model.
#endif

  struct SShader {
    rstl::vector< TCachedToken< CTexture > > mTextures;
    uchar* mData;
#if !RSTL_PRIME1
    CModel* mOwner;
    SShader* mPrev;
    SShader* mNext;
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    SShader(uchar* data) : mData(data) {};
#else
    SShader(uchar* data, CModel* owner);
    SShader(const SShader& other);
    ~SShader();
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    void UnlockTextures();
#else
    void UnlockTextures() const;
    void RemoveFromList();
    void MoveToThisFrameList();
#endif

  };
  static uint sTotalMemory;
// TODO: verify that this Prime/Echoes difference is real
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
#if !RSTL_PRIME1
  enum EDrawFlatFlags {
    // Guessed names; values select the existing unsorted/sorted surface lists.
    kDF_Unsorted = 0,
    kDF_Sorted = 1,
    kDF_All = 2,
  };
#endif

  CModel(const rstl::auto_ptr< uchar >& data, int length, IObjectStore& store);
  ~CModel();
  void Touch(int) const;
  void Draw(const CModelFlags&) const;
#if !RSTL_PRIME1
  void Draw(u64 mask, const CModelFlags& flags) const;
#endif

  void DrawUnsortedParts(const CModelFlags& flags) const;
  void DrawSortedParts(const CModelFlags& flags) const;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  void Draw(const float* positions, const float* normals, const CModelFlags& flags) const;
#else
  void DolphinDrawFlat(EDrawFlatFlags flags) const;
  void PreDrawModel(const CModelFlags& flags) const;
#endif

  bool IsLoaded(int matIdx) const;
#if !RSTL_PRIME1
  bool IsDefinitelyOpaque() const;
  int GetNumMaterialSets() const { return mMatSets.size(); }
  const CAABox& GetAABB() const;
  void UnlockTextures();
#endif

  const float* GetPositions() const;
  const float* GetNormals() const;
#if !RSTL_PRIME1
  const CCubeModel* GetModelInstance() const { return mModelInstance.get(); }
#endif

  void UpdateLastFrame() const;
#if !RSTL_PRIME1
  void VerifyCurrentShader(int shader) const;
#endif

  // Retail buffer relocation methods; names are inferred from their implementations.
  rstl::auto_ptr< uchar > GetData();
  uint GetDataSize() const;
  void RemapData(uchar* data);
#if RSTL_PRIME1
  const CCubeModel* GetCubeModel() const { return mModelInstance.get(); }
  const CAABox& GetBoundingBox() const { return mModelInstance->GetBoundingBox(); }
  int GetNumMaterialSets() const { return mMatSets.size(); }
  bool IsDefinitelyOpaque() const {
    return mModelInstance.get() != nullptr && !mModelInstance->GetAlphaSurfaces().IsValid();
  }
#endif

  static void DisableTextureTimeout();
  static void EnableTextureTimeout();
  static void FrameDone();
  static void AddToTotal(uint amt) { sTotalMemory += amt; }
  static void RemoveFromTotal(uint amt) { sTotalMemory -= amt; }
  static uint GetTotalMemory() { return sTotalMemory; }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  void RemoveFromList() const;
  void MoveToThisFrameList() const;
  void VerifyCurrentShader(int shader) const;
#else
private:
  void* SetupSkinMatrices() const;
#endif

#if RSTL_PRIME1
  void UnlockTextures() {
    rstl::vector< SShader >::iterator matIter;
    for (matIter = mMatSets.begin() + 1; matIter != mMatSets.end(); ++matIter) {
      matIter->UnlockTextures();
    }
    mModelInstance->UnlockTextures();
  }

private:
#endif

  rstl::single_ptr< uchar > mData;
  uint mDataLen;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  rstl::vector< void* > mSurfaces;
#else
  rstl::vector< CCubeSurface > mSurfaces;
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

};
#if !RSTL_PRIME1
CHECK_SIZEOF(CModel, 0x34)
#endif

// TODO: verify that this Prime/Echoes difference is real
RSTL_PRIME1_CONST CFactoryFnReturn FModelFactory(const SObjectTag& tag, const rstl::auto_ptr< uchar >& ptr,

                                     int len, const CVParamTransfer& xfer);

#endif // _CMODEL
