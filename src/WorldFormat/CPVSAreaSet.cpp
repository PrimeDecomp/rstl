#include "WorldFormat/CPVSAreaSet.hpp"

#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/PVS/CPVSVisSet.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"
#include "rstl/auto_ptr.hpp"

#if defined(TARGET_PC)
#include "Metaforce/Endian.hpp"
#endif

CPVSAreaSet::CPVSAreaSet(int numFeatures, int numLights, int num2ndLights, int numActors,
                         int leafSize, int lightIndexCount, const char* entityIds,
                         const char* lightLeaves, const char* octreeData)
: mNumFeatures(numFeatures)
, mNumLights(numLights)
, mNum2ndLights(num2ndLights)
, mNumActors(numActors)
, mLeafSize(leafSize)
, mLightIndexCount(lightIndexCount)
, mEntityIds(entityIds)
, mLightLeaves(lightLeaves)
, mOctree(CPVSVisOctree::MakePVSVisOctree(octreeData, 68)) {}

rstl::auto_ptr< CPVSAreaSet > CPVSAreaSet::MakeAreaSet(const char* data, int length) {
  CMemoryInStream in(data, length);
#if !RSTL_PRIME1
  const int numFeatures = in.ReadInt32();
  const int numLights = in.ReadInt32();
  const int num2ndLights = in.ReadInt32();
  const int numActors = in.ReadInt32();
  const int leafSize = in.ReadInt32();
  const int lightIndexCount = in.ReadInt32();
#else
  const int numFeatures = in.ReadLong();
  const int numLights = in.ReadLong();
  const int num2ndLights = in.ReadLong();
  const int numActors = in.Get< uint >();
  const int leafSize = in.Get< uint >();
  const int lightIndexCount = in.Get< uint >();
#endif

  const char* const areaData = data + in.GetReadPosition();
  const char* const lightLeaves = areaData + numActors * 4;
  const char* const octreeData = lightLeaves + lightIndexCount * leafSize;
  return rstl::auto_ptr< CPVSAreaSet >(rs_new CPVSAreaSet(numFeatures, numLights, num2ndLights,
                                                          numActors, leafSize, lightIndexCount,
                                                          areaData, lightLeaves, octreeData));
}

CPVSVisOctree& CPVSAreaSet::GetVisOctree() const { return mOctree; }

CPVSVisSet CPVSAreaSet::GetLightSet(int lightIndex) const {
#if !RSTL_PRIME1
  if (lightIndex >= mLightIndexCount) {
    return CPVSVisSet(kVSS_OutOfBounds);
  }
#endif

  rstl::auto_ptr< const char > leaf(mLightLeaves + mLeafSize * lightIndex);
  leaf.release();
#if RSTL_PRIME1
  return CPVSVisSet(mOctree.GetNumObjects(), mOctree.GetNumLights(),
                    rstl::auto_ptr< const char >(leaf));
#else
  return CPVSVisSet(mOctree.GetNumObjects(), mOctree.GetNumLights(), leaf);
#endif
}

int CPVSAreaSet::GetEntityIdByIndex(uint index) const {
#if defined(TARGET_PC)
  return read_bits< int >(mEntityIds + index * 4);
#else
  return CBasics::SwapBytes(reinterpret_cast< const int* >(mEntityIds)[index]);
#endif
}
