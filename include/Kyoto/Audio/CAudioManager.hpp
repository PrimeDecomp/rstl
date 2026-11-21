#ifndef _CAUDIOMANAGER
#define _CAUDIOMANAGER

#include "rstl/static_assert.hpp"
#include "Kyoto/Audio/CAudioHandle.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

namespace FMOD {
class System;
}


class CAudioVoice;
class CRandom16;

class CAudioManager {
public:
  struct SSoundDebugRecord { // Guessed name
    SSoundDebugRecord(CVector3f position, float innerRadius, float outerRadius,
                      const rstl::string& name)
    : mPosition(position), mInnerRadius(innerRadius), mOuterRadius(outerRadius), mName(name) {}

    CVector3f mPosition;
    float mInnerRadius;
    float mOuterRadius;
    rstl::string mName;
  };

  static void InitializeWithMemoryCallbacks(int aramOffset, int aramSize); // Guessed name
  static void RestartSystem();                                             // Guessed name
  static void Shutdown();
  static void Update(float dt);
  static void SetListenerVectors(const CVector3f& position, const CVector3f& velocity,
                                 const CVector3f& forward, const CVector3f& up); // Guessed name
  static float GetTotalCpuPercent();                                             // Guessed name
  static float GetDspCpuPercent();                                               // Guessed name
  static float GetStreamCpuPercent();                                            // Guessed name
  static float GetUpdateCpuPercent();                                            // Guessed name
  static int GetFMODAllocatedBytes();                                            // Guessed name
  static int GetChannelsPlaying();                                               // Guessed name
  static CAudioHandle AllocateVoice();                                           // Guessed name
  static CAudioVoice* LookupVoice(const CAudioHandle& handle);                   // Guessed name
  static CRandom16& GetRandom();                                                 // Guessed name
  static void SetSfxVolume(float volume);                                        // Guessed name
  static void SetSpeakerModeIndex(int mode);                                     // Guessed name
  static int GetSpeakerModeIndex();                                              // Guessed name
  static void StopAllVoices();                                                   // Guessed name
  static void StopContextVoices(int context);                                    // Guessed name
  static void SetContextPaused(int context, bool paused);                        // Guessed name
  static void SetVoiceContext(int context);                                      // Guessed name
  static int GetVoiceContext();                                                  // Guessed name
  static void UpdateVoiceIdSets(int currentArea,
                                const rstl::reserved_vector< int, 16 >& visibleAreas,
                                const rstl::reserved_vector< int, 16 >& otherAreas); // Guessed name
  static void SetMuted(bool muted);
  static rstl::vector< rstl::string > BuildVoiceDebugStrings();             // Guessed name
  static rstl::vector< SSoundDebugRecord > BuildSpatialVoiceDebugRecords(); // Guessed name
  static void SetDebugOption(int option, bool enabled);                     // Guessed name
  static bool GetDebugOption(int option);                                   // Guessed name
  static int AddLowPassAreaFilter(int frequency, float duration);
  static int AddCameraLowPassFilter(int frequency, float duration); // Guessed name
  static void RemoveCameraLowPassFilter(int id);                    // Guessed name

  static FMOD::System* mpFMODSystem; // Guessed name

private:
  friend class CAudioHandle;
  static void InitializeSystem();                                     // Guessed name
  static void InitializeSystemHook(int value);                        // Guessed name
  static CAudioVoice* FindVoiceForHandle(const CAudioHandle& handle); // Guessed name
  static rstl::reserved_vector< CAudioVoice, 96 > mVoices;            // Guessed name
};
NESTED_CHECK_SIZEOF(CAudioManager, SSoundDebugRecord, 0x24)


#endif // _CAUDIOMANAGER
