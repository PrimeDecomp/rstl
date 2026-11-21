#ifndef _CMATERIALDATAFACTORY
#define _CMATERIALDATAFACTORY

#include "rstl/static_assert.hpp"
#include "Kyoto/CAssetId.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "rstl/vector.hpp"

// Guessed name
class CMaterialDataFactory {
public:
  enum EPass {
    kP_DIFF,
    kP_BLOL,
    kP_CLR,
    kP_TRAN,
    kP_INCA,
    kP_RFLV,
    kP_RFLD,
    kP_LRLD,
    kP_LURD,
    kP_BLOD,
    kP_BLOI,
    kP_Count
  };

  // Guessed name
  struct SHeader {
    uint mFlags;
    uint mUniqueIndex;
    uint mSortIndex;
    uint mVertexDesc;
    uint mUvCount;
    u64 mCollisionFlags;
    uint mPassMask;
  };

  // Guessed name
  struct SPass {
    SPass();

    uint mFlags;
    CAssetId mTextureId;
    uint mUvIndex;
    uint x14_;
    const void* mAnimationData;
  };

  // Guessed name
  struct SColors {
    SColors();

    CColor mColor;
    CColor mDiffuseBloom;
  };

  // Guessed name
  struct SMaterial {
    SMaterial(SHeader header, const SPass* passes, const uchar* integers, const SColors& colors);

    SHeader mHeader;
    SPass mPasses[kP_Count];
    uchar mIntegers[4]; // OPAC, BLOD, BLOI, BNIF.
    SColors mColors;
  };

  static rstl::vector< SMaterial > BuildMaterialRecords(const void* data);
  static SMaterial ParseMaterialRecord(const void* data);
  static SHeader ParseMaterialChunks(const void* data, SPass* passes, uchar* integers,
                                    SColors& colors);
  static const void* ParsePassChunk(const void* const& data, SPass* passes);
  static const void* ParseColorChunk(const void* const& data, SColors& colors);
  static const void* ParseIntegerChunk(const void* const& data, uchar* integers);
  static CColor ReadRGBABytes(const uchar* const& data);
};
NESTED_CHECK_SIZEOF(CMaterialDataFactory, SHeader, 0x28)
NESTED_CHECK_SIZEOF(CMaterialDataFactory, SPass, 0x20)
NESTED_CHECK_SIZEOF(CMaterialDataFactory, SColors, 0x8)
NESTED_CHECK_SIZEOF(CMaterialDataFactory, SMaterial, 0x198)

#endif // _CMATERIALDATAFACTORY
