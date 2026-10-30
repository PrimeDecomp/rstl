#ifndef _CANIMTREENODE
#define _CANIMTREENODE
#include "rstl/RstlVersions.h"

#include "Kyoto/Animation/CCharAnimMemoryMetrics.hpp"

#include "Kyoto/Animation/IAnimReader.hpp"
#include "rstl/pair.hpp"
#include "rstl/rc_ptr.hpp"

#include "rstl/reserved_vector.hpp"
#include "rstl/string.hpp"

class CAnimTreeNode : public IAnimReader {
public:
#if !RSTL_PRIME1
  explicit CAnimTreeNode(const rstl::string& name);

  // IAnimReader
  ~CAnimTreeNode() override;
  bool IsCAnimTreeNode() const override;
#endif

  virtual uint Depth() const = 0;
#if RSTL_PRIME1
  CAnimTreeNode(const rstl::string&);
  ~CAnimTreeNode() override {
    CCharAnimMemoryMetrics::SubtractFromTotalSize(mName.size(), CCharAnimMemoryMetrics::kASS_Two);
  }
#endif

  virtual CAnimTreeEffectiveContribution VGetContributionOfHighestInfluence() const = 0;

  virtual uint VGetNumChildren() const = 0;
  virtual rstl::rc_ptr< CAnimTreeNode > VGetBestUnblendedChild() const = 0;
  virtual void VGetWeightedReaders(
      float weight, rstl::reserved_vector< rstl::pair< float, IAnimReader* >, 16 >& out) const = 0;

  CAnimTreeEffectiveContribution GetContributionOfHighestInfluence() const {
    return VGetContributionOfHighestInfluence();
  }

  rstl::rc_ptr< CAnimTreeNode > GetBestUnblendedChild() const { return VGetBestUnblendedChild(); }

  const rstl::string& GetPrimitiveName() const { return mName; }
#if RSTL_PRIME1
  bool IsCAnimTreeNode() const override;
#endif

protected:
  rstl::string mName;
};
CHECK_CHILD_SIZEOF(CAnimTreeNode, IAnimReader, (RSTL_PRIME1 ? RSTL_VERSION >= RSTL_R3IJ : RSTL_VERSION >= RSTL_R32J) ? 0xc : 0x10)

rstl::ncrc_ptr< CAnimTreeNode > Cast(const rstl::ownership_transfer< IAnimReader >& ptr);

#endif // _CANIMTREENODE
