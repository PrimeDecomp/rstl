#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/RstlVersions.h"
#include "rstl/macros.hpp"
#endif
#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/algorithm.hpp"

#if RSTL_CORRUPTION


CPASDatabase::CPASDatabase(CInputStream& in) : mDefaultState(-1), mStatePresent(64, false) {
#else
CPASDatabase::CPASDatabase(CInputStream& in) : mDefaultState(-1) {
#endif
  in.Get< int >();

  const uint stateCount = in.Get< uint >();
  mStates.reserve(stateCount);
  const int defaultState = in.Get< int >();
  for (int i = 0; i < stateCount; i++) {
    AddAnimState(CPASAnimState(in));
  }

#if RSTL_CORRUPTION
  if (stateCount != 0 && defaultState >= 0) {
#else
  if (stateCount != 0) {
#endif
    SetDefaultState(defaultState);
  }
}

#if RSTL_CORRUPTION
SPASAnimSelection CPASDatabase::FindBestAnimation(const CPASAnimParmData& data,
#else
rstl::pair< float, int > CPASDatabase::FindBestAnimation(const CPASAnimParmData& data,
#endif
                                                         int ignoreAnim) const {
#if RSTL_CORRUPTION
  rstl::vector< CPASAnimState >::const_iterator it =
      rstl::binary_find(mStates.begin(), mStates.end(), CPASAnimState(data.GetStateId()));
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  AUTO(it, rstl::binary_find(mStates.begin(), mStates.end(), CPASAnimState(data.GetStateId())));
#else
  rstl::vector< CPASAnimState >::const_iterator it =
      rstl::binary_find(mStates.begin(), mStates.end(), CPASAnimState(data.GetStateId()));
#endif
#endif
  if (it != mStates.end()) {
    CRandom16 random(0x1234);
    return it->FindBestAnimation(data.GetAnimParmData(), random, ignoreAnim);
  }
#if RSTL_CORRUPTION
  return SPASAnimSelection(0.f, -1, -1);
#else
  return rstl::pair< float, int >(0.f, -1);
#endif
}

#if RSTL_CORRUPTION
SPASAnimSelection CPASDatabase::FindBestAnimation(const CPASAnimParmData& data,
#else
rstl::pair< float, int > CPASDatabase::FindBestAnimation(const CPASAnimParmData& data,
#endif
                                                         CRandom16& random, int ignoreAnim) const {
#if RSTL_CORRUPTION
  rstl::vector< CPASAnimState >::const_iterator it =
      rstl::binary_find(mStates.begin(), mStates.end(), CPASAnimState(data.GetStateId()));
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  AUTO(it, rstl::binary_find(mStates.begin(), mStates.end(), CPASAnimState(data.GetStateId())));
#else
  rstl::vector< CPASAnimState >::const_iterator it =
      rstl::binary_find(mStates.begin(), mStates.end(), CPASAnimState(data.GetStateId()));
#endif
#endif
  if (it != mStates.end()) {
    return it->FindBestAnimation(data.GetAnimParmData(), random, ignoreAnim);
  }
#if RSTL_CORRUPTION
  return SPASAnimSelection(0.f, -1, -1);
#else
  return rstl::pair< float, int >(0.f, -1);
#endif
}

#if RSTL_CORRUPTION
bool CPASDatabase::HasState(int id) const { return mStatePresent[id]; }
#else
#if RSTL_PRIME1
size_t CPASDatabase::GetNumAnimStates() const { return mStates.size(); }
#endif

bool CPASDatabase::HasState(int id) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  AUTO(it, rstl::binary_find(mStates.begin(), mStates.end(), CPASAnimState(id)));
#else
  rstl::vector< CPASAnimState >::const_iterator it =
      rstl::binary_find(mStates.begin(), mStates.end(), CPASAnimState(id));
#endif
  return it != mStates.end();
}
#endif

const CPASAnimState* CPASDatabase::GetAnimState(int id) const {
  return rstl::binary_find(mStates.begin(), mStates.end(), CPASAnimState(id)).get_pointer();
}
#if !RSTL_CORRUPTION

#if RSTL_PRIME1
const CPASAnimState* CPASDatabase::GetAnimStateByIndex(int index) const {
  return &mStates[index];
}
#endif
#endif

void CPASDatabase::SetDefaultState(int state) { mDefaultState = state; }

void CPASDatabase::AddAnimState(const CPASAnimState& state) {
  const rstl::vector< CPASAnimState >::iterator it =
      rstl::lower_bound(mStates.begin(), mStates.end(), state);
  mStates.insert(it, state);
#if RSTL_CORRUPTION
  mStatePresent[state.GetStateId()] = true;
#endif
}
