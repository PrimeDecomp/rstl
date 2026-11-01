#ifndef _CSKINNEDMODEL
#define _CSKINNEDMODEL
#include "rstl/RstlVersions.h"

#include "types.h"

#include "Kyoto/TToken.hpp"
#if RSTL_PRIME1
#include "Kyoto/Animation/CSkinRules.hpp"
#include "Kyoto/Graphics/CModel.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/optional_object.hpp"

#if NONMATCHING
#include "Kyoto/Animation/CVertexMorphEffect.hpp"
#endif
#endif

class CModel;
class CSkinRules;
class CCharLayoutInfo;

class CModelFlags;
class CPoseAsTransforms;
class CTransform4f;

class CVector3f;
class CVertexMorphEffect; // This must be a forward declaration for GC matching

#if !RSTL_PRIME1
class CPoseAsTransforms_Linear;
struct SSkinningMatrices;

// Reconstructed workspace grouping; the field offsets are established by G2ME01.
struct SSkinningWorkspace {
  CTransform4f* mTransforms;
  SSkinningMatrices* mMatrices;
  int mBoneCount;
  mutable bool mOwned;
  bool mTransient;
  bool mUniformScale;
};
CHECK_SIZEOF(SSkinningWorkspace, 0x10)

class CSkinnedModelState {
  friend class CSkinnedModel;

public:
  CSkinnedModelState(int boneCount, bool transient);
  CSkinnedModelState(const CSkinnedModelState& other);
  ~CSkinnedModelState();

  const SSkinningWorkspace& GetWorkspace() const { return mWorkspace; }

private:
  CSkinnedModelState& operator=(const CSkinnedModelState&);
  SSkinningWorkspace mWorkspace;
};
CHECK_SIZEOF(CSkinnedModelState, 0x10)
#endif

class CSkinnedModel {
public:
  // Shared by both games.
  TLockedToken< CModel >& Model() { return mModel; }
  const TLockedToken< CModel >& GetModel() const { return mModel; }
  const TLockedToken< CSkinRules >& GetSkinRules() const { return mSkinRules; }
  const TLockedToken< CCharLayoutInfo >& GetLayoutInfo() const { return mLayoutInfo; }

  static void ClearPointGeneratorFunc();

#if RSTL_PRIME1
  // Prime has a vtable (__vt__13CSkinnedModel), a copy constructor and an ownership
  // parameter; Echoes has none of them.
  enum EDataOwnership {
    kDO_Unowned,
    kDO_Owned,
  };
  typedef void (*TPointGenFunc)(void*, const CVector3f*, const CVector3f*, int);
  typedef void (*TDrawFunc)(const float*, const float*, const void*);

  CSkinnedModel(const CSkinnedModel&);
  CSkinnedModel(const TLockedToken< CModel >&, const TLockedToken< CSkinRules >&,
                const TLockedToken< CCharLayoutInfo >&, EDataOwnership);
  virtual ~CSkinnedModel();

  void SetLayoutInfo(const TLockedToken< CCharLayoutInfo >& layout) { mLayoutInfo = layout; }

  // Prime skins into float position/normal workspaces.
  void CalculateDefault();
  int GetNumPoints() const { return mSkinRules->GetNumPoints(); }
  int GetNumVirtualBones() const { return mSkinRules->GetNumVirtualBones(); }
  const CVector3f* GetPositions() const {
    return reinterpret_cast< const CVector3f* >(mModel->GetPositions());
  }
  const CVector3f* GetNormals() const {
    return reinterpret_cast< const CVector3f* >(mModel->GetNormals());
  }
  void Calculate(const CPoseAsTransforms&, const rstl::optional_object< CVertexMorphEffect >&,
                 const float*, float*);
  void Draw(const CModelFlags&) const;
  void Draw(const float* positions, const float* normals, const CModelFlags& flags) const;
  void Draw(TDrawFunc func, void* data);
  void PostDrawFunc() const;
  float* AllocateNewWorkspace(float** nrmOut);

  static void SetPointGeneratorFunc(void*,
                                    void (*)(void*, const CVector3f*, const CVector3f*, int));
#if RSTL_VERSION >= RSTL_GM8P_00 && RSTL_VERSION != RSTL_GM8J_00
  static void SetSkinningBuffer(void* buffer, int size);
#endif
  static void AddDummySkinnedModelRef();
  static void RemoveDummySkinnedModelRef();

