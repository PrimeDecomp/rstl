#ifndef _IANIMSOURCEINFO
#define _IANIMSOURCEINFO
#include "rstl/RstlVersions.h"

#include "Kyoto/Animation/CCharAnimTime.hpp"

#if RSTL_PRIME1
#include "rstl/vector.hpp"

class CBoolPOINode;
class CInt32POINode;
class CParticlePOINode;
class CSoundPOINode;
#endif

class IAnimSourceInfo {
public:
#if RSTL_PRIME1
  virtual bool HasPOIData() const = 0;
  virtual const rstl::vector< CBoolPOINode >& GetBoolPOIStream() const = 0;
  virtual const rstl::vector< CInt32POINode >& GetInt32POIStream() const = 0;
  virtual const rstl::vector< CParticlePOINode >& GetParticlePOIStream() const = 0;
  virtual const rstl::vector< CSoundPOINode >& GetSoundPOIStream() const = 0;
#endif

  virtual CCharAnimTime GetAnimationDuration() const = 0;
#if !RSTL_PRIME1
  // Guessed name: tests whether the source has scale keys.
  virtual bool HasScaleData() const = 0;
#endif

  virtual ~IAnimSourceInfo() {}
};
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CHECK_SIZEOF(IAnimSourceInfo, 0x4)
#else
DECLARE_FULL_SIZE_FOR(IAnimSourceInfo, 0x4)
CHECK_SIZEOF(IAnimSourceInfo, IAnimSourceInfo_FULL_SIZE)
#endif

#endif // _IANIMSOURCEINFO
