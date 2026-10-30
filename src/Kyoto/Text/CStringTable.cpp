#include "rstl/RstlVersions.h"
#include "Kyoto/Text/CStringTable.hpp"

#include "Kyoto/CDvdRequest.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CFactoryMgr.hpp"
#include "Kyoto/Alloc/CMemory.hpp"

#include "Kyoto/Streams/CInputStream.hpp"
#if RSTL_PRIME1
#include "Kyoto/Streams/CMemoryInStream.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#if TARGET_LITTLE_ENDIAN || WCHAR_MAX > 0xffff
#include "string.h"
#endif
#endif

// TODO: verify that this Prime/Echoes difference is real
#include "rstl/pair.hpp"
#include "rstl/vector.hpp"
#include "rstl/algorithm.hpp"
#include "rstl/reserved_vector.hpp"
#if RSTL_PRIME1
#else
#include "string.h"
#endif

static const wchar_t skInvalidString[] = L"Invalid";
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_GM8P_00
static const FourCC skLanguages[] = {'ENGL', 'GERM', 'FREN', 'SPAN', 'ITAL', 'DUTC', 'JAPN'};
static FourCC mCurrentLanguage = skLanguages[0];

void CStringTable::SetLanguage(int language) { mCurrentLanguage = skLanguages[language]; }
#else
static FourCC mCurrentLanguage = 'ENGL';
#endif
#else
const FourCC skLanguages[] = {'ENGL', 'GERM', 'FREN', 'SPAN', 'ITAL', 'DUTC', 'JAPN'};
static FourCC mCurrentLanguage = skLanguages[0];

struct SLanguageInfo {
  FourCC mLanguage;
  uint mOffset;
  uint mSize;
};

struct SStringNameLess {
  template < typename T >
  bool operator()(const T& a, const T& b) const {
    return strcmp(a.mName, b.mName) < 0;
  }
};

void CStringTable::SetLanguage(int language) { mCurrentLanguage = skLanguages[language]; }
#endif

CStringTable::CStringTable(CInputStream& in)
: mStringCount(0)
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
, mData(NULL)
#if RSTL_VERSION >= RSTL_GM8P_00
, mReloadData(nullptr)
{
  Load(in);
}

CStringTable::~CStringTable() {}

