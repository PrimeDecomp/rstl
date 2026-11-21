#ifndef _CMATERIALTEXTURESTATE
#define _CMATERIALTEXTURESTATE

#include "rstl/static_assert.hpp"
#include "Kyoto/Graphics/CMaterialDataFactory.hpp"
#include "Kyoto/TToken.hpp"
#include "rstl/reserved_vector.hpp"

#include <dolphin/gx.h>



class CTexture;

// Guessed name
class CMaterialTextureState {
public:
  // Guessed name
  struct SMaterialTextures {
    int mUniqueIndex;
    rstl::reserved_vector< int, CMaterialDataFactory::kP_Count > mTextureIndices;
    int mPassTexMapIndices[CMaterialDataFactory::kP_Count];
    rstl::reserved_vector< GXTexGenSrc, CMaterialDataFactory::kP_Count > mTexCoordSources;
    int mPassTexCoordIndices[CMaterialDataFactory::kP_Count];
  };

  CMaterialTextureState(const rstl::vector< CMaterialDataFactory::SMaterial >& materials,
                        CSimplePool* pool);

  bool HasLockedTextures() const { return mTexturesLocked; } // Guessed name
  const rstl::vector< TCachedToken< CTexture > >& GetTextures() const { return mTextures; }

  void LockTextures();
  bool TryCacheTextures();
  void BindTextures(int materialIndex);
  int SetupTexCoordGenerators(int materialIndex) const;
  int GetUniqueIndex(int materialIndex) const { return mMaterials[materialIndex].mUniqueIndex; } // Guessed name
  int GetPassTexMapIndex(int materialIndex, int pass) const;
  int GetPassTexCoordIndex(int materialIndex, int pass) const;

private:
  void FinalizeMappings();

  rstl::vector< TCachedToken< CTexture > > mTextures;
  rstl::vector< SMaterialTextures > mMaterials;
  int mFirstUncachedTexture;
  bool mTexturesLocked;
  bool mTexturesLoaded;
  bool mMappingsFinalized;
};
CHECK_SIZEOF(CMaterialTextureState, 0x28)
NESTED_CHECK_SIZEOF(CMaterialTextureState, SMaterialTextures, 0xbc)


#endif // _CMATERIALTEXTURESTATE
