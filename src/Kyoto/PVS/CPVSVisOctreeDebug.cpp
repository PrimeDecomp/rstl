#include "Kyoto/PVS/CPVSVisOctree.hpp"

#ifdef ENABLE_DEBUG_TOOLS

namespace {
void TraversePVSNode(const CPVSVisOctree& tree, const uchar* node, const CAABox& bounds,
                     CPVSVisOctree::TLeafCallback callback, void* context, int depth,
                     int& ordinal) {
  const uchar flags = *node++;
  const uint count = tree.GetNumChildren(flags);
  if (count == 0) {
    switch (flags & 0x18) {
    case 0x18: {
      rstl::auto_ptr< const char > data(reinterpret_cast< const char* >(node));
      data.release();
      const CPVSVisSet set(tree.GetNumObjects(), tree.GetNumLights(), data);
      callback(tree, bounds, set, depth, ordinal++, context);
      break;
    }
    case 0x10:
      callback(tree, bounds, CPVSVisSet(kVSS_EndOfTree), depth, ordinal++, context);
      break;
    default:
      callback(tree, bounds, CPVSVisSet(kVSS_OutOfBounds), depth, ordinal++, context);
      break;
    }
    return;
  }

  // Prime's GetVisSet uses big-endian 16/24-bit offsets, or byte offsets.
  // CBasics::SwapBytes is a no-op on the big-endian GameCube target.
  // Every offset is relative to the first child, after the (count - 1) entries.
  const uint width = (flags & 0x60) == 0 ? 2 : ((flags & 0x20) != 0 ? 1 : 3);
  const uchar* children = node + (count - 1) * width;
  const CVector3f center =
      (bounds.GetMaxPoint() - bounds.GetMinPoint()) * 0.5f + bounds.GetMinPoint();
  for (uint child = 0; child < count; ++child) {
    uint offset = 0;
    if (child != 0) {
      const uchar* entry = node + (child - 1) * width;
      if (width == 1)
        offset = entry[0];
      else if (width == 2)
        offset = (entry[0] << 8) | entry[1];
      else
        offset = (entry[0] << 16) | (entry[1] << 8) | entry[2];
    }
    CVector3f min = bounds.GetMinPoint();
    CVector3f max = bounds.GetMaxPoint();
    uint bit = 1;
    for (uint axis = 0; axis < 3; ++axis) {
      if ((flags & (1 << axis)) != 0) {
        if ((child & bit) != 0)
          min[axis] = center[axis];
        else
          max[axis] = center[axis];
        bit <<= 1;
      }
    }
    TraversePVSNode(tree, children + offset, CAABox(min, max), callback, context, depth + 1,
                    ordinal);
  }
}
} // namespace

void CPVSVisOctree::TraverseLeaves(TLeafCallback callback, void* context) const {
  if (callback == nullptr || mOctreeData.get() == nullptr)
    return;
  int ordinal = 0;
  TraversePVSNode(*this, reinterpret_cast< const uchar* >(mOctreeData.get()), mBounds, callback,
                  context, 0, ordinal);
}

#endif
