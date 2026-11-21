#ifndef _CNAMEDANIMPOIDATA
#define _CNAMEDANIMPOIDATA

#include "rstl/static_assert.hpp"
#include "rstl/types.h"

#include "Kyoto/Animation/CAssetSoundPOINode.hpp"
#include "Kyoto/Animation/CParticlePOINode.hpp"

#include "rstl/string.hpp"
#include "rstl/vector.hpp"



class CInputStream;

// Guessed name. The particle and asset sound points of interest of one named animation event,
// each carrying the hash of its name.
class CNamedAnimPOIData {
public:
  // Guessed name.
  struct SParticleEntry {
    explicit SParticleEntry(CInputStream& in);

    rstl::string mName;
    uint mNameHash;
    CParticlePOINode mNode;
  };

  // Guessed name.
  struct SAssetSoundEntry {
    explicit SAssetSoundEntry(CInputStream& in);

    rstl::string mName;
    uint mNameHash;
    CAssetSoundPOINode mNode;
  };

  explicit CNamedAnimPOIData(CInputStream& in);
  CNamedAnimPOIData(int type, const rstl::string& name,
                    const rstl::vector< SParticleEntry >& particles,
                    const rstl::vector< SAssetSoundEntry >& assetSounds);

  // Guessed name. An entry without points of interest, used as a search key.
  static CNamedAnimPOIData MakeKey(int type, const rstl::string& name);

  bool operator<(const CNamedAnimPOIData& other) const;

  int GetType() const { return mType; }
  const rstl::string& GetName() const { return mName; }
  const rstl::vector< SParticleEntry >& GetParticleEntries() const { return mParticleEntries; }
  const rstl::vector< SAssetSoundEntry >& GetAssetSoundEntries() const {
    return mAssetSoundEntries;
  }

private:
  int mType; // Guessed name.
  rstl::string mName;
  rstl::vector< SParticleEntry > mParticleEntries;
  rstl::vector< SAssetSoundEntry > mAssetSoundEntries;
};
CHECK_SIZEOF(CNamedAnimPOIData, 0x34)


#endif // _CNAMEDANIMPOIDATA
