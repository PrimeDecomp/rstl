#ifndef _CANIMTREENODE
#define _CANIMTREENODE
#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"
#endif

#if RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#else
#include "Kyoto/Animation/CCharAnimMemoryMetrics.hpp"

#endif
#include "Kyoto/Animation/IAnimReader.hpp"
#include "rstl/pair.hpp"
#include "rstl/rc_ptr.hpp"

#include "rstl/reserved_vector.hpp"
#if !RSTL_CORRUPTION
#include "rstl/string.hpp"
#endif

class CAnimTreeNode : public IAnimReader {
public:
  explicit CAnimTreeNode(const rstl::string& name);
#if RSTL_PRIME1
  ~CAnimTreeNode() override {
    CCharAnimMemoryMetrics::SubtractFromTotalSize(mName.size(), CCharAnimMemoryMetrics::kASS_Two);
  }
#else
  ~CAnimTreeNode() override;
#endif
  bool IsCAnimTreeNode() const override;

  virtual uint Depth() const = 0;

  virtual CAnimTreeEffectiveContribution VGetContributionOfHighestInfluence() const = 0;
#if RSTL_CORRUPTION
  virtual uint VGetNumDirectChildren() const = 0; // Guessed name
#endif
  virtual uint VGetNumChildren() const = 0;
  virtual rstl::rc_ptr< CAnimTreeNode > VGetBestUnblendedChild() const = 0;
  virtual void VGetWeightedReaders(
      float weight, rstl::reserved_vector< rstl::pair< float, IAnimReader* >, 16 >& out) const = 0;
#if RSTL_CORRUPTION
  virtual rstl::rc_ptr< CAnimTreeNode > VGetDirectChild(uint index) const = 0; // Guessed name
#endif

  CAnimTreeEffectiveContribution GetContributionOfHighestInfluence() const {
    return VGetContributionOfHighestInfluence();
  }

  rstl::rc_ptr< CAnimTreeNode > GetBestUnblendedChild() const { return VGetBestUnblendedChild(); }

  const rstl::string& GetPrimitiveName() const { return mName; }

protected:
  rstl::string mName;
};
#if RSTL_CORRUPTION
CHECK_SIZEOF(CAnimTreeNode, 0x14)
#else
CHECK_CHILD_SIZEOF(CAnimTreeNode, IAnimReader, (RSTL_PRIME1 ? RSTL_VERSION >= RSTL_R3IJ : RSTL_VERSION >= RSTL_R32J) ? 0xc : 0x10)
#endif

rstl::ncrc_ptr< CAnimTreeNode > Cast(const rstl::ownership_transfer< IAnimReader >& ptr);

#endif // _CANIMTREENODE
