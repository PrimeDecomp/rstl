#ifndef _CPASDATABASE
#define _CPASDATABASE
#include "rstl/RstlVersions.h"

#include "Kyoto/CRandom16.hpp"

#include "types.h"

#include "Kyoto/Animation/CPASAnimState.hpp"

#include "rstl/pair.hpp"
#include "rstl/vector.hpp"

class CPASAnimParmData;

class CPASDatabase {

public:
  CPASDatabase(CInputStream& in);

  bool HasState(int id) const;
  const CPASAnimState* GetAnimState(int) const;

  rstl::pair< float, int > FindBestAnimation(const CPASAnimParmData& data, int ignoreAnim) const;
  rstl::pair< float, int > FindBestAnimation(const CPASAnimParmData&, CRandom16&, int) const;

  size_t GetNumAnimStates() const;
  const CPASAnimState* GetAnimStateByIndex(int index) const;

  void AddAnimState(const CPASAnimState& state);
  int GetDefaultState() const { return mDefaultState; }
  void SetDefaultState(int state);

private:
  rstl::vector< CPASAnimState > mStates;
  int mDefaultState;
};
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1 || RSTL_VERSION < RSTL_R3IJ
CHECK_SIZEOF(CPASDatabase, 0x14)
#endif

#endif // _CPASDATABASE
