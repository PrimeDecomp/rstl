#ifndef _CPOINODE
#define _CPOINODE
#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"
#endif

#if RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#endif
#include "rstl/types.h"

#include "Kyoto/Animation/CCharAnimTime.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/Animation/CSegId.hpp"
#endif
#include "rstl/string.hpp"
#if !RSTL_CORRUPTION
#include "rstl/vector.hpp"
#endif

#if !RSTL_CORRUPTION
class CInputStream;
#endif

enum EPOIType {
  kPT_Loop = 0,
  kPT_EmptyBool = 1,
  kPT_EmptyInt32 = 2,
  kPT_SoundInt32 = 4,
  kPT_Particle = 5,
  kPT_UserEvent = 6,
  kPT_RandRate = 7,
#if RSTL_CORRUPTION
  kPT_Sound = 8,
  kPT_StopLoopedSound = 9, // Guessed name
#else
  kPT_Sound = 8
#if !RSTL_PRIME1
  , kPT_StopLoopedSound = 9 // Guessed name
#endif
#endif
};

#if RSTL_CORRUPTION
class CInputStream;
#endif
// Echoes replaces Prime's node name string with a CRC32 hash of it.
class CPOINode {
public:
#if RSTL_CORRUPTION
  static const uint skInvalidNameHash; // Guessed name
  static const int skDefaultUnk2C; // Guessed name
#endif
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_CORRUPTION
  CPOINode(const uint nameHash, const EPOIType type, const CCharAnimTime& time, const int index,
           const bool unique, const float weight, const int charIdx, const int flags,
           const int unk2C = skDefaultUnk2C, const CSegId& segId = CSegId());
#else
#if RSTL_PRIME1
  CPOINode(const rstl::string& name, EPOIType type
#else
  CPOINode(const uint nameHash, const EPOIType type
#endif
           , const CCharAnimTime& time, const int index,
           const bool unique, const float weight, const int charIdx, const int flags);
#endif
  CPOINode(CInputStream& in);
  virtual ~CPOINode() {}

#if RSTL_CORRUPTION
  uint GetNameHash() const { return mNameHash; }
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const rstl::string& GetString() const { return mName; }
#else
  uint GetNameHash() const { return mNameHash; }
#endif

#endif
  const EPOIType GetPoiType() const { return static_cast< EPOIType >(mType); }
  const CCharAnimTime& GetTime() const { return mTime; }
  void SetTime(const CCharAnimTime& time) { mTime = time; }
  const int GetIndex() const { return mIndex; }
  const bool GetSaveState() const { return mUnique; }
  const float GetWeight() const { return mWeight; }
  const int GetCharacterIndex() const { return mCharIdx; }
  const int GetFlags() const { return mFlags; }
#if RSTL_CORRUPTION
  const int GetUnk2C() const { return mUnk2C; }
  const CSegId& GetSegId() const { return mSegId; }
#endif

#if RSTL_CORRUPTION
  bool operator>(const CPOINode& other) const { return mTime > other.mTime; }
  bool operator<(const CPOINode& other) const { return mTime < other.mTime; }
#else
  bool operator>(const CPOINode& other) const;
  bool operator<(const CPOINode& other) const;

#endif
  static int compare(const void* a, const void* b);
#if RSTL_CORRUPTION
  static uint GetHashForString(const char* str);

#else
#if !RSTL_PRIME1
  static uint GetHashForString(const char* str);

  // Guessed names. Sound nodes above skExtendedVersion carry segment and pitch fields;
  // skInvalidNameHash marks unused POI state slots.
  static const ushort skExtendedVersion;
  static const uint skInvalidNameHash;
#endif

#endif
protected:
#if RSTL_CORRUPTION
  ushort mVersion;
  uint mNameHash;
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  ushort x4_;
  rstl::string mName;
#else
  ushort mVersion;
  uint mNameHash;
#endif

#endif
  ushort mType;
  CCharAnimTime mTime;
  int mIndex;
  bool mUnique;
  float mWeight;
  int mCharIdx;
  int mFlags;
#if RSTL_CORRUPTION
  int mUnk2C;
  CSegId mSegId; // Guessed name
#endif
};
#if RSTL_CORRUPTION
CHECK_SIZEOF(CPOINode, 0x34)
#endif

#if !RSTL_CORRUPTION
#if !RSTL_PRIME1
inline bool CPOINode::operator>(const CPOINode& other) const { return mTime > other.mTime; }

inline bool CPOINode::operator<(const CPOINode& other) const { return mTime < other.mTime; }
#endif
DECLARE_FULL_SIZE_FOR(CPOINode, RSTL_PRIME1 ? 0x38 : 0x2c)
CHECK_SIZEOF(CPOINode, CPOINode_FULL_SIZE)

#endif
#endif // _CPOINODE
