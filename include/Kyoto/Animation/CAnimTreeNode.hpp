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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      float w, rstl::reserved_vector< rstl::pair< float, IAnimReader* >, 16 >& out) const = 0;
#else
      float weight, rstl::reserved_vector< rstl::pair< float, IAnimReader* >, 16 >& out) const = 0;
#endif

  CAnimTreeEffectiveContribution GetContributionOfHighestInfluence() const {
    return VGetContributionOfHighestInfluence();
  }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  rstl::rc_ptr< CAnimTreeNode > GetBestUnblendedChild() const {
    return VGetBestUnblendedChild();
  }
#else
  rstl::rc_ptr< CAnimTreeNode > GetBestUnblendedChild() const { return VGetBestUnblendedChild(); }
#endif

  const rstl::string& GetPrimitiveName() const { return mName; }
#if RSTL_PRIME1
  bool IsCAnimTreeNode() const override;
#endif

protected:
  rstl::string mName;
};
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_R3IJ
CHECK_SIZEOF(CAnimTreeNode, 0x10)
#else
CHECK_SIZEOF(CAnimTreeNode, 0x14)
#endif
#else
CHECK_CHILD_SIZEOF(CAnimTreeNode, IAnimReader, RSTL_VERSION >= RSTL_R32J ? 0xc : 0x10)
#endif

rstl::ncrc_ptr< CAnimTreeNode > Cast(const rstl::ownership_transfer< IAnimReader >& ptr);

#endif // _CANIMTREENODE
