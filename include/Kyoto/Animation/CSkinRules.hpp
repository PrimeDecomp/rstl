#ifndef _CSKINRULES
#define _CSKINRULES
#include "rstl/RstlVersions.h"
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"

#if RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#endif
#include "Kyoto/Animation/CVirtualBone.hpp"
#include "Kyoto/CFactoryFnReturn.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"

#if RSTL_CORRUPTION


class CFactoryFnReturn;
#endif
class CInputStream;
#if RSTL_CORRUPTION
class CVParamTransfer;
#endif
struct SObjectTag;

#if RSTL_CORRUPTION
namespace SkinRules {
static const int kMaximumBones = 256;
} // namespace SkinRules
#else
class CCharLayoutInfo;
class CModel;
class CPoseAsTransforms;
class CFactoryFnReturn;
class CVParamTransfer;
#endif

class CSkinRules {
public:
  CSkinRules(CInputStream& in);
  ~CSkinRules();

#if RSTL_CORRUPTION
  void BuildAccumulatedTransforms(const CPoseAsTransforms_Linear& pose,
                                  CTransform4f* out, int count, const uchar* boneIndices) const;
  void LoadMatrixBank(int bank, const uchar* remap) const; // Guessed name.
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  void InitLockedCacheState(const CModel& model);
  static void StartNextTransaction();
#else
  void BuildAccumulatedTransforms(const CPoseAsTransforms_Linear& pose,
                                  const CCharLayoutInfo& layoutInfo, CTransform4f* out) const;
  void LoadMatrixBank(int bank) const; // Guessed name.
#endif

  void BuildAccumulatedTransforms(const CPoseAsTransforms& pose,
                                  const CCharLayoutInfo& layoutInfo) const;
  void BuildPoints(volatile void* pipe) const;
  void BuildNormals(volatile void* pipe) const;
  void BuildNormalsFrom(const CVector3f* averageNormals, CVector3f* out) const;
#endif

  int GetNumPoints() const { return mVertexCount; }
#if RSTL_CORRUPTION

  int GetNumVirtualBones() const { return mBones.size(); }

  const rstl::vector< CVirtualBone >& GetVirtualBones() const { return mBones; }

  const uchar* GetVertexToBoneMap() const { return mVertexToBone.get(); }

  // Guessed names
  const rstl::vector< uchar >& GetUnsortedBoneIndices() const { return mUnsortedBoneIndices; }

  const rstl::vector< uchar >& GetSortedBoneIndices() const { return mSortedBoneIndices; }
#else
#if RSTL_PRIME1
  int GetNumNormals() const { return mNormalCount; }
#endif
  int GetNumVirtualBones() const { return mVirtualBones.size(); }
  const rstl::vector< CVirtualBone >& GetVirtualBones() const { return mVirtualBones; }
#if RSTL_PRIME1
  static int ProcessingPoints(int count, ushort** buf);
  static int ProcessingNormals(int count, ushort** buf);
#else
  const uchar* GetVertexToBoneMap() const { return mVertexToBone.get(); }
#endif
#endif

private:
#if RSTL_CORRUPTION
  static uint ReadHeader(CInputStream& in); // Guessed name

  uint mVersion; // Guessed name
  rstl::vector< CVirtualBone > mBones;
  rstl::vector< short > mMatrixIndices;
  rstl::vector< uchar > mUnsortedBoneIndices; // Guessed name
  rstl::vector< uchar > mSortedBoneIndices;   // Guessed name
  int mVertexCount;
  rstl::single_ptr< uchar > mVertexToBone;
};
CHECK_SIZEOF(CSkinRules, 0x4c)
#else
  rstl::vector< CVirtualBone > mVirtualBones;
#if !RSTL_PRIME1
  rstl::vector< short > mMatrixIndices;
#endif
  int mVertexCount;
#if RSTL_PRIME1
  int mNormalCount;
#else
  rstl::single_ptr< uchar > mVertexToBone;
#endif
};
#if !RSTL_PRIME1
CHECK_SIZEOF(CSkinRules, 0x28)

#endif
RSTL_PRIME1_CONST CFactoryFnReturn FSkinRulesFactory(const SObjectTag& tag, CInputStream& in,
                                                     const CVParamTransfer& params);
#endif

#if RSTL_CORRUPTION
CFactoryFnReturn FSkinRulesFactory(const SObjectTag& tag, const rstl::auto_ptr< uchar >& buffer,
                                   int size, const CVParamTransfer& params);


#endif
#endif // _CSKINRULES
