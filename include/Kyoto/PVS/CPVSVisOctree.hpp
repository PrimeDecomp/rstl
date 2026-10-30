#ifndef _CPVSVISOCTREE
#define _CPVSVISOCTREE
#include "rstl/RstlVersions.h"

#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/PVS/CPVSVisSet.hpp"
#include "rstl/auto_ptr.hpp"

class CPVSVisOctree {
public:
  CPVSVisOctree(const CAABox& bounds, int numObjects, int numLights, const char* data);
  int IterateSearch(uchar a, const CVector3f& pos) const;
  uint GetNumChildren(uchar a) const;
  uint GetNumObjects() const { return mNumObjects; }
  uint GetNumLights() const { return mNumLights; }

  static CPVSVisOctree MakePVSVisOctree(const char* data, int len);

  void SetTestPoint(const CPVSVisOctree& octree, const CVector3f& point);

  const CAABox& GetBounds() const { return mBounds; }
  // const CAABox& GetSearchBounds() const { return mSearchBounds; }
  // void SetSearchBounds(const CAABox& bounds) { mSearchBounds = bounds; }

  CPVSVisSet GetVisSet(const CVector3f& point) const;

#if RSTL_PRIME1
#ifdef ENABLE_DEBUG_TOOLS
  // Backported traversal interface; callback receives the leaf depth and ordinal.
  typedef void (*TLeafCallback)(const CPVSVisOctree&, const CAABox&, const CPVSVisSet&, int, int,
                               void*);
  void TraverseLeaves(TLeafCallback callback, void* context) const;
#endif
#endif

private:
  CAABox mBounds;
  uint mNumObjects;
  uint mNumLights;
  rstl::auto_ptr< const char > mOctreeData;
  uint _28;
  mutable CVector3f mMin;
  mutable CVector3f mMax;
};

#endif // _CPVSVISOCTREE
