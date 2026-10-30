#ifndef _CANIMTREETWEENBASE
#define _CANIMTREETWEENBASE
#include "rstl/RstlVersions.h"

#include "Kyoto/Animation/CAnimTreeDoubleChild.hpp"

class CAnimTreeTweenBase : public CAnimTreeDoubleChild {
#if RSTL_PRIME1
  static s32 sAdvancementDepth;
#endif

public:
  static const int kBlendRoot_Offset;
  static const int kBlendRoot_Rotation;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CAnimTreeTweenBase(const bool, const rstl::ncrc_ptr< CAnimTreeNode >& a,
                     const rstl::ncrc_ptr< CAnimTreeNode >& b, int, const rstl::string& name);
#else
  CAnimTreeTweenBase(const bool characterSpaceBlend, const rstl::ncrc_ptr< CAnimTreeNode >& a,
                     const rstl::ncrc_ptr< CAnimTreeNode >& b, int flags, const rstl::string& name);

  // IAnimReader
#endif

  ~CAnimTreeTweenBase() override;
#if !RSTL_PRIME1
  bool VHasOffset(const CSegId& seg) const override;
  CVector3f VGetOffset(const CSegId& seg) const override;
  CQuaternion VGetRotation(const CSegId& seg) const override;
  void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut) const override;
  void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut,
                           const CCharAnimTime& time) const override;
  void VGetJointData_Linear(const CCharLayoutInfo& layout, CJointData_LinearStorage& data,
                            const CCharAnimTime& time) const override;
  void VGetJointData_Linear(const CCharLayoutInfo& layout,
                            CJointData_LinearStorage& data) const override;
  rstl::optional_object< rstl::ownership_transfer< IAnimReader > > VSimplified() override;
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  virtual void SetBlendingWeight(float w) = 0;
#else
  // CAnimTreeNode
  void VGetWeightedReaders(
      float weight,
      rstl::reserved_vector< rstl::pair< float, IAnimReader* >, 16 >& out) const override;

  // CAnimTreeDoubleChild
  float VGetRightChildWeight() const override;

  virtual void SetBlendingWeight(float weight) = 0;
  virtual rstl::optional_object< rstl::ownership_transfer< IAnimReader > > VReverseSimplified();
  virtual float VGetBlendingWeight() const = 0;
#endif

  float GetBlendingWeight() const;
  bool CharacterSpaceBlend() const { return mCharacterSpaceBlend != 0; }
  int GetBlendRoot() const { return mFlags; }
#if RSTL_PRIME1
  void VGetWeightedReaders(
      float w, rstl::reserved_vector< rstl::pair< float, IAnimReader* >, 16 >& out) const override;
  float VGetRightChildWeight() const override;

  void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut) const override;
  void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut,
                           const CCharAnimTime& time) const override;
  bool VHasOffset(const CSegId& seg) const override;
  CVector3f VGetOffset(const CSegId& seg) const override;
  CQuaternion VGetRotation(const CSegId& seg) const override;

  rstl::optional_object< rstl::ownership_transfer< IAnimReader > > VSimplified() override;
  virtual rstl::optional_object< rstl::ownership_transfer< IAnimReader > > VReverseSimplified();
  virtual float VGetBlendingWeight() const = 0;
#endif

  static bool ShouldCullTree();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static void IncAdvancementDepth() { sAdvancementDepth++; }
  static void DecAdvancementDepth() { sAdvancementDepth--; }
#else
  static void IncAdvancementDepth() { ++sAdvancementDepth; }
  static void DecAdvancementDepth() { --sAdvancementDepth; }
#endif

protected:
  int mFlags;
  s32 mCharacterSpaceBlend : 1;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if NONMATCHING
  u32 mCullSelector : 2;
#else
  s32 mCullSelector : 2;
#endif
#else
  s32 mCullSelector : 2;

private:
  // Guessed names: common implementations of the timed and current-pose virtuals.
  void BlendSegData(const CCharLayoutInfo& layout, CJointData_LinearStorage& data,
                    rstl::optional_object< CCharAnimTime > time) const;
  void BlendSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut,
                            rstl::optional_object< CCharAnimTime > time) const;
  static s32 sAdvancementDepth;
#endif

};
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CHECK_SIZEOF(CAnimTreeTweenBase, 0x24)
#else
CHECK_CHILD_SIZEOF(CAnimTreeTweenBase, CAnimTreeDoubleChild, 0x8)
#endif

#endif // _CANIMTREETWEENBASE
