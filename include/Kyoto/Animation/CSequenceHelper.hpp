#ifndef _CSEQUENCEHELPER
#define _CSEQUENCEHELPER
#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CAnimSysContext.hpp"
#include "Kyoto/Animation/CSequenceFundamentals.hpp"

class IMetaAnim;
class CAnimTreeNode;
class CSequenceHelper {
public:
  CSequenceHelper(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                  const rstl::ncrc_ptr< CAnimTreeNode >& b, const CAnimSysContext& context);
  CSequenceHelper(const rstl::vector< rstl::rc_ptr< IMetaAnim > >& anims,
                  const CAnimSysContext& context);

  CSequenceFundamentals ComputeSequenceFundamentals() const;

private:
  CAnimSysContext mContext;
  rstl::vector< rstl::ncrc_ptr< CAnimTreeNode > > mNodes;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  rstl::vector< bool > x20_;
#else
  rstl::vector< bool > x28_;
#endif
};
CHECK_SIZEOF(CSequenceHelper, RSTL_PRIME1 ? 0x30 : 0x38)

#endif // _CSEQUENCEHELPER
