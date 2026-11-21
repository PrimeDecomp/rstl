#ifndef _CAUDIOSOUNDEFFECT
#define _CAUDIOSOUNDEFFECT

#include "rstl/static_assert.hpp"
#include "rstl/types.h"

#include "Kyoto/Audio/CAudioHandle.hpp"
#include "Kyoto/Audio/CAudioSampleDescription.hpp"
#include "Kyoto/CToken.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"



class CVector3f;

class CAudioSoundEffect {
public:
  CAudioSoundEffect(CInputStream& in, IObjectStore& pool, const SObjectTag& tag);
  ~CAudioSoundEffect();
  // Guessed name
  CAudioHandle PlaySpatial(int areaId, const CVector3f& position, float volume);
  CAudioHandle PlaySpatial(const CVector3f& position, float volume); // Guessed name
  CAudioHandle PlayPanned(int areaId, float volume, float pan);
  CAudioHandle PlayPanned(float volume, float pan);
  float GetCharacterSoundPriority() const;         // Guessed name
  bool IsExclusiveCharacterSound() const;          // Guessed name
  bool IsLooping() const;                          // Guessed name
  int GetLastSelectedDescriptionIndex() const;     // Guessed name
  void SetLastSelectedDescriptionIndex(int index); // Guessed name
  const rstl::string& GetName() const;             // Guessed name
  float GetDuration() const { return mDuration; }  // Guessed name
  // Guessed name
  rstl::vector< CAudioSampleDescription >& GetDescriptions() { return mDescriptions; }

private:
  uint x0_;
  uint mVersion;
  float mDuration;
  float xC_;
  rstl::string mName;
  rstl::vector< rstl::string > x20_;
  rstl::vector< CAudioSampleDescription > mDescriptions;
  bool mLooping;
  bool mExclusiveCharacterSound;
  float mCharacterSoundPriority;
  int mLastSelectedDescriptionIndex;
  CToken mSelfToken;
};
CHECK_SIZEOF(CAudioSoundEffect, 0x54)


#endif // _CAUDIOSOUNDEFFECT
