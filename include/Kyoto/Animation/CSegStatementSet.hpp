#ifndef _CSEGSTATEMENTSET
#define _CSEGSTATEMENTSET
#include "rstl/RstlVersions.h"

#if RSTL_PRIME1
#include "CCharLayoutInfo.hpp"
#endif

#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Animation/CSegStatement.hpp"

class CSegStatementSet {
public:
  virtual ~CSegStatementSet() = 0;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CSegStatementSet(void* ptr);
#else
  explicit CSegStatementSet(void* storage);
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CSegStatement& operator[](const CSegId& id) { return mSegData[id.val()]; }
  const CSegStatement& operator[](const CSegId& id) const { return mSegData[id.val()]; }

  CSegStatement& Statement(const CSegId& id) { return mSegData[id.val()]; }
  const CSegStatement& GetData(const CSegId& id) const { return mSegData[id.val()]; }
  void Set(const CSegId& id, const CQuaternion& rotation) { Statement(id).Set(rotation); }
  void Set(const CSegId& id, const CSegStatement& seg) { Statement(id) = seg; }

  void Set(const CSegId& id, const CVector3f& vec) { mSegData[id.val()].Set(vec); }

  void Add(const CSegIdList& list, const CCharLayoutInfo& info, const CSegStatementSet& set,
           float weight);
#else
  CSegStatement& operator[](const CSegId& seg) { return mSegData[seg.val()]; }
  const CSegStatement& operator[](const CSegId& seg) const { return mSegData[seg.val()]; }
#endif

protected:
  CSegStatement* mSegData;
};
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CHECK_SIZEOF(CSegStatementSet, 0x8)
#else
DECLARE_FULL_SIZE_FOR(CSegStatementSet, 0x8)
CHECK_SIZEOF(CSegStatementSet, CSegStatementSet_FULL_SIZE)
#endif

inline CSegStatementSet::~CSegStatementSet() {}

class CStackSegStatementSet : public CSegStatementSet {
public:
#if !RSTL_PRIME1
  CStackSegStatementSet();

  // CSegStatementSet
#endif

  ~CStackSegStatementSet() override;
#if RSTL_PRIME1
  CStackSegStatementSet();
#endif

};
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CHECK_SIZEOF(CStackSegStatementSet, 0x8)
#else
CHECK_CHILD_SIZEOF(CStackSegStatementSet, CSegStatementSet, 0x0)
#endif

#endif // _CSEGSTATEMENTSET
