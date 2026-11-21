#ifndef _IMETATRANS
#define _IMETATRANS
#include "rstl/RstlVersions.h"

#if !RSTL_CORRUPTION
#include "rstl/types.h"
#include "rstl/RstlVersions.h"
#endif
#include "Kyoto/Animation/CAnimTreeNode.hpp"
#include "rstl/rc_ptr.hpp"

#if !RSTL_CORRUPTION
class COutputStream;
#endif

class CAnimSysContext;
#if RSTL_CORRUPTION
class COutputStream;
#endif

#if RSTL_CORRUPTION
enum EMetaTransType { kMTT_MetaAnim = 0, kMTT_PhaseTrans = 2, kMTT_Snap = 3, kMTT_Trans = 4 };
#else
enum EMetaTransType { kMTT_MetaAnim, kMTT_Trans, kMTT_PhaseTrans, kMTT_Snap };
#endif

class IMetaTrans {
public:
  virtual ~IMetaTrans() = 0;
  virtual rstl::ncrc_ptr< CAnimTreeNode >
  VGetTransitionTree(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                     const rstl::ncrc_ptr< CAnimTreeNode >& b,
                     const CAnimSysContext& animSys) const = 0;
  virtual EMetaTransType GetType() const = 0;
  virtual void WriteTransData(COutputStream& out) const = 0;

  rstl::ncrc_ptr< CAnimTreeNode > GetTransitionTree(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                                    const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                                    const CAnimSysContext& animSys) const {
    return VGetTransitionTree(a, b, animSys);
  }
};

inline IMetaTrans::~IMetaTrans() {}

#endif // _IMETATRANS
