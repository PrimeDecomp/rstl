#include "Kyoto/Graphics/CMaterialDataFactory.hpp"

const void* CMaterialDataFactory::ParseIntegerChunk(const void* const& data, uchar* integers) {
  const void* cursor = data;
  int index = -1;
  switch (*reinterpret_cast< const int*& >(cursor)++) {
  case 'OPAC':
    index = 0;
    break;
  case 'BLOD':
    index = 1;
    break;
  case 'BLOI':
    index = 2;
    break;
  case 'BNIF':
    index = 3;
    break;
  }
  if (index != -1) {
    integers[index] = *static_cast< const uint* >(cursor);
  }
  return reinterpret_cast< const uint*& >(cursor) += 1;
}

CColor CMaterialDataFactory::ReadRGBABytes(const uchar* const& data) {
  const uchar* bytes = static_cast< const uchar* >(data);
  return CColor(bytes[0], bytes[1], bytes[2], bytes[3]);
}

const void* CMaterialDataFactory::ParseColorChunk(const void* const& data, SColors& colors) {
  const void* cursor = data;
  const int type = *static_cast< const int* >(cursor);
  reinterpret_cast< const uint*& >(cursor) += 1;
  switch (type) {
  case 'CLR ': {
    const uchar* bytes = static_cast< const uchar* >(cursor);
    colors.mColor = ReadRGBABytes(bytes);
  } break;
  case 'DIFB': {
    const uchar* bytes = static_cast< const uchar* >(cursor);
    colors.mDiffuseBloom = ReadRGBABytes(bytes);
  } break;
  }
  return reinterpret_cast< const uint*& >(cursor) += 1;
}

const void* CMaterialDataFactory::ParsePassChunk(const void* const& data, SPass* passes) {
  const void* cursor = data;
  const uint size = *reinterpret_cast< const uint*& >(cursor)++;
  const void* start = cursor;
  const int type = *reinterpret_cast< const int*& >(cursor)++;
  int index;
  switch (type) {
  case 'DIFF':
    index = kP_DIFF;
    break;
  case 'CLR ':
    index = kP_CLR;
    break;
  case 'TRAN':
    index = kP_TRAN;
    break;
  case 'INCA':
    index = kP_INCA;
    break;
  case 'RFLV':
    index = kP_RFLV;
    break;
  case 'RFLD':
    index = kP_RFLD;
    break;
  case 'LRLD':
    index = kP_LRLD;
    break;
  case 'LURD':
    index = kP_LURD;
    break;
  case 'BLOD':
    index = kP_BLOD;
    break;
  case 'BLOI':
    index = kP_BLOI;
    break;
  case 'BLOL':
    index = kP_BLOL;
    break;
  default:
    return static_cast< const uchar* >(start) + size;
  }
  SPass& pass = passes[index];
  pass.mFlags = *reinterpret_cast< const uint*& >(cursor)++;
  pass.mTextureId = CAssetId(*reinterpret_cast< const u64*& >(cursor)++);
  pass.mUvIndex = *reinterpret_cast< const uint*& >(cursor)++;
  pass.x14_ = *reinterpret_cast< const uint*& >(cursor)++;
  if (pass.x14_ != 0) {
    pass.mAnimationData = cursor;
  } else {
    pass.mAnimationData = nullptr;
  }
  return static_cast< const uchar* >(start) + size;
}

CMaterialDataFactory::SColors::SColors()
: mColor(CColor::White()), mDiffuseBloom(CColor::White()) {}

CMaterialDataFactory::SHeader CMaterialDataFactory::ParseMaterialChunks(const void* data,
                                                                        SPass* passes,
                                                                        uchar* integers,
                                                                        SColors& colors) {
  const uint flags = *reinterpret_cast< const uint*& >(data)++;
  const uint uniqueIndex = *reinterpret_cast< const uint*& >(data)++;
  const uint sortIndex = *reinterpret_cast< const uint*& >(data)++;
  const uint vertexDesc = *reinterpret_cast< const uint*& >(data)++;
  const uint uvCount = *reinterpret_cast< const uint*& >(data)++;
  const u64 collisionFlags = *reinterpret_cast< const u64*& >(data)++;
  SHeader header;
  header.mFlags = flags;
  header.mUniqueIndex = uniqueIndex;
  header.mSortIndex = sortIndex;
  header.mVertexDesc = vertexDesc;
  header.mUvCount = uvCount;
  header.mCollisionFlags = collisionFlags;
  header.mPassMask = 0;
  integers[1] = 255;
  integers[2] = 0;
  integers[0] = 255;
  integers[3] = 32;
  for (;;) {
    const int type = *static_cast< const int* >(data);
    reinterpret_cast< const uint*& >(data) += 1;
    switch (type) {
    case 'END ': {
      for (int i = 0; i < kP_Count; ++i) {
        if (passes[i].mTextureId != kInvalidAssetId) {
          header.mPassMask |= 1 << i;
        }
      }
      return header;
    }
    case 'INT ': {
      const void* chunk = data;
      data = ParseIntegerChunk(chunk, integers);
      break;
    }
    case 'CLR ': {
      const void* chunk = data;
      data = ParseColorChunk(chunk, colors);
      break;
    }
    case 'PASS': {
      const void* chunk = data;
      data = ParsePassChunk(chunk, passes);
      break;
    }
    default:
      const uint size = *static_cast< const uint* >(data);
      data = static_cast< const uchar* >(data) + size + sizeof(uint);
      break;
    }
  }
}

CMaterialDataFactory::SPass::SPass()
: mTextureId(kInvalidAssetId), mUvIndex(0), x14_(0), mAnimationData(nullptr) {}

CMaterialDataFactory::SMaterial::SMaterial(SHeader header, const SPass* passes,
                                           const uchar* integers, const SColors& colors)
: mHeader(header), mColors(colors) {
  for (int i = 0; i < kP_Count; ++i) {
    mPasses[i] = passes[i];
  }
  for (int i = 0; i < 4; ++i) {
    mIntegers[i] = integers[i];
  }
}

CMaterialDataFactory::SMaterial CMaterialDataFactory::ParseMaterialRecord(const void* data) {
  SPass passes[kP_Count];
  uchar integers[4];
  SColors colors;
  return SMaterial(ParseMaterialChunks(data, passes, integers, colors), passes, integers, colors);
}

rstl::vector< CMaterialDataFactory::SMaterial >
CMaterialDataFactory::BuildMaterialRecords(const void* data) {
  const uint count = *static_cast< const uint* >(data);
  const uchar* cursor = static_cast< const uchar* >(data) + sizeof(uint);
  rstl::vector< SMaterial > materials;
  materials.reserve(count);
  for (uint i = 0; i < count; ++i) {
    const uint size = *reinterpret_cast< const uint* >(cursor);
    cursor += sizeof(uint);
    materials.push_back(ParseMaterialRecord(cursor));
    cursor += size;
  }
  return materials;
}

