#ifndef _CSKINNEDMODEL
#define _CSKINNEDMODEL
#include "rstl/RstlVersions.h"

#include "types.h"

#if RSTL_PRIME1
#include "Kyoto/Animation/CSkinRules.hpp"
#endif

#include "Kyoto/TToken.hpp"
#if RSTL_PRIME1
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
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
typedef void (*TDrawFunc)(const float*, const float*, const void*);
#else
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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  enum EDataOwnership {
    kDO_Unowned,
    kDO_Owned,
  };
#else
  typedef void (*TPointGenFunc)(const CSkinnedModel&, const SSkinningWorkspace&, void*);
  typedef void (*TDrawFunc)(const SSkinningWorkspace&, void*);
  // Guessed flag names.
  enum EDrawFlags { kDF_Unsorted = 2, kDF_Sorted = 4, kDF_Flat = 8 };
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CSkinnedModel(const CSkinnedModel&);
  CSkinnedModel(const TLockedToken< CModel >&, const TLockedToken< CSkinRules >&,
                const TLockedToken< CCharLayoutInfo >&, EDataOwnership);
  virtual ~CSkinnedModel();
#else
  CSkinnedModel(const TLockedToken< CModel >& model, const TLockedToken< CSkinRules >& skinRules,
                const TLockedToken< CCharLayoutInfo >& layoutInfo);
  ~CSkinnedModel();

  static void ClearPointGeneratorFunc();
#endif

  TLockedToken< CModel >& Model() { return mModel; }
  const TLockedToken< CModel >& GetModel() const { return mModel; }
  const TLockedToken< CCharLayoutInfo >& GetLayoutInfo() const { return mLayoutInfo; }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  void SetLayoutInfo(const TLockedToken< CCharLayoutInfo >& layout) { mLayoutInfo = layout; }
#else
  void SetLayoutInfo(const TToken< CCharLayoutInfo >& layout) { mLayoutInfo = layout; }
  const TLockedToken< CSkinRules >& GetSkinRules() const { return mSkinRules; }
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
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
#else
  CSkinnedModelState MakeDefaultStorage() const;
  CSkinnedModelState MakeStorage(bool transient) const; // Guessed name.
  void StoreCalculation(CSkinnedModelState& state, const CPoseAsTransforms_Linear* pose) const;
  void Draw(const CPoseAsTransforms_Linear* pose, const CModelFlags& flags) const;
  void Draw(const CPoseAsTransforms_Linear* pose, TDrawFunc callback, void* context) const;
  void DolphinDrawWithFlags(const CPoseAsTransforms_Linear* pose, uint drawFlags,
                            const CModelFlags& flags) const;
  void DrawFromState(const CSkinnedModelState& state, const CModelFlags& flags) const;
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  float* AllocateNewWorkspace(float** nrmOut);
#else
  // Guessed names for the workspace draw overloads and sampling helpers.
  void DolphinDrawFromWorkspace(const SSkinningWorkspace& workspace, uint drawFlags,
                                const CModelFlags& flags) const;
  void DolphinDrawFromWorkspace(const SSkinningWorkspace& workspace, uint drawFlags,
                                const CModelFlags& flags, u64 mask) const;
  CVector3f GetSkinnedPosition(const SSkinningWorkspace& workspace, int vertex) const;
  CVector3f GetSkinnedNormal(const SSkinningWorkspace& workspace, int vertex) const;
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static void SetPointGeneratorFunc(void*,
                                    void (*)(void*, const CVector3f*, const CVector3f*, int));
  static void ClearPointGeneratorFunc();
#if RSTL_VERSION >= RSTL_GM8P_00 && RSTL_VERSION != RSTL_GM8J_00
  static void SetSkinningBuffer(void* buffer, int size);
#endif
  static void AddDummySkinnedModelRef();
  static void RemoveDummySkinnedModelRef();
#else
  static void SetPointGeneratorFunc(void* context, TPointGenFunc callback);
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  void Construct();
  void AllocateStorage();
  static void TickAllocations();
  static void* EnsureAllocation(int size);
#else
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

#if RSTL_PRIME1
  typedef void (*TPointGenFunc)(void*, const CVector3f*, const CVector3f*, int);
#endif

  static TPointGenFunc sPointGen;
  static void* sPointGenData;

#if RSTL_PRIME1
  static void Draw(const TDrawFunc func, const float* positions, const float* normals, void* data) {
    func(positions, normals, data);
  }

private:
#endif

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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
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
