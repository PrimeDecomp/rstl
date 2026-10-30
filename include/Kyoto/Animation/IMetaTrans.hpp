#ifndef _IMETATRANS
#define _IMETATRANS
#include "rstl/RstlVersions.h"

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#include "types.h"

class CAnimTreeNode;
#else
#include "Kyoto/Animation/CAnimTreeNode.hpp"
#endif

#include "rstl/rc_ptr.hpp"

class COutputStream;

class CAnimSysContext;

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
enum EMetaTransType {
  kMTT_MetaAnim,
  kMTT_Trans,
  kMTT_PhaseTrans,
  kMTT_Snap,
};
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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  virtual void WriteTransData(COutputStream&) const = 0;
#else
  virtual void WriteTransData(COutputStream& out) const = 0;
#endif

  rstl::ncrc_ptr< CAnimTreeNode > GetTransitionTree(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                                    const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                                    const CAnimSysContext& animSys) const {
    return VGetTransitionTree(a, b, animSys);
  }
};

inline IMetaTrans::~IMetaTrans() {}

#endif // _IMETATRANS
