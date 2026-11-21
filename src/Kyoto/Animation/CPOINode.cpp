#include "rstl/RstlVersions.h"
#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CPOINode.hpp"

#include "Kyoto/CCrc32.hpp"

#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/string.hpp"

#if RSTL_CORRUPTION


const uint CPOINode::skInvalidNameHash = uint(-1);
const int CPOINode::skDefaultUnk2C = -1;

static uint ReadNameHash(ushort version, CInputStream& in) {
  if (version >= 4) {
    // Skip the stored name; the hash follows it.
    while (in.ReadUint8()) {
    }
    return in.ReadInt32();
  }
  rstl::string name(in);
  return CPOINode::GetHashForString(name.data());
}

CPOINode::CPOINode(const uint nameHash, const EPOIType type, const CCharAnimTime& time,
                   const int index, const bool unique, const float weight, const int charIdx,
                   const int flags, const int unk2C, const CSegId& segId)
: mVersion(3)
, mNameHash(nameHash)
#else
#if !RSTL_PRIME1
const ushort CPOINode::skExtendedVersion = 1;
const uint CPOINode::skInvalidNameHash = uint(-1);

static uint ReadNameHash(ushort version, CInputStream& in) {
  if (version >= 3) {
    // Skip the stored name; the hash follows it.
    while (in.ReadUint8()) {
    }
    return in.ReadInt32();
  }
  rstl::string name(in);
  return CPOINode::GetHashForString(name.data());
}

#endif

CPOINode::CPOINode(
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    const rstl::string& name,
#else
    const uint nameHash,
#endif
    const EPOIType type, const CCharAnimTime& time, const int index, const bool unique,
    const float weight, const int charIdx, const int flags)
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
: x4_(1)
, mName(name)
#else
: mVersion(2)
, mNameHash(nameHash)
#endif
#endif
, mType(type)
, mTime(time)
, mIndex(index)
, mUnique(unique)
, mWeight(weight)
, mCharIdx(charIdx)
#if RSTL_CORRUPTION
, mFlags(flags)
, mUnk2C(unk2C)
, mSegId(segId) {}
#else
, mFlags(flags) {}
#endif

CPOINode::CPOINode(CInputStream& in)
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
: x4_(in.Get< ushort >())
, mName(in)
#else
: mVersion(in.Get< ushort >())
, mNameHash(ReadNameHash(mVersion, in))
#endif
, mType(in.Get< ushort >())
, mTime(in)
#if RSTL_CORRUPTION
, mIndex(in.Get< int >())
#else
, mIndex(in.Get< uint >())
#endif
, mUnique(in.Get< bool >())
, mWeight(in.Get< float >())
, mCharIdx(in.Get< int >())
#if RSTL_CORRUPTION
, mFlags(in.Get< int >())
, mUnk2C(in.Get< int >())
, mSegId(in) {}
#else
, mFlags(in.Get< int >()) {}
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
bool CPOINode::operator>(const CPOINode& other) const { return mTime > other.mTime; }

bool CPOINode::operator<(const CPOINode& other) const { return mTime < other.mTime; }
#else
uint CPOINode::GetHashForString(const char* str) { return CCRC32::CalculateString(str, -1); }
#endif

int CPOINode::compare(const void* a, const void* b) {
  const CPOINode& aNode = *static_cast< const CPOINode* >(a);
  const CPOINode& bNode = *static_cast< const CPOINode* >(b);
  return aNode > bNode ? 1 : aNode < bNode ? -1 : 0;
}
