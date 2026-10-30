#ifndef _CSKINRULES
#define _CSKINRULES
#include "rstl/RstlVersions.h"

#include "Kyoto/Animation/CVirtualBone.hpp"
#include "Kyoto/CFactoryFnReturn.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"

class CInputStream;
struct SObjectTag;

class CCharLayoutInfo;
class CModel;
class CPoseAsTransforms;
class CFactoryFnReturn;
class CVParamTransfer;

class CSkinRules {
public:
  CSkinRules(CInputStream& in);
  ~CSkinRules();

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  void InitLockedCacheState(const CModel& model);
  static void StartNextTransaction();
#else
  void BuildAccumulatedTransforms(const CPoseAsTransforms_Linear& pose,
                                  const CCharLayoutInfo& layoutInfo, CTransform4f* out) const;
  void LoadMatrixBank(int bank) const; // Guessed name.
#endif

#if RSTL_PRIME1
  void BuildAccumulatedTransforms(const CPoseAsTransforms& pose,
                                  const CCharLayoutInfo& layoutInfo) const;
  void BuildPoints(volatile void* pipe) const;
  void BuildNormals(volatile void* pipe) const;
  void BuildNormalsFrom(const CVector3f* averageNormals, CVector3f* out) const;
#endif

  int GetNumPoints() const { return mVertexCount; }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  int GetNumNormals() const { return mNormalCount; }
  int GetNumVirtualBones() const { return mVirtualBones.size(); }
  const rstl::vector< CVirtualBone >& GetVirtualBones() const { return mVirtualBones; }
  static int ProcessingPoints(int count, ushort** buf);
  static int ProcessingNormals(int count, ushort** buf);

private:
  rstl::vector< CVirtualBone > mVirtualBones;
  int mVertexCount;
  int mNormalCount;
#else
  int GetNumVirtualBones() const { return mBones.size(); }
  const rstl::vector< CVirtualBone >& GetVirtualBones() const { return mBones; }
  const uchar* GetVertexToBoneMap() const { return mVertexToBone.get(); }

private:
  rstl::vector< CVirtualBone > mBones;
  rstl::vector< short > mMatrixIndices;
  int mVertexCount;
  rstl::single_ptr< uchar > mVertexToBone;
#endif
};
#if !RSTL_PRIME1
CHECK_SIZEOF(CSkinRules, 0x28)

#endif
RSTL_PRIME1_CONST CFactoryFnReturn FSkinRulesFactory(const SObjectTag& tag, CInputStream& in,
                                                     const CVParamTransfer& params);

#endif // _CSKINRULES
