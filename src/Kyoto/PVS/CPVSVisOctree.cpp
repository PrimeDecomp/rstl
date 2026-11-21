#include "rstl/RstlVersions.h"
#include "rstl/RstlVersions.h"
#include "Kyoto/PVS/CPVSVisOctree.hpp"

#if RSTL_CORRUPTION
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "string.h"


#else
#if !RSTL_PRIME1
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"
#if RSTL_VERSION >= RSTL_R32J
#include "string.h"

#include "CPVSVisOctree.inc"
#endif
#endif
#endif

int CPVSVisOctree::IterateSearch(uchar octant, const CVector3f& pos) const {
  if (!(octant & 7)) {
    return -1;
  }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CVector3f halfExtent = (mMax - mMin) * 0.5f;
  CVector3f center = halfExtent + mMin;
#else
  const CVector3f halfExtent = (mMax - mMin) * 0.5f;
  const CVector3f& center = halfExtent + mMin;
#endif

  float minX, maxX;
  float minY, maxY;
  float minZ, maxZ;
  bool highX, highY, highZ;
#if RSTL_CORRUPTION
  const float centerX = center[0];
  if (pos[0] > centerX) {
    minX = centerX;
#else
  if (pos[0] > center[0]) {
    minX = center[0];
#endif
    maxX = mMax[0];
    highX = true;
  } else {
#if RSTL_CORRUPTION
    maxX = centerX;
#else
    maxX = center[0];
#endif
    minX = mMin[0];
    highX = false;
  }
  if (pos[1] > center[1]) {
    minY = center[1];
    maxY = mMax[1];
    highY = true;
  } else {
    maxY = center[1];
    minY = mMin[1];
    highY = false;
  }
  if (pos[2] > center[2]) {
    minZ = center[2];
    maxZ = mMax[2];
    highZ = true;
  } else {
    maxZ = center[2];
    minZ = mMin[2];
    highZ = false;
  }

  int xCount = 1;
  int yCount = 1;
  if (octant & 1)
    xCount = 2;
  if (octant & 2)
    yCount = 2;
  mMin = CVector3f(octant & 1 ? minX : mMin[0], octant & 2 ? minY : mMin[1],
                   octant & 4 ? minZ : mMin[2]);
  mMax = CVector3f(octant & 1 ? maxX : mMax[0], octant & 2 ? maxY : mMax[1],
                   octant & 4 ? maxZ : mMax[2]);
  return highX * bool(octant & 1) + highY * xCount * bool(octant & 2) +
         highZ * xCount * yCount * bool(octant & 4);
}

uint CPVSVisOctree::GetNumChildren(uchar octant) const {
  static const uint childCount[8] = {0, 2, 2, 4, 2, 4, 4, 8};
  return childCount[octant & 7];
}
#if RSTL_CORRUPTION

void CPVSVisOctree::TraverseLeaves(TLeafCallback callback, void* userData) {
  mMin = mBounds.GetMinPoint();
  mMax = mBounds.GetMaxPoint();
  mCurrentNodeOffset = 0;
  TraverseLeavesRecursive(callback, 0, 0, userData);
}

CAABox CPVSVisOctree_GetChildBounds(uchar octant, int child, CAABox bounds) {
  const CVector3f center = bounds.GetCenterPoint();
  const CVector3f min = bounds.GetMinPoint();
  const CVector3f max = bounds.GetMaxPoint();
  const float* centerCoords = center.GetArrayPtr();
  const float* minCoords = min.GetArrayPtr();
  const float* maxCoords = max.GetArrayPtr();
  float minX, maxX;
  float minY, maxY;
  float minZ, maxZ;
  const bool highX = (octant & 1) && (child & 1);
  if (octant & 1) {
    child >>= 1;
  }
  if (highX) {
    minX = centerCoords[0];
    maxX = maxCoords[0];
  } else {
    minX = minCoords[0];
    maxX = centerCoords[0];
  }
  const bool highY = (octant & 2) && (child & 1);
  if (octant & 2) {
    child >>= 1;
  }
  if (highY) {
    minY = centerCoords[1];
    maxY = maxCoords[1];
  } else {
    minY = minCoords[1];
    maxY = centerCoords[1];
  }
  const bool highZ = (octant & 4) && (child & 1);
  if (highZ) {
    minZ = centerCoords[2];
    maxZ = maxCoords[2];
  } else {
    minZ = minCoords[2];
    maxZ = centerCoords[2];
  }
  const CVector3f childMin(octant & 1 ? minX : minCoords[0], octant & 2 ? minY : minCoords[1],
                           octant & 4 ? minZ : minCoords[2]);
  const CVector3f childMax(octant & 1 ? maxX : maxCoords[0], octant & 2 ? maxY : maxCoords[1],
                           octant & 4 ? maxZ : maxCoords[2]);
  return CAABox(childMin, childMax);
}

void CPVSVisOctree::TraverseLeavesRecursive(TLeafCallback callback, int depth, int child,
                                          void* userData) {
  const CAABox bounds(mMin, mMax);
  const uint nodeOffset = mCurrentNodeOffset;
  const char* const data = mOctreeData.get();
  uchar nodeData = CCast::ToUint8(data[nodeOffset]);
  const int numChildren = GetNumChildren(nodeData);
  if (numChildren > 0) {
    for (int i = 0; i < numChildren; ++i) {
      mCurrentNodeOffset = nodeOffset + 1;
      const CAABox childBounds = CPVSVisOctree_GetChildBounds(nodeData, i, bounds);
      mMin = childBounds.GetMinPoint();
      mMax = childBounds.GetMaxPoint();
      if (i != 0) {
        if ((nodeData & 0x60) == 0) {
#ifdef __MWERKS__
          mCurrentNodeOffset +=
              CBasics::SwapBytes(reinterpret_cast< const ushort* >(data + mCurrentNodeOffset)[i - 1]);
#else
          ushort offset;
          memcpy(&offset, data + mCurrentNodeOffset + (i - 1) * sizeof(offset), sizeof(offset));
          mCurrentNodeOffset += CBasics::SwapBytes(offset);
#endif
        } else if (nodeData & 0x20) {
          mCurrentNodeOffset += CCast::ToUint8(data[mCurrentNodeOffset + i - 1]);
        } else {
          const uchar* offset =
              reinterpret_cast< const uchar* >(data) + mCurrentNodeOffset + (i - 1) * 3;
          mCurrentNodeOffset += (offset[0] << 16) + (offset[1] << 8) + offset[2];
        }
      }
      if ((nodeData & 0x60) == 0) {
        mCurrentNodeOffset += (GetNumChildren(nodeData) - 1) * 2;
      } else if (nodeData & 0x20) {
        mCurrentNodeOffset += GetNumChildren(nodeData) - 1;
      } else {
        mCurrentNodeOffset += (GetNumChildren(nodeData) - 1) * 3;
      }
      TraverseLeavesRecursive(callback, depth + 1, i, userData);
    }
  } else {
    CPVSVisSet vis(kVSS_OutOfBounds);
    switch (nodeData & 0x18) {
    case 24: {
      rstl::auto_ptr< const char > leaf(data + (nodeOffset + 1));
      leaf.release();
      vis = CPVSVisSet(GetNumObjects(), GetNumLights(), rstl::auto_ptr< const char >(leaf));
      break;
    }
    case 8:
      vis = CPVSVisSet(kVSS_OutOfBounds);
      break;
    case 16:
      vis = CPVSVisSet(kVSS_EndOfTree);
      break;
    }
    callback(*this, bounds, vis, depth, child, userData);
  }
}
#endif
