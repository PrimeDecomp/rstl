#ifndef _CPASDATABASE
#define _CPASDATABASE
#include "rstl/RstlVersions.h"
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"

#if RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#else
#include "Kyoto/CRandom16.hpp"

#endif
#include "rstl/types.h"

#include "Kyoto/Animation/CPASAnimState.hpp"

#if RSTL_CORRUPTION
#include "rstl/bit_vector.hpp"
#endif
#include "rstl/pair.hpp"
#include "rstl/vector.hpp"

class CPASAnimParmData;
#if RSTL_CORRUPTION
class CRandom16;
#endif

class CPASDatabase {

public:
  CPASDatabase(CInputStream& in);

  bool HasState(int id) const;
  const CPASAnimState* GetAnimState(int) const;

#if RSTL_CORRUPTION
  SPASAnimSelection FindBestAnimation(const CPASAnimParmData& data, int ignoreAnim) const;
  SPASAnimSelection FindBestAnimation(const CPASAnimParmData&, CRandom16&, int) const;
#else
  rstl::pair< float, int > FindBestAnimation(const CPASAnimParmData& data, int ignoreAnim) const;
  rstl::pair< float, int > FindBestAnimation(const CPASAnimParmData&, CRandom16&, int) const;
#endif

  size_t GetNumAnimStates() const;
  const CPASAnimState* GetAnimStateByIndex(int index) const;

  void AddAnimState(const CPASAnimState& state);
  int GetDefaultState() const { return mDefaultState; }
  void SetDefaultState(int state);

private:
  rstl::vector< CPASAnimState > mStates;
  int mDefaultState;
#if RSTL_CORRUPTION
  rstl::bit_vector<> mStatePresent; // Guessed name.
#endif
};
#if RSTL_CORRUPTION
CHECK_SIZEOF(CPASDatabase, 0x28)
#endif
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_CORRUPTION
#if !RSTL_PRIME1 || RSTL_VERSION < RSTL_R3IJ
CHECK_SIZEOF(CPASDatabase, 0x14)
#endif
#endif

#endif // _CPASDATABASE
