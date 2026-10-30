#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CPOINode.hpp"

#include "Kyoto/CCrc32.hpp"

#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/string.hpp"

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
, mType(type)
, mTime(time)
, mIndex(index)
, mUnique(unique)
, mWeight(weight)
, mCharIdx(charIdx)
, mFlags(flags) {}

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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
, mIndex(in.Get< uint >())
#else
, mIndex(in.Get< int >())
#endif
, mUnique(in.Get< bool >())
, mWeight(in.Get< float >())
, mCharIdx(in.Get< int >())
, mFlags(in.Get< int >()) {}

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

