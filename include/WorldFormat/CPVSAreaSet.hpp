#ifndef _CPVSAREASET
#define _CPVSAREASET

#include "rstl/RstlVersions.h"
#include "rstl/static_assert.hpp"

#include "Kyoto/PVS/CPVSVisOctree.hpp"

class CInputStream;
class CPVSVisSet;

class CPVSAreaSet {
public:
  CPVSAreaSet(int numFeatures, int numLights, int num2ndLights, int numActors, int leafSize,
              int lightIndexCount, const char* entityIds, const char* lightLeaves,
              const char* octreeData);

  const CPVSVisOctree& GetOctree() const { return mOctree; }
  static rstl::auto_ptr< CPVSAreaSet > MakeAreaSet(const char* data, int length);
  CPVSVisSet GetLightSet(int lightIndex) const;
  int GetEntityIdByIndex(uint index) const;
  CPVSVisOctree& GetVisOctree() const;

  int GetNum2ndLights() const { return mNum2ndLights; }
  int GetNumLights() const { return mNumLights; }
  int GetNumFeatures() const { return mNumFeatures; }
  int GetNumActors() const { return mNumActors; }
  int GetLightIndexCount() const { return mLightIndexCount; } // Guessed name
  int GetNumLightSets() const { return mLightIndexCount; }
  bool Has2ndLayerLights() const { return mNum2ndLights != 0; }

private:
  int mNumFeatures;
  int mNumLights;
  int mNum2ndLights;
  int mNumActors;
  int mLeafSize;
  int mLightIndexCount;
  const char* mEntityIds;
  const char* mLightLeaves;
  mutable CPVSVisOctree mOctree;
};
CHECK_SIZEOF(CPVSAreaSet, 0x64)

#endif // _CPVSAREASET
