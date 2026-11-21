#ifndef _CPASANIMSTATE
#define _CPASANIMSTATE
#include "rstl/RstlVersions.h"
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"

#if RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#else
#include "Kyoto/CRandom16.hpp"

#endif
#include "rstl/types.h"

#include "Kyoto/Animation/CPASAnimInfo.hpp"
#include "Kyoto/Animation/CPASAnimParm.hpp"
#include "Kyoto/Animation/CPASParmInfo.hpp"
#include "Kyoto/Animation/CharacterCommon.hpp"

#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"

#if RSTL_CORRUPTION


class CRandom16;

struct SPASAnimSelection { // Guessed name.
  SPASAnimSelection(float weight, int animId, int animIndex)
  : mWeight(weight), mAnimId(animId), mAnimIndex(animIndex) {}

  bool IsValid() const { return mAnimId >= 0; }

  float mWeight;
  int mAnimId;
  int mAnimIndex;
};
CHECK_SIZEOF(SPASAnimSelection, 0xC)

#endif
class CPASAnimState {
public:
  CPASAnimState(int id);
  CPASAnimState(CInputStream& in);

#if RSTL_CORRUPTION
  CPASAnimParm GetAnimParmData(int animIndex, unsigned int parmIdx) const;
#else
  CPASAnimParm GetAnimParmData(int, unsigned int) const;
  // Guessed name; reconstructed from Prime's stripped editing method.
  void AddAnimParmData(int animId, const rstl::reserved_vector< CPASAnimParm, 8 >& parms);
#endif
  const CPASAnimInfo* GetAnimInfoByIndex(int index) const; // Guessed name.
#if RSTL_CORRUPTION
  SPASAnimSelection FindBestAnimation(const rstl::reserved_vector< CPASAnimParm, 8 >& parms,
#else

  rstl::pair< float, int > FindBestAnimation(const rstl::reserved_vector< CPASAnimParm, 8 >& parms,
#endif
                                             CRandom16& random, int ignoreAnim) const;

  pas::EAnimationState GetStateId() const { return mId; }
  bool HasAnims() const { return static_cast< uint >(mAnims.size()) != 0; }
#if !RSTL_PRIME1
  int GetNumAnims() const { return mAnims.size(); }
#endif

  bool operator<(const CPASAnimState& rhs) const { return mId < rhs.mId; }

private:
  float ComputeExactMatchWeight(uint idx, const CPASAnimParm& parm,
                                CPASAnimParm::UParmValue value) const;
  float ComputePercentErrorWeight(uint idx, const CPASAnimParm& parm,
                                  CPASAnimParm::UParmValue value) const;
  float ComputeAngularPercentErrorWeight(uint idx, const CPASAnimParm& parm,
                                         CPASAnimParm::UParmValue value) const;
#if RSTL_CORRUPTION
  SPASAnimSelection PickRandomAnimation(CRandom16& random) const;
#else
  int PickRandomAnimation(CRandom16& random) const;
#endif

  pas::EAnimationState mId;
  rstl::vector< CPASParmInfo > mParms;
  rstl::vector< CPASAnimInfo > mAnims;
#if RSTL_CORRUPTION
  mutable rstl::vector< SPASAnimSelection > mSelectionCache;
#else
  mutable rstl::vector< int > mSelectionCache;
#endif
};
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1 || RSTL_VERSION < RSTL_R3IJ
CHECK_SIZEOF(CPASAnimState, 0x34)
#endif

#endif // _CPASANIMSTATE
