#ifndef _CAUDIOHANDLE
#define _CAUDIOHANDLE

#include "rstl/static_assert.hpp"
#include "rstl/types.h"

class CVector3f;

class CAudioHandle {
public:
  CAudioHandle();
  CAudioHandle(int voiceIndex);

  int GetVoiceIndex() const; // Guessed name
  static CAudioHandle NullHandle() { return CAudioHandle(); }
  void operator=(const CAudioHandle& other) { mID = other.mID; }
  bool operator==(const CAudioHandle& other) const { return mID == other.mID; }
  bool operator!=(const CAudioHandle& other) const { return mID != other.mID; }
  operator bool() const { return mID != 0; }
  void Clear() { mID = 0; }
  // Guessed name
  void StopWithFade();
  // Guessed name
  void SetVolume(float volume);
  // Guessed name
  void SetPosition(const CVector3f& position);
  // Guessed name
  void SetPositionAndScaledVelocity(const CVector3f& position, const CVector3f& velocity,
                                    float scale);
  float GetCharacterSoundPriority() const;                                           // Guessed name
  bool IsExclusiveCharacterSound() const;                                            // Guessed name
  bool IsLooping() const;                                                            // Guessed name
  float GetElapsedSeconds() const;                                                   // Guessed name
  void SetPitchBend(float pitch);                                                    // Guessed name
  void SetPan(float pan);                                                            // Guessed name
  bool IsActiveAndNotStopping() const;                                               // Guessed name
  void SetMinimumMaximumDistance(float minimum, float maximum);                      // Guessed name
  void SetSurroundPan(float pan);                                                    // Guessed name
  void SetPositionAndVelocity(const CVector3f& position, const CVector3f& velocity); // Guessed name

private:
  uint mID;
  static uint mRefCount;
};
CHECK_SIZEOF(CAudioHandle, 0x4)

#endif // _CAUDIOHANDLE
