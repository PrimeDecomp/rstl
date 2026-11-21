#ifndef _CAUDIOSAMPLE
#define _CAUDIOSAMPLE

#include "rstl/static_assert.hpp"
#include "Kyoto/Audio/CAudioChannel.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/string.hpp"


struct SObjectTag;

class CAudioSample {
public:
  CAudioSample(const rstl::auto_ptr< uchar >& data, int size, const SObjectTag& tag);
  ~CAudioSample();
  CAudioChannel CreateSpatialChannel(const CVector3f& position, float minimum, float maximum,
                                     bool paused) const; // Guessed name
  CAudioChannel CreatePannedChannel(bool paused) const;  // Guessed name

private:
  FMOD::Sound* mSound;
  FMOD_MODE mMode;
  int x8_;
  rstl::string mName;
};
CHECK_SIZEOF(CAudioSample, 0x1c)


#endif // _CAUDIOSAMPLE