void CStringTable::Load(CInputStream& in)
#endif
{
  in.ReadLong();
  in.ReadLong();
  int langCount = in.Get(TType< int >());
  mStringCount = in.Get(TType< uint >());
  rstl::vector< rstl::pair< FourCC, uint > > langOffsets(langCount);
  for (int i = 0; i < langCount; ++i) {
    langOffsets.push_back(in.Get(TType< rstl::pair< FourCC, uint > >()));
#else
, mNameCount(0)
, mData(nullptr)
, mNames(nullptr)
, mStrings(nullptr)
, mReloadData(nullptr) {
  Load(in);
#endif

  }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  int offset = langOffsets.front().second;
#else
CStringTable::~CStringTable() {}

void CStringTable::Load(CInputStream& in) {
  in.ReadInt32();
  uint version = in.ReadInt32();
  int langCount = in.Get(TType< int >());
  mStringCount = in.Get(TType< int >());

  rstl::reserved_vector< SLanguageInfo, 7 > languages;
#endif

  for (int i = 0; i < langCount; ++i) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    if (langOffsets[i].first == mCurrentLanguage) {
      offset = langOffsets[i].second;
#else
    FourCC language = in.Get(TType< FourCC >());
    uint offset = in.ReadUint32();
    uint size = version != 0 ? in.Get(TType< uint >()) : 0;
    SLanguageInfo info = {language, offset, size};
    languages.push_back(info);
  }

  uint offset = languages[0].mOffset;
  uint size = languages[0].mSize;
  for (int i = 0; i < langCount; ++i) {
    if (languages[i].mLanguage == mCurrentLanguage) {
      offset = languages[i].mOffset;
      size = languages[i].mSize;
#endif

      break;
    }
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  for (uint i = 0; i < offset; ++i) {
    in.ReadChar();
#else
  if (version != 0) {
    mNameCount = in.Get(TType< int >());
    uint namesSize = in.Get(TType< uint >());
    mData = rs_new uchar[namesSize + size];
    mNames = reinterpret_cast< SStringName* >(mData.get());
    mStrings = reinterpret_cast< const wchar_t** >(mData.get() + namesSize);
    in.ReadBytes(mNames, namesSize);
#endif

  }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  uint dataLen = in.Get(TType< uint >());
#if TARGET_LITTLE_ENDIAN || WCHAR_MAX > 0xffff
  mNativeStrings.clear();
  rstl::vector< uchar > data(dataLen, uchar(0));
  in.ReadBytes(data.data(), dataLen);
  if (mStringCount < 0 || static_cast< uint >(mStringCount) > dataLen / sizeof(uint)) {
    mStringCount = 0;
    return;
  }

  CMemoryInStream offsets(data.data(), mStringCount * sizeof(uint));
  mNativeStrings.reserve(mStringCount);
  for (int i = 0; i < mStringCount; ++i) {
    uint pos = offsets.ReadLong();
    rstl::vector< wchar_t > text;
    bool terminated = false;
    while (pos <= dataLen && dataLen - pos >= sizeof(ushort)) {
      ushort unit;
      memcpy(&unit, data.data() + pos, sizeof(unit));
      uint codepoint = CBasics::SwapBytes(unit);
      pos += sizeof(unit);
      if (codepoint == 0) {
        terminated = true;
        break;
      }

      // Preserve UTF-16 on 16-bit wchar_t hosts; combine surrogate pairs on
      // hosts whose wide characters can hold a complete Unicode code point.
      if (sizeof(wchar_t) > sizeof(ushort) && codepoint >= 0xd800 && codepoint <= 0xdbff &&
          dataLen - pos >= sizeof(ushort)) {
        memcpy(&unit, data.data() + pos, sizeof(unit));
        const ushort low = CBasics::SwapBytes(unit);
        if (low >= 0xdc00 && low <= 0xdfff) {
          codepoint = 0x10000 + ((codepoint - 0xd800) << 10) + low - 0xdc00;
          pos += sizeof(unit);
        }
      }
      text.push_back(static_cast< wchar_t >(codepoint));
    }
    if (!terminated) {
      text.clear();
      for (const wchar_t* c = skInvalidString; *c; ++c) {
        text.push_back(*c);
      }
    }
    text.push_back(0);
    mNativeStrings.push_back(text);
  }
#else
  mData = rs_new uchar[dataLen];
  in.ReadBytes(mData.get(), dataLen);
#endif
#else
  for (uint i = 0; i < offset; ++i) {
    in.ReadInt8();
  }

  if (version == 0) {
    size = in.Get(TType< uint >());
    mData = rs_new uchar[size];
    mStrings = reinterpret_cast< const wchar_t** >(mData.get());
  }
  in.ReadBytes(mStrings, size);

  if (version != 0) {
    SStringName* entry = mNames;
    for (int i = 0; i < mNameCount; ++i, ++entry) {
      entry->mName += reinterpret_cast< uint >(mNames);
    }
  }

  uint* entry = reinterpret_cast< uint* >(mStrings);
  for (int i = 0; i < mStringCount; ++i, ++entry) {
    *entry += reinterpret_cast< uint >(mStrings);
  }
#endif

}

const wchar_t* CStringTable::GetString(int idx) const {
  if (idx < 0 || idx >= mStringCount) {
    return skInvalidString;
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if TARGET_LITTLE_ENDIAN || WCHAR_MAX > 0xffff
  return mNativeStrings[idx].data();
#else
  int offset = *(reinterpret_cast< const int* >(mData.get()) + idx);
  return reinterpret_cast< const wchar_t* >(mData.get() + offset);
#endif
#else
  return mStrings[idx];
#endif

}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_GM8P_00
void CStringTable::Reload(CAssetId id, CResFactory& factory) {
  mReloadData = nullptr;
  mReloadData = rs_new SReloadData(id, factory);
}

void CStringTable::TryFinishReload() {
  SReloadData* data = mReloadData.get();
  if (data != nullptr && data->mRequest->IsComplete()) {
    CMemoryInStream in(data->mBuffer.get(), data->mSize);
    Load(in);
    mReloadData = nullptr;
  }
}

bool CStringTable::IsReloading() const { return !mReloadData.null(); }

CStringTable::SReloadData::SReloadData(CAssetId id, CResFactory& factory)
: mRequest(nullptr) {
  const SObjectTag tag('STRG', id);
  mSize = factory.ResourceSize(tag);
  mBuffer = static_cast< uchar* >(CMemory::Alloc(mSize, IAllocator::kHI_RoundUpLen));
  mRequest = factory.LoadResourceAsync(tag, mBuffer.get());
}

CStringTable::SReloadData::~SReloadData() {}
#endif
#else
int CStringTable::GetStringIndexByName(const char* name) const {
  SStringName* begin = mNames;
  if (mNameCount > 0 && *name != '\0') {
    SStringName* end = begin + mNameCount;
    SStringName key = {name};
    SStringNameLess cmp;
    SStringName* it = rstl::lower_bound(begin, end, key, cmp);
    if (it != end && strcmp(it->mName, name) == 0) {
      return it->mIndex;
    }
  }
  return -1;
}
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
const CFactoryFnReturn FStringTableFactory(const SObjectTag& tag, CInputStream& in,
#else
const wchar_t* CStringTable::GetStringByName(const char* name) const {
  return GetString(GetStringIndexByName(name));
}

CStringTable::SReloadData::~SReloadData() {}

CFactoryFnReturn FStringTableFactory(const SObjectTag& tag, CInputStream& in,
#endif

                                     const CVParamTransfer& xfer) {
  return rs_new CStringTable(in);
}