  void Construct();
  void AllocateStorage();
  static void TickAllocations();
  static void* EnsureAllocation(int size);

  static void Draw(const TDrawFunc func, const float* positions, const float* normals, void* data) {
    func(positions, normals, data);
  }
#else
  // Echoes skins from a pose into a matrix workspace (SSkinningWorkspace).
  typedef void (*TPointGenFunc)(const CSkinnedModel&, const SSkinningWorkspace&, void*);
  typedef void (*TDrawFunc)(const SSkinningWorkspace&, void*);
  // Guessed flag names.
  enum EDrawFlags { kDF_Unsorted = 2, kDF_Sorted = 4, kDF_Flat = 8 };

  CSkinnedModel(const TLockedToken< CModel >& model, const TLockedToken< CSkinRules >& skinRules,
                const TLockedToken< CCharLayoutInfo >& layoutInfo);
  ~CSkinnedModel();

  void SetLayoutInfo(const TToken< CCharLayoutInfo >& layout) { mLayoutInfo = layout; }

  CSkinnedModelState MakeDefaultStorage() const;
  CSkinnedModelState MakeStorage(bool transient) const; // Guessed name.
  void StoreCalculation(CSkinnedModelState& state, const CPoseAsTransforms_Linear* pose) const;
  void Draw(const CPoseAsTransforms_Linear* pose, const CModelFlags& flags) const;
  void Draw(const CPoseAsTransforms_Linear* pose, TDrawFunc callback, void* context) const;
  void DolphinDrawWithFlags(const CPoseAsTransforms_Linear* pose, uint drawFlags,
                            const CModelFlags& flags) const;
  void DrawFromState(const CSkinnedModelState& state, const CModelFlags& flags) const;
  // Guessed names for the workspace draw overloads and sampling helpers.
  void DolphinDrawFromWorkspace(const SSkinningWorkspace& workspace, uint drawFlags,
                                const CModelFlags& flags) const;
  void DolphinDrawFromWorkspace(const SSkinningWorkspace& workspace, uint drawFlags,
                                const CModelFlags& flags, u64 mask) const;
  CVector3f GetSkinnedPosition(const SSkinningWorkspace& workspace, int vertex) const;
  CVector3f GetSkinnedNormal(const SSkinningWorkspace& workspace, int vertex) const;

  static void SetPointGeneratorFunc(void* context, TPointGenFunc callback);

private:
  // Guessed names; these have no original GameCube symbol evidence.
  void LoadMatrixBank(int& currentBank, int bank) const;
  void BuildSkinningMatrices(const CTransform4f* transforms, SSkinningMatrices* matrices,
                             bool uniformScale) const;
  template < class TDraw, class TFlatDraw >
  void DolphinDrawInternal(const SSkinningWorkspace& workspace, uint drawFlags,
                           const CModelFlags& flags, const TDraw& draw,
                           const TFlatDraw& flat) const;
#endif

private:
  static TPointGenFunc sPointGen;
  static void* sPointGenData;

  TLockedToken< CModel > mModel;
  TLockedToken< CSkinRules > mSkinRules;
  TLockedToken< CCharLayoutInfo > mLayoutInfo;
#if RSTL_PRIME1
  mutable rstl::auto_ptr< float > mVertWorkspace;
  mutable rstl::auto_ptr< float > mNormalWorkspace;
  bool mOwned;
  bool mDisableWorkspaces;
#endif
};

#if RSTL_PRIME1
// No Echoes symbols reference this class.
class CSkinnedModelWithAvgNormals {
  CSkinnedModel mSkinnedModel;
  rstl::auto_ptr< float > mAvgNormals;

public:
  CSkinnedModelWithAvgNormals(const CSkinnedModel& model);
  const CSkinnedModel& GetSkinnedModel() const { return mSkinnedModel; }
  float* GetAvgNormals() const { return mAvgNormals.get(); }
};
#else
CHECK_SIZEOF(CSkinnedModel, 0x24)
#endif

#endif // _CSKINNEDMODEL
