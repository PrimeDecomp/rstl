#ifndef _CPOINODE
#define _CPOINODE
#include "rstl/RstlVersions.h"

#include "types.h"

#include "Kyoto/Animation/CCharAnimTime.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

class CInputStream;

enum EPOIType {
  kPT_Loop = 0,
  kPT_EmptyBool = 1,
  kPT_EmptyInt32 = 2,
  kPT_SoundInt32 = 4,
  kPT_Particle = 5,
  kPT_UserEvent = 6,
  kPT_RandRate = 7,
  kPT_Sound = 8
#if !RSTL_PRIME1
  , kPT_StopLoopedSound = 9 // Guessed name
#endif
};

// Echoes replaces Prime's node name string with a CRC32 hash of it.
class CPOINode {
public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CPOINode(const rstl::string& name, EPOIType type
#else
  CPOINode(const uint nameHash, const EPOIType type
#endif
           , const CCharAnimTime& time, const int index,
           const bool unique, const float weight, const int charIdx, const int flags);
  CPOINode(CInputStream& in);
  virtual ~CPOINode() {}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const rstl::string& GetString() const { return mName; }
#else
  uint GetNameHash() const { return mNameHash; }
#endif

  const EPOIType GetPoiType() const { return static_cast< EPOIType >(mType); }
  const CCharAnimTime& GetTime() const { return mTime; }
  void SetTime(const CCharAnimTime& time) { mTime = time; }
  const int GetIndex() const { return mIndex; }
  const bool GetSaveState() const { return mUnique; }
  const float GetWeight() const { return mWeight; }
  const int GetCharacterIndex() const { return mCharIdx; }
  const int GetFlags() const { return mFlags; }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  bool operator>(const CPOINode& other) const;
  bool operator<(const CPOINode& other) const;
#else
  bool operator>(const CPOINode& other) const { return mTime > other.mTime; }
  bool operator<(const CPOINode& other) const { return mTime < other.mTime; }
#endif

  static int compare(const void* a, const void* b);
#if !RSTL_PRIME1
  static uint GetHashForString(const char* str);

  // Guessed names. Sound nodes above skExtendedVersion carry segment and pitch fields;
  // skInvalidNameHash marks unused POI state slots.
  static const ushort skExtendedVersion;
  static const uint skInvalidNameHash;
#endif

protected:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  ushort x4_;
  rstl::string mName;
#else
  ushort mVersion;
  uint mNameHash;
#endif

  ushort mType;
  CCharAnimTime mTime;
  int mIndex;
  bool mUnique;
  float mWeight;
  int mCharIdx;
  int mFlags;
};
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION < RSTL_R3IJ
CHECK_SIZEOF(CPOINode, 0x38)
#endif
#else
DECLARE_FULL_SIZE_FOR(CPOINode, 0x2c)
CHECK_SIZEOF(CPOINode, CPOINode_FULL_SIZE)
#endif

#endif // _CPOINODE
