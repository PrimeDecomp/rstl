#ifndef _IMETAANIM
#define _IMETAANIM
#include "rstl/RstlVersions.h"

#include "Kyoto/Animation/CAnimTreeNode.hpp"
#include "Kyoto/Animation/CCharAnimTime.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/set.hpp"
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#include "rstl/string.hpp"
#endif

class COutputStream;

enum EMetaAnimType { kMAT_Play, kMAT_Blend, kMAT_PhaseBlend, kMAT_Random, kMAT_Sequence };

class CPrimitive;
class CCharAnimTime;
class IAnimReader;
class CAnimSysContext;

class CPreAdvanceIndicator {
#if !RSTL_PRIME1
public:
  explicit CPreAdvanceIndicator(const CCharAnimTime& time) : mIsTime(true), mTime(time) {}
  bool IsTime() const { return mIsTime; }
  bool IsString() const { return !mIsTime; }
  const CCharAnimTime& GetTime() const { return mTime; }
  uint GetNameHash() const { return mNameHash; } // Guessed name.

private:
#endif

  bool mIsTime;
  CCharAnimTime mTime;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  char mString[50];

public:
  explicit CPreAdvanceIndicator(const CCharAnimTime& time) : mIsTime(true), mTime(time) {}
  explicit CPreAdvanceIndicator(const char* string);
  bool IsTime() const;
  const CCharAnimTime& GetTime() const;
  bool IsString() const;
  const char* GetString() const;
#else
  uint mNameHash;
#endif
};
CHECK_SIZEOF(CPreAdvanceIndicator, RSTL_PRIME1 ? 0x40 : 0x10)
#if !RSTL_PRIME1

namespace rstl {
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(CPreAdvanceIndicator)
} // namespace rstl
#endif

class CMetaAnimTreeBuildOrders {
public:
  static CMetaAnimTreeBuildOrders NoSpecialOrders();
  static CMetaAnimTreeBuildOrders PreAdvanceForAll(const CPreAdvanceIndicator& ind);

  rstl::optional_object< CPreAdvanceIndicator > mRecursiveAdvance;
  rstl::optional_object< CPreAdvanceIndicator > mSingleAdvance;
};
#if !RSTL_PRIME1
CHECK_SIZEOF(CMetaAnimTreeBuildOrders, 0x28)
#endif

class IMetaAnim {
public:
  virtual ~IMetaAnim() = 0;

  virtual rstl::ncrc_ptr< CAnimTreeNode >
  GetAnimationTree(const CAnimSysContext& animSys, const CMetaAnimTreeBuildOrders& orders) const;
  virtual void GetUniquePrimitives(rstl::set< CPrimitive >& primsOut) const = 0;
  virtual EMetaAnimType GetType() const = 0;
  virtual void WriteAnimData(COutputStream& out) const = 0;
  virtual rstl::ncrc_ptr< CAnimTreeNode >
  VGetAnimationTree(const CAnimSysContext& animSys,
                    const CMetaAnimTreeBuildOrders& orders) const = 0;

  void PutTo(COutputStream& out) const;

  static void AdvanceAnim(IAnimReader& anim, const CCharAnimTime& dt);
  static CCharAnimTime GetTime(const CPreAdvanceIndicator& ind, const IAnimReader& anim);
};

inline IMetaAnim::~IMetaAnim() {}

#endif // _IMETAANIM
